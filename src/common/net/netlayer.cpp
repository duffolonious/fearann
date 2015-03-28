/*
 * netlayer.cpp
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

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "msgbase.h"
#include "netlayer.h"

#ifdef WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#endif

#include <netdb.h>
#include <cerrno>
#include <fcntl.h>


/// This is the limit size of the packet (it's 2^16 for TCP, but this sould be
/// enough)
#define PACKET_MAX_SIZE 32768

/// This is the exact size of the ping packet (2 bytes for id, 4 bytes for
/// timestamp)
#define PING_BUFFER_LENGTH 6

/** Misc utilities used in several places, we save code size and maintainance
 * time having them unified.
 */
namespace NetLayerUtils
{
	/** Create a socket checking for errors, and set it to non-blocking. */
	int createTCPNonBlockingSocket()
	{
		// just a wrapper around the system call, for error checking
		int sock = socket(PF_INET, SOCK_STREAM, 0);
		if (sock == -1) {
			LogERR("Creating socket: '%s'", strerror(errno));
		} else {
			// set to non-blocking
			fcntl(sock, F_SETFL, O_NONBLOCK);
		}

		return sock;
	}

	/** Translate hostname to IP, returns pointer 0 if there's an error, the
	 * translated IP otherwise */
	string getHostByName(const char* hostname)
	{
		// mafm: this returns just the first address (h->h_addr) for the
		// hostname, we could do something more elaborate but this is
		// probably enough for our purposes
		LogDBG("Resolving hostname: %s", hostname);
		struct hostent* h = gethostbyname(hostname);
		if (!h) {
			LogERR("gethostbyname: '%s'", strerror(h_errno));
			return string("1.1.1.1");
		} else {
			string ip = inet_ntoa(*((struct in_addr *)h->h_addr));
			return ip;
		}
	}
}


//--------------------------- Netlink -------------------------
Netlink::RawPacket::RawPacket(const char* buffer, size_t s)
{
	size = s;
	data = new char[size];
	memcpy(data, buffer, s);
}

Netlink::RawPacket::~RawPacket()
{
	delete [] data;
}

Netlink::Buffer::Buffer(size_t s) :
	size(s)
{
	buffer = new char[size];
	front = 0;
	back = 0;
}

Netlink::Buffer::~Buffer()
{
	delete [] buffer;
}

size_t Netlink::Buffer::getStreamSize() const
{
	PERM_ASSERT(back >= front && back <= size);
	return (back - front);
}

char* Netlink::Buffer::operator [] (size_t index)
{
	PERM_ASSERT(index <= size);
	return buffer+index;
}

Netlink::Netlink(int socket, const char* ip, int port) :
	mSocket(socket), mIP(ip), mPort(port), mWorkBuffer(PACKET_MAX_SIZE*2)
{
}

Netlink::Netlink() :
	mSocket(0), mIP("<not set>"), mPort(0), mWorkBuffer(PACKET_MAX_SIZE*2)
{
}

Netlink::~Netlink()
{
	while (!mSendQueue.empty()) {
		delete mSendQueue.front();
		mSendQueue.pop_front();
	}

        LogDBG("Statistics: sent (%u p, %u B, %.02f B/p), recv (%u p, %u B, %.02f B/p)",
	       mNetlinkStats.packetsReceived,
	       mNetlinkStats.bytesReceived,
	       static_cast<float>(mNetlinkStats.bytesReceived)/static_cast<float>(mNetlinkStats.packetsReceived),
	       mNetlinkStats.packetsSent,
	       mNetlinkStats.bytesSent,
	       static_cast<float>(mNetlinkStats.bytesSent)/static_cast<float>(mNetlinkStats.packetsSent));
}

void Netlink::disconnect()
{
	// don't act if the socket is not valid
	if (mSocket == 0)
		return;

	// do close
	int result = close(mSocket);
	if (result == -1) {
		LogERR("close: %s", strerror(errno));
	}

	// mark as closed, for isConnected and similar tests
	mSocket = 0;
}

size_t Netlink::getBytesInSendQueue()
{
	size_t totalSize = 0;
	for (size_t i = 0; i < mSendQueue.size(); ++i) {
		totalSize += mSendQueue[i]->size;
	}
	return totalSize;
}

