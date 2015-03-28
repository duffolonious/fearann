/*
 * netlayer.h
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

#ifndef __FEARANN_COMMON_NET_NETLAYER_H__
#define __FEARANN_COMMON_NET_NETLAYER_H__


/** \file netlayer
 *
 * The classes contained here are directly related with the network layer, and
 * these are the socket layer (which abstracts the operations in the sockets, in
 * example, sends a message fully and not in chunks) and the netlink
 * representation, which is what we need to know in the application (socket
 * related with a connection, queue of data received, etc).
 *
 * Typically, the client should create a base netlink which will be the only
 * one, and it's the connection with the server, all the communication will use
 * this channel. In the server, the base netlink is the one which listens for
 * new connections, and then there's a new netlink for each incoming connection
 * (potential player). So client and server should create the netlink and
 * socketlayer objects, and then just use the socketlayer to perform the
 * operations (retrieve data for the netlink, send a message through the given
 * netlink, etc).
 *
 * Why do this, instead of using a shiny and fully featured network library?
 * Well, at the time of writing this it seems that there are no suitable network
 * libraries availabe. On one hand we have simple netowork libraries which are
 * mostly wrappers around the usual sockets, in this case our simple abstraction
 * seems to be enough and so we save to require yet another freaking external
 * library to build the project. On the other hand we have Raknet, which is
 * probably the most famous network library available out there with free
 * software licences, but it turned to be infamous after a year using it in
 * other projects: half of the versions don't compile straight in GNU/Linux, it
 * usually has serious bugs such as sending data over the wrong connection, or
 * accepting duplicate packets. The code is formatter so awkfully that it's
 * hardly readable, and the coding practices (although using C++) are not
 * Object-Oriented in many parts -- if somebody can't write readable code, it's
 * not very sensible to expect that it works reliably.
 *
 * Anyway, why to use TCP instead of UDP for the tailored solution?  UDP is
 * supposed to be used mainly in two situations: 1) When the data doesn't need
 * to get there, non-reliable packets; 2) When the TCP model to retransmit
 * packets is not good enough because of timing issues. In this project:
 *
 * 1) Building unpredictable systems is much more difficult. If all the data is
 * reliable and comes in the correct order, you avoid a lot of problems: in
 * example, when you receive packets telling you the position of other player
 * has changed, you just process them in order, no big deal. If the packets come
 * out of order, you have to find a way to know if the message that you're
 * receiving is previous to the last one processed: in this case the player
 * would move 2m forward and then 1m backward again. So you need to send more
 * data, create additional protocols for different classes of packets, etc --
 * much more work, and much more prone to errors. After all, most of the data
 * that we send is supposed to be reliable, and it must came in order, even if
 * they are chat messages or notifications about friends on-line. Introducing a
 * lot of complexity just to save a few bytes here and there is not worth the
 * effort.
 *
 * 2) This is not a First Person Shooter, nor is intended to be the best game
 * ever (not in this decade anyway :)) and the big commercial success of the XXI
 * century. FPS need a lot of accuracy (so the delays in network messages are
 * critical) to be playable, and servers with tenths of thousands of players
 * too, they need to priorize some packets over another in order to avoid
 * undesirable effects (such as players jumping from one place to another). But
 * this game should need about the same responsiveness as you have with remote
 * terminal sessions, and TCP has been doing this in the last decades. It's very
 * hard to make a new protocol and make it work as reliably and more responsive
 * than TCP, so until proven otherwise our implementation will be with TCP. It
 * can be changed easily anyway, if there are evidences that it's causing
 * problems.
 */

#include <string>
#include <deque>
#include <list>

class MsgBase;
class MsgHdlFactory;


/** Representation of a link from the application point of view, containing the
 * socket, stream received from that connection not yet processed (converted to
 * messages), and in general all the data that we may need about each
 * connection.  The client will have only one netlink (the connection with the
 * server), the server will have as many netlinks as clients.
 *
 * Another functionality is to abstract the send/recv operations. This class
 * ensures that a message is fully sent independently of the protocol used in
 * the lower layers. A similar approach is used for incoming data: an operation
 * reads all the available data, and after that extracts from the incoming
 * buffer those messages that we received completely, and returns the result of
 * the operation to the caller.
 */
class Netlink
{
public:
	Netlink(int socket, const char* ip, int port);
	Netlink();
	~Netlink();

	/** Return whether or not we're connected */
	bool isConnected() const;
	/** Disconnect */
	void disconnect();

	/** Process the existing message in the incoming stream (if any) */
	bool processIncomingMsgs(MsgHdlFactory& factory);
	/** Process the messages in the outgoing queue (if any).  Sometimes the
	 * data cannot be sent because the client can't process the data quickly
	 * enough.  In these cases, we need to queue the data and send it later,
	 * this function should probably be called in the same loop as
	 * processIncoming (to save cycles looping through all the connections).
	 * Each new message calls this function anyway, to try to send the data
	 * immediately. */
	void processOutgoingMsgs();

	/** Send a mesage to the peer (it puts the message in the queue and
	 * tries to send data right away) */
	bool sendMsg(MsgBase& msg);

	/** Get bytes in send queue, to see if we should queue more or not (in
	 * example when sending files) */
	size_t getBytesInSendQueue();
  
	/** Operator to compare two connections */
	bool operator == (const Netlink& other) const;
	/** Operator to compare two connections */
	bool operator != (const Netlink& other) const;

