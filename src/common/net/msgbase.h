/*
 * msgbase.h
 * Copyright (C) 2006 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file msgbase
 *
 * This file contains all the base classes to use for messages, pieces of
 * information sent via network between client and server.
 *
 * The relation among the classes is as follows: the message type is part of the
 * message class, it has its own class to perform several operations for
 * convenience (translation between string and uint32 representation,
 * comparison, and so on). The actual messages present in the code would be
 * derived from the base class, which provides methods to serialize and
 * deserialize the data that it contains. After being sent from the client or
 * the server, the peer receives the byte stream and invoques operations to
 * create a message from the flow, and passes the message to the handler, which
 * should know what to do with the message (prepare the data contained in it to
 * be sent to a manager, etc).
 *
 * Messages and handlers are bound with the pseudo-factory. Client and server
 * should create a factory object, instantiate the handlers and register message
 * and handler in the pseudo-factory in one step (that's why it's not a
 * canonical factory, it's more like a structure binding a message with the
 * handler).
 *
 * The typical use of messages would be this (client and server roles are
 * interchangeable, both them send and receive messages in the same way):
 *
 * 1) The client sends a message, in example requesting to log in, so it creates
 * the login message and fills in the username, password, etc.
 *
 * 2) The client relies on the network layer to send the message to the server,
 * which includes to serialize the message (calling the serialization method of
 * the message itself, unless the implementation changes), and sends the message
 * over the socket.
 *
 * 3) The server receives the message on the other end, when the loop reaches
 * the socket and reads the data received. If some data is read, it tries to
 * find a new message in the received stream (which happens if it recognizes the
 * message type, and the stream has at least the size that the message needs),
 * creates the message and asks the message to deserialize the stream.
 *
 * 4) once the message is deserialized (this is, the slots of the message such
 * as username and password are in place again), it's passed to the handler so
 * it calls the login manager in the server with the required data.
 *
 * The last step means that the handler is the only thing that we need to change
 * for most minor changes in the message format or the login manager. Also, for
 * some messages that are not associated with managers (because the functions
 * needed to handle the message are very simple, or whatever reason), the
 * handler is the manager itself.
 *
 * Steps 3 and 4 are currently done inside the pseudo-factory.
 */

#ifndef __FEARANN_COMMON_NET_MSGBASE_H__
#define __FEARANN_COMMON_NET_MSGBASE_H__

#include "common/datatypes.h"
 
#include <string>
#include <vector>
#include <map>

class Netlink;


/** Identifier of a message, made a class of its own to be able to perform some
 * things with it, such as operators for comparison and translations among the
 * several representations (unsigned int code, string, etc).
 */
class MsgType
{
public:
	/** Constructor with code name as parameter. */
	MsgType(const char* codeName);
	/** Constructor with code number as parameter. */
	MsgType(uint32_t code);

	/** Equal Comparator */
	bool operator == (const MsgType& other) const;
	/** Not Equal Comparator */
	bool operator != (const MsgType& other) const;

	/** Get the type as string representation */
	const char* getName() const;
	/** Get the type as uint32 representation */
	uint32_t getID() const;
	/** Set the type by string representation */
	void setType(const char* codeName);
	/** Set the type by uint32 representation */
	void setType(uint32_t type);

private:
	/// The uint32 representation
	uint32_t typeID;
	/// The string representation
	std::string typeName;
};


/** A message to be sent through the network. The format is:
 *
 * <uint16_t size> <uint32_t type> <content>
 *
 * Note that this is an abstract class, the derived classes implementing the
 * different messages should provide their own functions to de/serialize the
 * actual data, setting the message type and so on.
 *
 * \note mafm: At the moment, this serial message class has everything to allow
 * its de/serialization, even the methods to read/write the data types from/to
 * the buffer. Maybe this is too much overhead when instantiating a message and
 * in the future it would be better to have a manager to do this, and the
 * messages using the manager to write in their own buffer. Anyway, the code for
 * the whole class is not that much and it shouldn't be a heavy burden at the
 * moment and until proved otherwise, so I leave this shiny version at the
 * moment.
 */
class MsgBase
{
public:
	/** Default constructor. */
	MsgBase();
	/** Destructor. */
	virtual ~MsgBase() { }

	/** Get the message type (abstract because it's defined for each derived
	 * message separately) */
	virtual MsgType getType() const = 0;
	/** Get the lenght of the message, including header (this is, the whole
	 * size of the chunk of data sent through the network) */
	uint32_t getLength() const;
	/** Get the buffer itself, ready to be sent by network layers */
	const char* getBuffer() const;
	/** Create a message of this type, note that the message created has to
	 * be deleted by the caller, otherwise it's a leak. */
	virtual MsgBase* createInstance() = 0;
	/** Serialize the content, so it's packed and ready to send by the
	 * network. This prepares the packet (headers and so on) and relies on
	 * the concrete implementation for the actual data. */
	void serialize();
	/** Counterpart of the serialization, it must be performed in the same
	 * order so the peer can rebuild an exact copy of the message. Note that
	 * it needs the buffer in the argument, because the message comes from
	 * the network -- the message is created empty initially, when received
	 * in the peer. */
	void deserialize(const char* buffer, size_t size);

protected:
	/// Whether is not this message is already serialized
	bool mIsSerialized;
	/// Whether is not this message is already deserialized
	bool mIsDeserialized;