bool Netlink::recvAvailableData(char* rawRecvBuffer, size_t& bytesRead)
{
	// mafm: Instead of reading al the available data from the socket, if
	// any, and push them into the buffer, it seems to be better to read
	// only once and process the messages right away. In this way the buffer
	// tends to be shorter and thus the algorithms of other parts work much
	// faster, under heavy load.

	int rec = recv(mSocket, rawRecvBuffer, PACKET_MAX_SIZE, 0);
	if (rec == 0) {
		// peer disconnected
		bytesRead = 0;
		return false;
	} else if (rec < 0) {
		// no data available, haven't read anything
		bytesRead = 0;
		return true;
	} else {
		// read some data
		bytesRead = static_cast<size_t>(rec);
		return true;
	}
}

bool Netlink::processIncomingMsgs(MsgHdlFactory& factory)
{
	// mafm: Note that returning false means that the peer closed the
	// socket, so we must close our end too, so be careful.  The
	// implementation of this function is somewhat tricky, but I couldn't
	// find a cleaner solution which were somewhat performant.

	// we first define local buffers to be used with recv()
	static Buffer recvBuffer(PACKET_MAX_SIZE);
	static size_t bytesRead = 0;

	// move the remaining data from previous messages to the beginning of
	// the buffer, to make sure that we'll have room for the rest of
	// operations
	if (mWorkBuffer.front != 0) {
		PERM_ASSERT(mWorkBuffer.getStreamSize() <= PACKET_MAX_SIZE);
		memcpy(recvBuffer[0],
		       mWorkBuffer[mWorkBuffer.front],
		       mWorkBuffer.getStreamSize());
		memcpy(mWorkBuffer[0],
		       recvBuffer[0],
		       mWorkBuffer.getStreamSize());
		mWorkBuffer.back = mWorkBuffer.getStreamSize();
		mWorkBuffer.front = 0;
	}
		
	// check if we received something, otherwise stop here
	bool result = recvAvailableData(recvBuffer[0], bytesRead);
	if (!result) {
		// peer disconnected
		return false;
	} else if (result && bytesRead == 0) {
		// we don't have data to process
		return true;
	}

	// append to work buffer (to process the stream "unsliced")
	PERM_ASSERT(mWorkBuffer.front == 0);
	memcpy(mWorkBuffer[mWorkBuffer.back], recvBuffer[0], bytesRead);
	mWorkBuffer.back += bytesRead;

	// loop while there's data enough to process new messages
	while (mWorkBuffer.getStreamSize() >= sizeof(uint16_t)+sizeof(uint32_t)) {
		uint16_t nextMsgSize = ( (*mWorkBuffer[mWorkBuffer.front] << 8) & 0xff00 )
			| ( (*mWorkBuffer[mWorkBuffer.front+1] & 0xff) );

		if (nextMsgSize > mWorkBuffer.getStreamSize()) {
			// there's not enough data, wait for more
			/*
			LogDBG("Not enough data: next message size %u, data avail. %u",
			       nextMsgSize, mWorkBuffer.getStreamSize());
			*/
			return true;
		} else {
			// get the message type
			char type[5] = "init";
			type[0] = *mWorkBuffer[mWorkBuffer.front+2];
			type[1] = *mWorkBuffer[mWorkBuffer.front+3];
			type[2] = *mWorkBuffer[mWorkBuffer.front+4];
			type[3] = *mWorkBuffer[mWorkBuffer.front+5];
			MsgType nextMsgType(type);

			/*
			LogDBG("Packet type='%s' size=%u found, %u bytes in buffer",
			       nextMsgType.getName(), nextMsgSize,
			       mWorkBuffer.getStreamSize());
			*/

			// handle the message based on the given factory
			factory.handleStream(*this,
					     nextMsgType.getID(),
					     &mWorkBuffer.buffer[mWorkBuffer.front],
					     nextMsgSize);

			// remove the message from the buffer
			if (nextMsgSize == mWorkBuffer.getStreamSize()) {
				mWorkBuffer.front = 0;
				mWorkBuffer.back = 0;
			} else {
				mWorkBuffer.front += nextMsgSize;
				PERM_ASSERT(mWorkBuffer.back >= mWorkBuffer.front);
			}
	      
			// update the stats
			++mNetlinkStats.packetsReceived;
			mNetlinkStats.bytesReceived += nextMsgSize;
		}
	}

	// not enough data to continue processing, return true
	return true;
}