	/** To get the socket of this connection */
	int getSocket() const;
	/** To set the socket of this connection */
	void setSocket(int socket);
	/** To get the IP of this connection */
	const char* getIP() const;
	/** To set the IP of this connection */
	void setIP(const char* IP);
	/** To get the port of this connection */
	uint16_t getPort() const;
	/** To set the port of this connection */
	void setPort(uint16_t port);

private:
	/// The socket itself
	int mSocket;
	/// The IP of the connection
	std::string mIP;
	/// The port of the connection
	uint16_t mPort;

	/** Raw packet (message) to be sent
	 */
	class RawPacket {
	public:
		RawPacket(const char* buffer, size_t s);
		~RawPacket();
		size_t size;
		char* data;
	};

	/// Buffer to store the data to be sent, the rest of the considerations
	/// are the same as the ones for receive-buffers
	std::deque<RawPacket*> mSendQueue;

	/** Buffers to store the received data, waiting to be provided to a
	 * message to be deserialized.
	 */
	class Buffer {
	public:
		Buffer(size_t s);
		~Buffer();
		size_t getStreamSize() const;
		char* operator [] (size_t index);

		char* buffer;
		const size_t size;
		size_t front;
		size_t back;
	};

	/// Work buffer (to store the unprocessed data and so on).  It has to be
	/// able to contain two full packets.
	Buffer mWorkBuffer;

	/// Statistics for the connection
	class NetLinkStats {
	public:
		NetLinkStats() {
			packetsReceived = 0; bytesReceived = 0; 
			packetsSent = 0; bytesSent = 0;
		}
		uint32_t packetsReceived;
		uint32_t bytesReceived;
		uint32_t packetsSent;
		uint32_t bytesSent;
	} mNetlinkStats;

	/** Function to receive all the available data. If a whole packet is
	 * received (or more than one), it will put it in the queue of packets
	 * of the netlink.  Returns false if peer disconnected, true
	 * otherwise. */
	bool recvAvailableData(char* rawRecvBuffer, size_t& bytesRead);
};


/** Simple abstraction of the sockets below this interface. This class performs
 * some basic actions that both client and server need to use, such as
 * connecting a socket to a remote server or listening to some port of the
 * machine for incoming connections; so what client and server should do is to
 * create an object from this class, and use the operations that they need.
 *
 * The argument of the constructor is the netlink that will be used as base for
 * the operations (the socket connected to the server in client, the listener in
 * server).
 */
class SocketLayer
{
public:
	/** Default constructor */
	SocketLayer(Netlink* netlink);
	/** Destructor */
	~SocketLayer();

	/** Function to use in the server to begin to listen for
	 * clients. Basically, this is all that a server needs to begin
	 * listening, after that it can check for incoming data and connections,
	 * or send data. */
	bool listenForClients(const char* addr, int port, int backlog);

	/** Function for the server to check for new connections. */
	bool acceptIncoming(int& socket, std::string& ip, int& port);

	/** Function to use in the clients to connect to the server. Basically,
	 * this is all that the client needs to do before starting to send and
	 * receive data. */
	bool connectToServer(const char* addr, int port, time_t timeout=5);

	/** Function to use in clients and server to disconnect: stop listening
	 * from clients in server, stop being connected to the server in
	 * clients */
	void disconnect();
	/** Return whether or not we're connected */
	bool isConnected() const;

private:
	/// The base socket to listen to (it's the socket used to 'listen' in a
	/// server, the socket used to 'connect' in the clients)
	Netlink* mBaseNetlink;
};


/** Abstraction for the ping server.  This opens a TCP socket and listens for
 * clients, when receives some client adds the incoming socket to a list, and
 * preriodically reads the socket to see if there's incoming data.  When there's
 * data it just bounces back the data to the client, so it can measure the time
 * that it took to receive the reply.
 */
class PingServer
{
public:
	/** Destructor */
	~PingServer();

	/** Listen for clients to connect */
	bool listenForClients(const char* addr, int port, int backlog);
	/** Accept incoming connections */
	void acceptIncoming();
	/** Process ping requests */
	void processPingRequests();

private:
	/// The base socket to listen to (it's the socket used to 'listen' in a
	/// server, the socket used to 'connect' in the clients)
	Netlink* mBaseNetlink;
	/// The socket to listen for ping requests
	int mPingListener;
	/// List of sockets connected to ping server
	std::list<int> mPingClients;
};


/** Abstraction for the ping client.  It has to create a connection with each
 * server; and for those who accept connection, start sending packets regularly,
 * and waiting for a reply.  When the reply arrives, we have the identification
 * of the server and the timestamp that we sent; so we can calculate which is
 * the delay to each server.
 */
class PingClient
{
public:
	/** Helper class to store servers and results */
	class PingServerEntry {
	public:
		PingServerEntry(uint16_t i, int s) :
			id(i), timestamp(0), socket(s) { }
		uint16_t id;
		uint32_t timestamp;
		int socket;
	};

public:
	/** Destructor */
	~PingClient();

	/** Add a server */
	void addServer(const char* addr, int port, uint16_t id);
	/** Collect ping reply data */
	void collectPingReplyData(std::list<PingServerEntry>& pingReplies);
	/** Send pings to servers */
	void sendPings(uint32_t timestamp);
	/** Process ping replies */
	void processPingReplies();

private:
	/// List of sockets of ping servers
	std::list<PingServerEntry> mPingServers;
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
