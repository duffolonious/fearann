/*
 * msgbase.cpp
 * Copyright (C) 2006-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#include "config.h"

#include <cstring>
#include <cstdlib>

#include "netlayer.h"

#include "msgbase.h"


//---------------------------- MsgType ---------------------------
MsgType::MsgType(const char* codeName)
{
	setType(codeName);
}

MsgType::MsgType(uint32_t code)
{
	setType(code);
}

bool MsgType::operator == (const MsgType& other) const
{
	return (typeID == other.typeID);
}

bool MsgType::operator != (const MsgType& other) const
{
	return ! (typeID == other.typeID);
}

const char* MsgType::getName() const
{
	return typeName.c_str();
}

uint32_t MsgType::getID() const
{
	return typeID;
}

void MsgType::setType(const char* codeName)
{
	if (strlen(codeName) != 4)
		LogERR("Bad code name '%s', msg type won't be valid",
		       codeName);
	typeName = codeName;
	typeID = ( (codeName[0] << 24) & 0xff000000 )
		| ( (codeName[1] << 16) & 0x00ff0000 )
		| ( (codeName[2] << 8) & 0x0000ff00 )
		| ( codeName[3] & 0x000000ff );
}

void MsgType::setType(uint32_t type)
{
	typeName[0] = (type >> 24) & 0xff;
	typeName[1] = (type >> 16) & 0xff;
	typeName[2] = (type >> 8) & 0xff;
	typeName[3] = (type) & 0xff;
	typeID = type;
}


//---------------------------- MsgBase ---------------------------
MsgBase::Buffer::Buffer() :
	mSize(0), mData(0)
{
	// mafm: This implementation is not very efficient, but at the time of
	// writing this files are transferred at up to 2GB (with sleep functions
	// in client and server loops, to avoid using 100% of CPU).  This is of
	// course where the client needs more power, and it's very unlikely that
	// neither clients or servers will use those speeds with an "official"
	// game server (unless we have 100K users in the same server or things
	// like that).
}

MsgBase::Buffer::~Buffer()
{
	delete [] mData;
}

void MsgBase::Buffer::append(const char* buffer, size_t s)
{
	if (mData == 0) {
		// empty buffer, append the new one
		mSize = s;
		mData = new char[s];
		memcpy(mData, buffer, s);
	} else {
		// existing buffer, append using a bigger buffer
		char* final = new char[mSize+s];
		memcpy(final, mData, mSize);
		memcpy(final+mSize, buffer, s);
		mSize += s;
		// delete old buffer
		char* aux = mData;
		delete [] aux;
		mData = final;
	}
}

char MsgBase::Buffer::popFront()
{
	char c;
	extractFront(&c, 1);
	return c;
}

void MsgBase::Buffer::extractFront(char* buffer, size_t s)
{
	if (s > mSize) {
		LogERR("Requesting %zu but only %u bytes available in the buffer",
		       s, mSize);
		buffer = 0;
		return;
	} else {
		// copy data to the given buffer
		memcpy(buffer, mData, s);

		// extract from existing buffer
		if (s == mSize) {
			// all of it
			delete [] mData;
			mData = 0;
			mSize = 0;
		} else {
			// part of it, only
			char* final = new char[mSize-s];
			memcpy(final, mData+s, mSize-s);
			mSize -= s;
			// delete old buffer
			char* aux = mData;
			delete [] aux;
			mData = final;
		}
	}
}

void MsgBase::Buffer::overwritePosition(size_t index, char c)
{
	if (index > mSize) {
		LogERR("Trying to overwrite non-existent position (%zu, size %u), skipping",
		       index, mSize);
	} else {
		mData[index] = c;
	}
}

const char* MsgBase::Buffer::getBuffer() const
{
	return mData;
}

uint32_t MsgBase::Buffer::getSize() const
{
	return mSize;
}

MsgBase::MsgBase() :
	mIsSerialized(false), mIsDeserialized(false)
{
}

void MsgBase::serialize()
{
	if (mIsSerialized) {
		LogERR("Message (type '%s', size %u) already serialized, skipping",
		       getType().getName(), mBuffer.getSize());
		return;
	}

	// reserve space for size and message type
	uint16_t size = 0;
	write(size);
	uint32_t type = getType().getID();
	write(type);

	// perform the message especific serialization
	serializeData();

	// actually write the size, now that we know it
	size = mBuffer.getSize();
	char s0 = static_cast<char>((size >> 8) & 0xff);
	char s1 = static_cast<char>(size & 0xff);
	mBuffer.overwritePosition(0, s0);
	mBuffer.overwritePosition(1, s1);

	mIsSerialized = true;
}

void MsgBase::deserialize(const char* buffer, size_t size)
{
	if (mIsDeserialized) {
		LogERR("Message (type '%s', size %u) already deserialized, skipping",
		       getType().getName(), mBuffer.getSize());
		return;
	}

	// set up the local buffer with the given data, ignoring header
	size_t headerSize = sizeof(uint16_t) + sizeof(uint32_t);
	PERM_ASSERT(size >= headerSize);
	mBuffer.append(buffer+headerSize, size-headerSize);

	// perform the message especific deserialization
	deserializeData();

	// check that the deserialization was complete and so the buffer is
	// empty
	if (mBuffer.getSize() != 0) {
		LogERR("Deserializing message (type '%s', size %zu), buffer not empty (%u bytes)",
		       getType().getName(), size, mBuffer.getSize());
	}

	mIsDeserialized = true;
}

bool MsgBase::bufferGEThan(size_t bytes)
{
	bool bigEnough = mBuffer.getSize() >= bytes;
	if (!bigEnough)
		LogERR("Deserializing msg, trying to read more data (%zu) than the available (%u)",
			bytes, mBuffer.getSize());
	return bigEnough;
}

void MsgBase::write(const char* data, size_t size)
{
	mBuffer.append(data, size);
}

void MsgBase::write(std::string& s)
{
	write(s.c_str() + '\0', s.size()+1);
}

void MsgBase::write(char c)
{
	mBuffer.append(&c, 1);
}

void MsgBase::write(bool b)
{
	write(static_cast<char>(b));
}

void MsgBase::write(uint64_t i)
{
	char aux[8];
	aux[0] = static_cast<char>((i >> 56) & 0xff);
	aux[1] = static_cast<char>((i >> 48) & 0xff);
	aux[2] = static_cast<char>((i >> 40) & 0xff);
	aux[3] = static_cast<char>((i >> 32) & 0xff);
	aux[4] = static_cast<char>((i >> 24) & 0xff);
	aux[5] = static_cast<char>((i >> 16) & 0xff);
	aux[6] = static_cast<char>((i >> 8) & 0xff);
	aux[7] = static_cast<char>((i >> 0) & 0xff);
	write(aux, 8);
}

void MsgBase::write(uint32_t i)
{
	char aux[4];
	aux[0] = static_cast<char>((i >> 24) & 0xff);
	aux[1] = static_cast<char>((i >> 16) & 0xff);
	aux[2] = static_cast<char>((i >> 8) & 0xff);
	aux[3] = static_cast<char>((i >> 0) & 0xff);
	write(aux, 4);
}

void MsgBase::write(int32_t i)
{
	write(static_cast<uint32_t>(i));
}

void MsgBase::write(uint16_t i)
{
	char aux[2];
	aux[0] = static_cast<char>((i >> 8) & 0xff);
	aux[1] = static_cast<char>((i >> 0) & 0xff);
	write(aux, 2);
}

void MsgBase::write(int16_t i)
{
	write(static_cast<uint16_t>(i));
}

void MsgBase::write(uint8_t i)
{
	write(static_cast<char>(i));
}

void MsgBase::write(int8_t i)
{
	write(static_cast<uint8_t>(i));
}

void MsgBase::write(float f)
{
	write(*(uint32_t*)&f);
}

void MsgBase::write(Vector3 v3)
{
	write(v3.x);
	write(v3.y);
	write(v3.z);
}

void MsgBase::read(char* data, size_t size)
{
	// check if there's enough data in the buffer
	if (!bufferGEThan(sizeof(char)*size))
		return;
	mBuffer.extractFront(data, size);
}

void MsgBase::read(std::string& s)
{
	// mafm: due to the interface available in vector (we cannot use
	// std::string for this), we have to iterate considering the front
	// position only, until we find the null character terminating the
	// string, or else we go out of bounds

	// mafm: needed this because we expect the string to be empty, and it
	// might be not empty using dummy initialization values (<none>, etc).
	// The result expected in the rest of the data types is to overwrite the
	// old value if exists, and replace it with the data that we read, so we
	// want to maintain this consistency :)
	s.clear();

	while (bufferGEThan(sizeof(char))) {
		char c = mBuffer.popFront();
		if (c == '\0')
			return;
		else
			s.append(1, c);
	}
}

void MsgBase::read(char& c)
{
	// check if there's enough data in the buffer
	if (!bufferGEThan(sizeof(char)))
		return;
	c = mBuffer.popFront();
}

void MsgBase::read(bool& b)
{
	char c;
	read(c);
	b = static_cast<bool>(c);
}

void MsgBase::read(uint64_t& i)
{
	char c0, c1, c2, c3, c4, c5, c6, c7;
	read(c0); read(c1); read(c2); read(c3); read(c4); read(c5); read(c6); read(c7);
	i = ( (static_cast<uint64_t>(c0) << 56) & 0xff00000000000000LL )
		| ( (static_cast<uint64_t>(c1) << 48) & 0x00ff000000000000LL )
		| ( (static_cast<uint64_t>(c2) << 40) & 0x0000ff0000000000LL )
		| ( (static_cast<uint64_t>(c3) << 32) & 0x000000ff00000000LL )
		| ( (static_cast<uint64_t>(c4) << 24) & 0x00000000ff000000LL )
		| ( (static_cast<uint64_t>(c5) << 16) & 0x0000000000ff0000LL )
		| ( (static_cast<uint64_t>(c6) << 8)  & 0x000000000000ff00LL )
		| ( static_cast<uint64_t>(c7) & 0x00000000000000ffLL );
}

void MsgBase::read(uint32_t& i)
{
	char c0, c1, c2, c3;
	read(c0); read(c1); read(c2); read(c3);
	i = ( (static_cast<uint32_t>(c0) << 24) & 0xff000000 )
		| ( (static_cast<uint32_t>(c1) << 16) & 0x00ff0000 )
		| ( (static_cast<uint32_t>(c2) << 8) & 0x0000ff00 )
		| ( static_cast<uint32_t>(c3) & 0x000000ff );
}

void MsgBase::read(int32_t& i)
{
	uint32_t aux;
	read(aux);
	i = static_cast<int32_t>(aux);
}

void MsgBase::read(uint16_t& i)
{
	char c0, c1;
	read(c0); read(c1);
	i = ( (static_cast<uint16_t>(c0) << 8) & 0xff00 )
		| ( static_cast<uint16_t>(c1) & 0x00ff );
}

void MsgBase::read(int16_t& i)
{
	uint16_t aux;
	read(aux);
	i = static_cast<int16_t>(aux);
}

void MsgBase::read(uint8_t& i)
{
	char c;
	read(c);
	i = static_cast<uint8_t>(c);
}

void MsgBase::read(int8_t& i)
{
	char c;
	read(c);
	i = static_cast<int8_t>(c);
}

void MsgBase::read(float& f)
{
	uint32_t aux;
	read(aux);
	f = *(float*)&aux;
}

void MsgBase::read(Vector3& v3)
{
	read(v3.x);
	read(v3.y);
	read(v3.z);
}

uint32_t MsgBase::getLength() const
{
	return mBuffer.getSize();
}

const char* MsgBase::getBuffer() const
{
	return mBuffer.getBuffer();
}


//-------------------------- MsgHdlFactory -------------------------
MsgHdlFactory::~MsgHdlFactory()
{
	// delete message objects passed initially to the factory
	while (!factories.empty()) {
		delete factories.begin()->second.first;
		delete factories.begin()->second.second;
		factories.erase(factories.begin());
	}
}

void MsgHdlFactory::registerMsgWithHdl(MsgBase* msg, MsgHdlBase* hdl)
{
	uint32_t key = msg->getType().getID();
	if (factories.find(key) != factories.end()) {
		LogWRN("Msg already registered (type: '%s')",
		       msg->getType().getName());
	} else if (msg->getType() != hdl->getMsgType()) {
		LogERR("Msg types mismatch (msg: '%s', hdl: '%s')",
		       msg->getType().getName(),
		       hdl->getMsgType().getName());
	} else {
		// register the message and handler
		factories[key].first = msg;
		factories[key].second = hdl;
	}
}

MsgBase* MsgHdlFactory::createMsgInstance(uint32_t key)
{
	if (factories.find(key) == factories.end()) {
		LogERR("Msg not found (type: '%s')",
		       MsgType(key).getName());
		return 0;
	} else
		return factories[key].first->createInstance();
}

MsgHdlBase* MsgHdlFactory::getHdl(uint32_t key)
{
	if (factories.find(key) == factories.end()) {
		LogERR("Handler not found (type: '%s')",
		       MsgType(key).getName());
		return 0;
	} else
		return factories[key].second;
}

bool MsgHdlFactory::handleStream(Netlink& netlink,
				 uint32_t key,
				 const char* buffer,
				 uint32_t size)
{
	// mafm: Note that we wouldn't need to create new instances of the
	// message only for this. The current operation is that it creates a
	// message based on the type especified in the stream, and it passes the
	// raw buffer data to deserialize the message, etc.  What we could do
	// instead is to use the instance of the message stored in the factory
	// itself, cleaning up the buffer between calls. This would be very good
	// not only to save memory and time when creating new instances, but to
	// avoid to define the createInstance function in every message: as
	// there are lots of messages it would save quite a lot of typing.
	//
	// The drawback is that we wouldn't have a clean message each time --
	// the buffer would be clear but the variables of the derived messages
	// would not. The solution would be to provide abstract functions for
	// this, in the same way that serializeData and deserializeData, but
	// this is a bigger problem than the time required to type the
	// createInstance function for each type; so at the moment we'll stick
	// with this.

	// see if we have the needed msg+hdl
	MsgBase* msg = createMsgInstance(key);
	MsgHdlBase* hdl = getHdl(key);
	if (!msg || !hdl)
		return false;

	// deserialize and use the message
	msg->deserialize(buffer, size);
	hdl->handleMsg(*msg, &netlink);
	delete msg;

	return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