bool Netlink::sendMsg(MsgBase& msg)
{
	// mafm: we could maybe improve this (provided that copying buffers up
	// to 32KB and typically of tenths or hundreds of bytes make any
	// different in this kind of program) by not copying into the queue if
	// there's no queued data, but the logic of the function gets dirty (it
	// has to be copied later if the message can't be sent, etc) and the
	// code bigger, so at the moment the implementation is the simpler one

	// prepare the message
	msg.serialize();

	// check if the packet is longer than the limit for the packet
	if (msg.getLength() > PACKET_MAX_SIZE)
		return false;

	/*
	LogDBG("Sending packet %s to socket %d (IP='%s') (length %u)",
	       msg.getType().getName(),
	       getSocket(), getIP(), msg.getLength());
	*/

	mSendQueue.push_back(new RawPacket(msg.getBuffer(), msg.getLength()));

	// update the stats
	++mNetlinkStats.packetsSent;

	// try to process immediately
	processOutgoingMsgs();

	return true;
}

void Netlink::processOutgoingMsgs()
{
	// mafm: In general we don't care about errors, because if the peer
	// closed the socket or some similar problem, we'll catch it in the next
	// round of processIncoming.  The problem is that send returns error
	// EWOULDBLOCK a lot of times in example when sending the content and
	// you're in a local system, so we have to ignore it to not flood the
	// logs.

	// loop to send the maximum data possible
	while (!mSendQueue.empty()) {
		// just send a fragment
		int sent = send(getSocket(),
				mSendQueue[0]->data,
				mSendQueue[0]->size,
				MSG_NOSIGNAL);
		if (sent == static_cast<int>(mSendQueue[0]->size)) {
			// packet sent fully, remove from queue
			RawPacket* full = mSendQueue[0];
			mSendQueue.pop_front();
			delete full;

			// update the stats
			mNetlinkStats.bytesSent += sent;
		} else if (sent > 0) {
			// remove the part sent (substituting for a new packet
			// really). it's a bit expensive, but if the next
			// iteration results in error, it's difficult to
			// recover...
			RawPacket* part = new RawPacket(mSendQueue[0]->data + sent,
							mSendQueue[0]->size - sent);
			RawPacket* full = mSendQueue[0];
			mSendQueue.pop_front();
			mSendQueue.insert(mSendQueue.begin(), part);
			delete full;

			// update the stats
			mNetlinkStats.bytesSent += sent;
		} else {
			// see comment in the beginning
			// LogDBG("send: %s", strerror(errno));
			return;
		}
	}
}

bool Netlink::operator == (const Netlink& other) const
{
	return (mSocket == other.mSocket
		&& mIP == other.mIP
		&& mPort == other.mPort);
}

bool Netlink::operator != (const Netlink& other) const
{
	return ! (mSocket == other.mSocket);
}

bool Netlink::isConnected() const
{
	return (mSocket != 0);
}

int Netlink::getSocket() const
{
	return mSocket;
}

void Netlink::setSocket(int socket)
{
	mSocket = socket;
}

const char* Netlink::getIP() const
{
	return mIP.c_str();
}

void Netlink::setIP(const char* IP)
{
	mIP = IP;
}

uint16_t Netlink::getPort() const
{
	return mPort;
}

void Netlink::setPort(uint16_t port)
{
	mPort = port;
}


//--------------------------- SocketLayer -------------------------
SocketLayer::SocketLayer(Netlink* netlink) :
	mBaseNetlink(netlink)
{
}

SocketLayer::~SocketLayer()
{
	disconnect();
	mBaseNetlink = 0;
}

bool SocketLayer::isConnected() const
{
	return mBaseNetlink->isConnected();
}

bool SocketLayer::listenForClients(const char* addr, int port, int backlog)
{
	// get a new socket
	int listener = NetLayerUtils::createTCPNonBlockingSocket();
	mBaseNetlink->setSocket(listener);

	// fill in the netlink data
	mBaseNetlink->setIP(addr);
	mBaseNetlink->setPort(port);

	// set up the address to listen to
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = inet_addr(addr);
	memset(&(my_addr.sin_zero), '\0', 8);

	// set SO_REUSEADDR on a socket to true
	int optval = 1;
	int result = setsockopt(listener,
				SOL_SOCKET,
				SO_REUSEADDR,
				&optval,
				sizeof(int));
	if (result == -1) {
		LogERR("setsockopt: '%s'", strerror(errno));
		return false;
	}

	// bind the socket to the address:port
	result = bind(listener,
		      (struct sockaddr *)&my_addr,
		      sizeof(struct sockaddr));
	if (result == -1) {
		LogERR("bind: %s", strerror(errno));
		return false;
	}

	// start to listen
	result = listen(listener, backlog);
	if (result == -1) {
		LogERR("listen: %s", strerror(errno));
		return false;
	}

	LogNTC("Succesfully listening to %s:%d, backlog %d",
	       addr, port, backlog);

	return true;
}

