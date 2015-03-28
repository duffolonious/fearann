/*
 * srvnetworkmgr.cpp
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

#include "common/net/msgs.h"
#include "common/configmgr.h"

#include "server/world/srvworldmgr.h"
#include "server/login/srvloginmgr.h"
#include "server/content/srvcontentmgr.h"

#include "server/net/srvmsghdls.h"

#include "srvnetworkmgr.h"

#include <cstdlib>
#include <fcntl.h>


//---------------------------- SrvNetworkMgr ---------------------
template <> SrvNetworkMgr* Singleton<SrvNetworkMgr>::INSTANCE = 0;

SrvNetworkMgr::SrvNetworkMgr() :
	mSocketLayer(&mNetlink), mMaxPlayers(0)
{
	registerMsgHdls();

	// Start listening on the network
	string address = ConfigMgr::instance().getConfigVar("Server.Network.Address", "-");
	int port = atoi(ConfigMgr::instance().getConfigVar("Server.Network.Port", "-1"));
	int maxPlayers = atoi(ConfigMgr::instance().getConfigVar("Server.Network.MaxPlayers", "-1"));
	if (address == "-" || port == -1 || maxPlayers == -1) {
		LogERR("Unable to load network-related values from cfg file");
	} else if (!startListening(address.c_str(), port, maxPlayers)) {
		LogERR("Unable to start listening: %s:%d, maxplayers=%d",
		       address.c_str(), port, maxPlayers);
	}
}

void SrvNetworkMgr::finalize()
{
	while (!mConnList.empty()) {
		mConnList.front()->disconnect();
		delete mConnList.front();
		mConnList.pop_front();
	}

	mSocketLayer.disconnect();
}

void SrvNetworkMgr::registerMsgHdls()
{
#define	REGHDL(msg, hdl) mMsgHdlFactory.registerMsgWithHdl(new msg, new hdl);

	// connection messages
	REGHDL(MsgConnect, MsgHdlConnect);
	REGHDL(MsgLogin, MsgHdlLogin);
	REGHDL(MsgNewUser, MsgHdlNewUser);
	REGHDL(MsgNewChar, MsgHdlNewChar);
	REGHDL(MsgDelChar, MsgHdlDelChar);
	REGHDL(MsgJoin, MsgHdlJoin);

	// console
	REGHDL(MsgChat, MsgHdlChat);
	REGHDL(MsgCommand, MsgHdlCommand);

	// dialog
	REGHDL(MsgNPCDialog, MsgHdlNPCDialog);
	REGHDL(MsgNPCDialogReply, MsgHdlNPCDialogReply);

	// contact list
	REGHDL(MsgContactAdd, MsgHdlContactAdd);
	REGHDL(MsgContactDel, MsgHdlContactDel);

	// content
	REGHDL(MsgContentQueryUpdate, MsgHdlContentQueryUpdate);

	// entities
	REGHDL(MsgEntityMove, MsgHdlEntityMove);

	// inventory
	REGHDL(MsgInventoryGet, MsgHdlInventoryGet);
	REGHDL(MsgInventoryDrop, MsgHdlInventoryDrop);

	// trading
	REGHDL(MsgTrade, MsgHdlTrade);

	// combat
	REGHDL(MsgCombat, MsgHdlCombat);
	REGHDL(MsgCombatAction, MsgHdlCombatAction);
}

bool SrvNetworkMgr::isConnected()
{
	return mSocketLayer.isConnected();
}

bool SrvNetworkMgr::startListening(const char* host, int port, int maxPlayers)
{
	mMaxPlayers = maxPlayers;

	if (mSocketLayer.isConnected()) {
		LogWRN("Already listening, refusing to connect again");
		return false;
	} else {
		// the third parameter is the number of connections waiting
		// (backlog)
		bool resultGame = mSocketLayer.listenForClients(host, port, 8);
		bool resultPing = mPingServer.listenForClients(host, port-1, 8);
		if (!resultGame || !resultPing)
			LogERR("Couldn't set up Game or Ping listeners");
		return resultGame && resultPing;
	}
}

void SrvNetworkMgr::disconnectPlayer(Netlink& netlink)
{
	// mafm: do not call this if we detect the disconnection trying to
	// process the incoming data, it's only to be used when the decision
	// comes from other managers.

	for (list<Netlink*>::iterator it = mConnList.begin();
	     it != mConnList.end(); ++it) {
		if (**it == netlink) {
			LogERR("Disconnecting player: %s:%d",
			       netlink.getIP(), netlink.getPort());
			mConnList.erase(it);
			netlink.disconnect();
			delete &netlink;
			return;
		}
	}

	// not found
	LogERR("Couldn't find netlink when trying to disconnect player: %s:%d",
	       netlink.getIP(), netlink.getPort());
}

void SrvNetworkMgr::processIncomingMsgs()
{
	if (!mSocketLayer.isConnected()) {
		// we just return without printing anything because otherwise it
		// floods the client log until we get connected for the first
		// time
		return;
	}

	// attend ping requests
	mPingServer.acceptIncoming();
	mPingServer.processPingRequests();

	// to accept incoming connections, just 1 for loop so we autoprotect
	// from floods
	int socket = 0, port = 0; 
	string ip;
	bool result = mSocketLayer.acceptIncoming(socket, ip, port);
	if (result) {
		if (mConnList.size() >= mMaxPlayers) {
			LogWRN("MaxPlayers=%d reached, rejecting connection: %d (IP %s, port %d)",
			       mMaxPlayers, socket, ip.c_str(), port);
			close(socket);
		} else {
			LogDBG("Accepting incoming connection: %d (IP: %s, port %d)",
			       socket, ip.c_str(), port);
			fcntl(socket, F_SETFL, O_NONBLOCK);
			Netlink* netlink = new Netlink(socket, ip.c_str(), port);
			mConnList.push_back(netlink);
		}
        }

	// loops through all the connections to see if they send anything new
	// (and if the connection was closed, which is done with the result of
	// the recv() system call).
	for (list<Netlink*>::iterator it = mConnList.begin();
	     it != mConnList.end(); ++it) {
		// it will try to send queued messages
		(**it).processOutgoingMsgs();

		// we read data and there may be some messages available (so we
		// should treat the messages appropriately)
		bool resultProcessing = (**it).processIncomingMsgs(mMsgHdlFactory);
		if (!resultProcessing) {
			LogDBG("Connection closed, invoking disconnection: %d", (*it)->getSocket());
			SrvLoginMgr::instance().removeConnection(*it);
			delete *it;
			it = mConnList.erase(it);
		}
	}
}

void SrvNetworkMgr::sendToConnection(MsgBase& msg, Netlink* netlink)
{
	int result = netlink->sendMsg(msg);
	if (!result) {
		LogERR("Message '%s' for connection %d (IP='%s') too big (%u)",
		       msg.getType().getName(),
		       netlink->getSocket(),
		       netlink->getIP(),
		       msg.getLength());
		return;
	}
}

void SrvNetworkMgr::sendToPlayer(MsgBase& msg, const LoginData* player)
{
	int result = player->getNetlink()->sendMsg(msg);
	if (!result) {
		LogERR("Message '%s' for player '%s' (IP='%s') too big (%u)",
		       msg.getType().getName(),
		       player->getPlayerName(),
		       player->getIP(),
		       msg.getLength());
		return;
	}
}

void SrvNetworkMgr::sendToAllConnections(MsgBase& msg)
{
	vector<LoginData*> allConnections;
	SrvLoginMgr::instance().getAllConnections(allConnections);

	for (size_t i = 0; i < allConnections.size(); ++i) {
		sendToPlayer(msg, allConnections[i]);
	}
}

void SrvNetworkMgr::sendToAllPlayers(MsgBase& msg)
{
	vector<LoginData*> allPlayers;
	SrvLoginMgr::instance().getAllConnectionsPlaying(allPlayers);

	for (size_t i = 0; i < allPlayers.size(); ++i) {
		sendToPlayer(msg, allPlayers[i]);
	}
}

void SrvNetworkMgr::sendToPlayerList(MsgBase& msg, vector<LoginData*>& playerList)
{
	for (size_t i = 0; i < playerList.size(); ++i) {
		sendToPlayer(msg, playerList[i]);
	}
}

void SrvNetworkMgr::sendToAllButPlayer(MsgBase& msg, const LoginData* player)
{
	vector<LoginData*> allPlayers;
	SrvLoginMgr::instance().getAllConnectionsPlaying(allPlayers);

	for (size_t i = 0; i < allPlayers.size(); ++i) {
		if (player != allPlayers[i]) {
			sendToPlayer(msg, allPlayers[i]);
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