	/** This function shuold be performed by the derived class depending on
	 * the actual data contained */
	virtual void serializeData() = 0;
	/** Counterpart of the serialization, it must be performed in the same
	 * order so the peer can rebuild an exact copy of the message */
	virtual void deserializeData() = 0;

	/** Dump the data to the buffer in apropriate network format */
	void write(const char* data, size_t size);
	/** Dump the data to the buffer in apropriate network format */
	void write(std::string& i);
	/** Dump the data to the buffer in apropriate network format */
	void write(char i);
	/** Dump the data to the buffer in apropriate network format */
	void write(bool i);
	/** Dump the data to the buffer in apropriate network format */
	void write(uint64_t i);
	/** Dump the data to the buffer in apropriate network format */
	void write(uint32_t i);
	/** Dump the data to the buffer in apropriate network format */
	void write(int32_t i);
	/** Dump the data to the buffer in apropriate network format */
	void write(uint16_t i);
	/** Dump the data to the buffer in apropriate network format */
	void write(int16_t i);
	/** Dump the data to the buffer in apropriate network format */
	void write(uint8_t i);
	/** Dump the data to the buffer in apropriate network format */
	void write(int8_t i);
	/** Dump the data to the buffer in apropriate network format */
	void write(float f);
	/** Dump the data to the buffer in apropriate network format */
	void write(Vector3 v3);

	/** Extract the data from the buffer in apropriate format */
	void read(char* data, size_t size);
	/** Extract the data from the buffer in apropriate format */
	void read(std::string& s);
	/** Extract the data from the buffer in apropriate format */
	void read(char& c);
	/** Extract the data from the buffer in apropriate format */
	void read(bool& b);
	/** Extract the data from the buffer in apropriate format */
	void read(uint64_t& i);
	/** Extract the data from the buffer in apropriate format */
	void read(uint32_t& i);
	/** Extract the data from the buffer in apropriate format */
	void read(int32_t& i);
	/** Extract the data from the buffer in apropriate format */
	void read(uint16_t& i);
	/** Extract the data from the buffer in apropriate format */
	void read(int16_t& i);
	/** Extract the data from the buffer in apropriate format */
	void read(uint8_t& i);
	/** Extract the data from the buffer in apropriate format */
	void read(int8_t& i);
	/** Extract the data from the buffer in apropriate format */
	void read(float& f);
	/** Extract the data from the buffer in apropriate format */
	void read(Vector3& v3);

private:
	/** Convenience wrapper around a simple buffer
	 */
	class Buffer {
	public:
		Buffer();
		~Buffer();
		/** Append to the buffer */
		void append(const char* buffer, size_t size);
		/** Pop the front element */
		char popFront();
		/** Extract given size from the front */
		void extractFront(char* buffer, size_t size);
		/** Overwrite given position (to set message size, in example) */
		void overwritePosition(size_t index, char c);
		/** Get the buffer itself */
		const char* getBuffer() const;
		/** Get current size of the buffer */
		uint32_t getSize() const;
	private:
		uint32_t mSize;
		char* mData;
	} mBuffer;

	/** Check if there's at least the given amount of data in the buffer, so
	 * we don't go out of bounds when deserializing even if there are coding
	 * errors. This is only necessary in the 'read' methods erasing data
	 * from the buffer, the methods using other methods don't need to
	 * check. */
	bool bufferGEThan(size_t bytes);
};


/** Interface for a message handler. This is the glue code between the the
 * network message and the manager, so changes are decoupled.
 */
class MsgHdlBase
{
public:
	/** Destructor. */
	virtual ~MsgHdlBase() {}

	/** Returns the message type that this handler can work with (abstract
	 * because we need to define the type when defining the handler
	 * itself). */
	virtual MsgType getMsgType() const = 0;
	/** Handle a message, return false if failed to do so. */
	virtual void handleMsg(MsgBase& msg, Netlink* netlink) = 0;
};


/** This is a pseudo-factory for network messages and their handlers.  The
 * reason to call it a pseudo-factory is that it's not a canonical one: instead
 * of being used to create the messages or the handlers, it will be used by
 * client and server network managers to create the messages from the stream
 * received, and to call the apropriate handler altogether.
 */
class MsgHdlFactory
{
public:
	/** Destructor. */
	~MsgHdlFactory();

	/** Function to register messages and handlers at once. Both client and
	 * server need to register a different types of messages, so it's better
	 * to let the network manager of each one to decide.  Note that the user
	 * of this function must provide an object created and don't delete it
	 * (it will be used by this class while active, and it will be deleted
	 * in the destructor).  There are few elegant alternatives (in example
	 * we could msg->createInstance, but we can't do that with handlers
	 * without defining that function in handlers too), so it seems fair to
	 * leave it in this way. */
	void registerMsgWithHdl(MsgBase* msg, MsgHdlBase* hdl);

	/** Handle a stream by creating the message, performing deserialization
	 * and calling the handler. */
	bool handleStream(Netlink& netlink,
			  uint32_t key,
			  const char* buffer,
			  uint32_t size);

private:
	/// This is the structure holding the message types based on keys
	std::map<uint32_t, std::pair<MsgBase*, MsgHdlBase*> > factories;

	/** Create an instance of the class, deserialized with given buffer */
	MsgBase* createMsgInstance(uint32_t key);

	/** Get the handler of a message */
	MsgHdlBase* getHdl(uint32_t key);
};


#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
