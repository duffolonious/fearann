/*
 * botnetmgr.cpp
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

#include "config.h"

#include "common/net/msgs.h"

#include "bot/bot.h"

#include "bot/net/botmsghdls.h"

#include "botnetmgr.h"


BotNetworkMgr::BotNetworkMgr() :
	mSocketLayer(&mNetlink)
{
	registerMsgHdls();
}

BotNetworkMgr::~BotNetworkMgr()
{
	mSocketLayer.disconnect();
}

void BotNetworkMgr::registerMsgHdls()
{
#define	REGHDL(msg, hdl) mMsgHdlFactory.registerMsgWithHdl(new msg, new hdl);

	// test messages
	REGHDL(MsgTestDataTypes, MsgHdlTestDataTypes);

	// connection messages
	REGHDL(MsgConnectReply, MsgHdlConnectReply);
	REGHDL(MsgLoginReply, MsgHdlLoginReply);
	REGHDL(MsgNewUserReply, MsgHdlNewUserReply);
	REGHDL(MsgNewCharReply, MsgHdlNewCharReply);
	REGHDL(MsgDelCharReply, MsgHdlDelCharReply);
	REGHDL(MsgJoinReply, MsgHdlJoinReply);

	// chat
	REGHDL(MsgChat, MsgHdlChat);
	// dialog
	REGHDL(MsgNPCDialogReply, MsgHdlNPCDialogReply);

	// contact list
	REGHDL(MsgContactStatus, MsgHdlContactStatus);

	// content
	REGHDL(MsgContentUpdateList, MsgHdlContentUpdateList);
	REGHDL(MsgContentDeleteList, MsgHdlContentDeleteList);
	REGHDL(MsgContentFilePart, MsgHdlContentFilePart);

	// entities
	REGHDL(MsgEntityCreate, MsgHdlEntityCreate);
	REGHDL(MsgEntityMove, MsgHdlEntityMove);
	REGHDL(MsgEntityDestroy, MsgHdlEntityDestroy);

	// inventory
	REGHDL(MsgInventoryListing, MsgHdlInventoryListing);
	REGHDL(MsgInventoryAdd, MsgHdlInventoryAdd);
	REGHDL(MsgInventoryDel, MsgHdlInventoryDel);

	// player data
	REGHDL(MsgPlayerData, MsgHdlPlayerData);

	// time messages
	REGHDL(MsgTimeMinute, MsgHdlTimeMinute);

	// trade messages
	REGHDL(MsgTrade, MsgHdlTrade);

	// combat messages
	REGHDL(MsgCombat, MsgHdlCombat);
	REGHDL(MsgCombatResult, MsgHdlCombatResult);
}

bool BotNetworkMgr::isConnected()
{
	return mSocketLayer.isConnected();
}

bool BotNetworkMgr::connectToServer(const char* host, int port)
{
	if (mSocketLayer.isConnected())
	{
		LogWRN("Already connected, refusing to connect again");
		return false;
	}
	else
		return mSocketLayer.connectToServer(host, port);
}

void BotNetworkMgr::disconnect()
{
	mSocketLayer.disconnect();
}

void BotNetworkMgr::sendToServer(MsgBase& msg)
{
	if (!mSocketLayer.isConnected())
	{
		LogERR("No connection, a message was dropped");
		return;
	}

	bool result = mNetlink.sendMsg(msg);
	if (!result)
	{
		LogERR("Message could not be sent to server");
		return;
	}
}

void BotNetworkMgr::processIncomingMsgs()
{
	if (!mSocketLayer.isConnected())
	{
		// we just return without printing anything because
		// otherwise it floods the client log until we get
		// connected for the first time
		return;
	}

	// it will try to send queued messages
	mNetlink.processOutgoingMsgs();

	// this processes the messages alone internally, we just need
	// to worry if the server disconnects
	bool result = mNetlink.processIncomingMsgs(mMsgHdlFactory);
	if (!result)
	{
		LogNTC("Connection to server lost");
		disconnect();
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
