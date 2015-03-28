/*
 * srvnetworkmgr.h
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

/// @defgroup srvnet Server Network Group
/// This group contains all network related classes in the server.
/// @{

#ifndef __FEARANN_SERVER_NET_MGR_H__
#define __FEARANN_SERVER_NET_MGR_H__


#include "common/patterns/singleton.h"
#include "common/net/netlayer.h"
#include "common/net/msgbase.h"


#include <vector>
#include <list>

class LoginData;


/** Network manager for the server, abstracting all the operations.  The rest of
 * the application shouldn't worry about this, except for connecting and sending
 * messages, and very little else.
 *
 * This class is also responsible for calling the appropriate modules when
 * messages requiring server's attention are received.  The application has to
 * call this manager every frame, or with some other frequency, so it can
 * process the incoming data.
 */
class SrvNetworkMgr : public Singleton<SrvNetworkMgr>
{
public:
	/** Finalize, do whatever cleanup needed when the server shuts down. */
	void finalize();

	/** Returns whether we are still connected to server. */
	bool isConnected();

	/** Start listening for clients */
	bool startListening(const char* address, int port, int maxPlayers);

	/** Disconnect a player from the server.  Call this function only if the
	 * decision to disconnect comes from some manager, not if we detect and
	 * stalled connection because the peer closed it. */
	void disconnectPlayer(Netlink& netlink);

	/** Send a message to a netlink connection (at least useful for ping
	 * replies, when we don't even create a LoginData for them) */
	void sendToConnection(MsgBase& msg, Netlink* netlink);
	/** Send a message to a player */
	void sendToPlayer(MsgBase& msg, const LoginData* loginData);
	/** Send a message to the given list of players */
	void sendToPlayerList(MsgBase& msg, std::vector<LoginData*>& playerList);
	/** Send a message to all players (not connections) */
	void sendToAllPlayers(MsgBase& msg);
	/** Send a message to all players (not connections), except for one
	 * player */
	void sendToAllButPlayer(MsgBase& msg, const LoginData* loginData);
	/** Send a message to all connections, no matter whether they're
	 * playing or not */
	void sendToAllConnections(MsgBase& msg);

	/** Process incoming messages, called from the main app every frame or
	 * at least with some regularity */
	void processIncomingMsgs();

private:
	/** Singleton friend access */
	friend class Singleton<SrvNetworkMgr>;

	/// Connection object (with data about us and the peer, the server)
	Netlink mNetlink;

	/// Network layer
	SocketLayer mSocketLayer;

	/// The message and handler factory
	MsgHdlFactory mMsgHdlFactory;

	/// The list of connections (clients)
	std::list<Netlink*> mConnList;

	/// The handler for incoming ping requests
	PingServer mPingServer;

	/// Maximum number of players accepted
	uint32_t mMaxPlayers;


	/** Default constructor */
	SrvNetworkMgr();

	/** Register message handlers, called once to set it up */
	void registerMsgHdls();
};


#endif

/// @}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