bool SocketLayer::connectToServer(const char* addr, int port, time_t timeout)
{
	// if we're given an hostname, convert automagically
	string ip;
	if (isalpha(addr[0])) {
		ip = NetLayerUtils::getHostByName(addr);
	} else {
		ip = addr;
	}

	// get a new socket
	int listener = NetLayerUtils::createTCPNonBlockingSocket();
	mBaseNetlink->setSocket(listener);

	LogNTC("Trying to connect to %s:%d (socket=%d)",
	       addr, port, listener);

	// set up the server address and connect
	struct sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	memset(&(dest_addr.sin_zero), '\0', 8);
	connect(listener, (struct sockaddr*)(&dest_addr), sizeof(struct sockaddr));

	// poll to see if the connection was successful or not
	struct pollfd ufds[1];
	ufds[0].fd = listener;
	ufds[0].events = POLLOUT;
	int result = poll(ufds, 1, 5*1000); // milliseconds
	if (result > 0 && ufds[0].revents & POLLOUT) {
		// final setup
		mBaseNetlink->setIP(ip.c_str());
		mBaseNetlink->setPort(port);
		LogNTC("Succesfully connected to %s:%d (socket=%d)",
		       addr, port, listener);
		return true;
	} else {
		// we didn't get a reply in the timeout interval
		LogERR("poll (timeout %lds): %s", timeout, strerror(errno));
		mBaseNetlink->setSocket(0);
		close(listener);
		return false;
	}
}

bool SocketLayer::acceptIncoming(int& socket, string& ip, int& port)
{
	// connector's address information
	struct sockaddr_in incoming_addr;
	socklen_t sin_size = sizeof(struct sockaddr_in);
	// avoid valgrind complaints
	incoming_addr.sin_addr.s_addr = 0L;

	// values only valid if function returns true
	socket = accept(mBaseNetlink->getSocket(),
			(struct sockaddr*)&incoming_addr,
			&sin_size);
	ip = inet_ntoa(incoming_addr.sin_addr);
	port = incoming_addr.sin_port;

	return (socket != -1);
}

void SocketLayer::disconnect()
{
	mBaseNetlink->disconnect();
}


//--------------------------- PingServer -------------------------
PingServer::~PingServer()
{
	for (list<int>::iterator it = mPingClients.begin();
	     it != mPingClients.end(); ++it) {
		close(*it);
	}
	close(mPingListener);
}

bool PingServer::listenForClients(const char* addr, int port, int backlog)
{
	// get a new socket
	mPingListener = NetLayerUtils::createTCPNonBlockingSocket();

	// set up the address to listen to
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = inet_addr(addr);
	memset(&(my_addr.sin_zero), '\0', 8);

	try {
		// set SO_REUSEADDR on a socket to true
		int optval = 1;
		int result = setsockopt(mPingListener,
					SOL_SOCKET,
					SO_REUSEADDR,
					&optval,
					sizeof(int));
		if (result == -1)
			throw "setsockopt";

		// bind the socket to the address:port
		result = bind(mPingListener,
			      (struct sockaddr *)&my_addr,
			      sizeof(struct sockaddr));
		if (result == -1)
			throw "bind";

		// start to listen
		result = listen(mPingListener, backlog);
		if (result == -1)
			throw "listen";
	} catch (const char* service) {
		LogERR("%s: '%s'", service, strerror(errno));
		return false;
	}

	LogNTC("Succesfully setting up %s:%d for ping server",
	       addr, port);
	return true;
}

void PingServer::acceptIncoming()
{
	// connector's address information
	struct sockaddr_in incoming_addr;
	socklen_t sin_size = sizeof(struct sockaddr_in);

	// check if we have incoming connections
	int socket = accept(mPingListener,
			    (struct sockaddr *)&incoming_addr,
			    &sin_size);
	if (socket != -1) {
		mPingClients.push_back(socket);
		string ip = inet_ntoa(incoming_addr.sin_addr);
		int port = incoming_addr.sin_port;
		LogNTC("Incoming ping connection: %s:%d", ip.c_str(), port);
	}
}

void PingServer::processPingRequests()
{
	static char buffer[PING_BUFFER_LENGTH];

	// loop through the existing connections to know if we received anything
	for (list<int>::iterator it = mPingClients.begin();
	     it != mPingClients.end(); ++it) {
		int rec = recv(*it, buffer, sizeof(buffer), 0);
		if (rec == 0) {
			// peer disconnected
			it = mPingClients.erase(it);
		} else if (rec < 0) {
			// no data available, haven't read anything
		} else if (rec == 6) {
			// we found a proper ping request
			// LogDBG("Ping request from socket=%d: %d received", *it, rec);

			// send a reply
			int sent = send(*it,
					buffer,	PING_BUFFER_LENGTH,
					MSG_NOSIGNAL);
			if (sent == 6) {
				// packet sent fully
			} else {
				LogDBG("Couldn't send ping reply for socket=%d: %d sent",
				       *it, sent);
			}
		} else {
			// read some data
			LogDBG("Bad incoming ping data socket=%d: %d received",
			       *it, rec);
		}
	}
}


//--------------------------- PingClient -------------------------
PingClient::~PingClient()
{
	for (list<PingServerEntry>::iterator it = mPingServers.begin();
	     it != mPingServers.end(); ++it) {
		close(it->socket);
	}
}

void PingClient::addServer(const char* addr, int port, uint16_t id)
{
	// if we're given an hostname, convert automagically
	string ip;
	if (isalpha(addr[0])) {
		ip = NetLayerUtils::getHostByName(addr);
	} else {
		ip = addr;
	}

	// get a new socket
	int serverSocket = NetLayerUtils::createTCPNonBlockingSocket();

	// set up the address to listen to
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	memset(&(server_addr.sin_zero), '\0', 8);

	try {
		// set SO_REUSEADDR on a socket to true
		int optval = 1;
		int result = setsockopt(serverSocket,
					SOL_SOCKET,
					SO_REUSEADDR,
					&optval,
					sizeof(int));
		if (result == -1)
			throw "setsockopt";

		// connect the socket to the address:port. we don't care about
		// errors, because since it's blocking we would have to poll it.
		// it will fail later when trying to send, anyway
		connect(serverSocket,
			(struct sockaddr*)(&server_addr),
			sizeof(struct sockaddr));
	} catch (const char* service) {
		LogERR("%s: '%s'", service, strerror(errno));
		return;
	}

	mPingServers.push_back(PingServerEntry(id, serverSocket));
	LogNTC("Succesfully setting up socket to ping server %s:%d",
	       addr, port);
}

void PingClient::collectPingReplyData(list<PingServerEntry>& pingReplies)
{
	pingReplies = mPingServers;
}

void PingClient::sendPings(uint32_t timestamp)
{
	static char buffer[PING_BUFFER_LENGTH];

	for (list<PingServerEntry>::iterator it = mPingServers.begin();
	     it != mPingServers.end(); ++it) {
		// skip invalid sockets
		if (it->socket == -1)
			continue;

		// prepare buffer
		buffer[0] = (it->id >> 8);
		buffer[1] = it->id & 0x00ff;
		buffer[2] = (timestamp >> 24) & 0x000000ff;
		buffer[3] = (timestamp >> 16) & 0x000000ff;
		buffer[4] = (timestamp >> 8) & 0x000000ff;
		buffer[5] = timestamp & 0x000000ff;

		// send a ping packet
		int sent = send(it->socket,
				buffer,
				PING_BUFFER_LENGTH,
				MSG_NOSIGNAL);
		if (sent == 6) {
			// packet sent fully
		} else {
			LogDBG("Couldn't send ping for socket=%d id=%u: %d sent",
			       it->socket, it->id, sent);
			
		}
	}
}

void PingClient::processPingReplies()
{
	static char buffer[PING_BUFFER_LENGTH];

	for (list<PingServerEntry>::iterator it = mPingServers.begin();
	     it != mPingServers.end(); ++it) {
		// attempt to read data
		int rec = recv(it->socket, buffer, sizeof(buffer), 0);
		if (rec == 0) {
			// peer disconnected
			LogDBG("Ping reply for socket=%d id=%u disconnected, removing",
			       it->socket, it->id);
			it->socket = -1;
		} else if (rec < 0) {
			// no data available, haven't read anything
			it->timestamp = 0;
		} else if (rec == 6) {
			// read the reply
			uint16_t id = ((buffer[0] << 8) & 0xff00)
				+ ((buffer[1]) & 0x00ff);
			uint32_t timestamp =
				((buffer[2] << 24) & 0xff000000)
				+ ((buffer[3] << 16) & 0x00ff0000)
				+ ((buffer[4] << 8) & 0x0000ff00)
				+ ((buffer[5]) & 0x000000ff);
			it->timestamp = timestamp;
			if (it->id != id)
				LogDBG("Ping reply should be valid, but id differs!! (original=%u, reply=%u)",
				       it->id, id);
		} else {
			// not enough data
			LogDBG("Ping reply for socket=%d id=%u with %d (expected exact ping size)",
			       it->socket, it->id, rec);
			it->timestamp = 0;
		}
	}
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
