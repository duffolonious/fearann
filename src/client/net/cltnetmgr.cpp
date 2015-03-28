/*
 * cltnetmgr.cpp
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

#include "client/content/cltcontentmgr.h"
#include "client/cegui/cltceguiconsole.h"

#include "cltmsghdls.h"

#include "cltnetmgr.h"


//----------------------- CltNetworkMgr ----------------------------
template <> CltNetworkMgr* Singleton<CltNetworkMgr>::INSTANCE = 0;

CltNetworkMgr::CltNetworkMgr() :
	mSocketLayer(&mNetlink)
{
	registerMsgHdls();
}

CltNetworkMgr::~CltNetworkMgr()
{
	mSocketLayer.disconnect();
}

void CltNetworkMgr::registerMsgHdls()
{
#define	REGHDL(msg, hdl) mMsgHdlFactory.registerMsgWithHdl(new msg, new hdl);

	// test messages
	REGHDL(MsgTestDataTypes, CltMsgHdlTestDataTypes);

	// connection messages
	REGHDL(MsgConnectReply, CltMsgHdlConnectReply);
	REGHDL(MsgLoginReply, CltMsgHdlLoginReply);
	REGHDL(MsgNewUserReply, CltMsgHdlNewUserReply);
	REGHDL(MsgNewCharReply, CltMsgHdlNewCharReply);
	REGHDL(MsgDelCharReply, CltMsgHdlDelCharReply);
	REGHDL(MsgJoinReply, CltMsgHdlJoinReply);

	// chat
	REGHDL(MsgChat, CltMsgHdlChat);

	// dialog
	REGHDL(MsgNPCDialog, CltMsgHdlNPCDialog);

	// contact list
	REGHDL(MsgContactStatus, CltMsgHdlContactStatus);

	// content
	REGHDL(MsgContentUpdateList, CltMsgHdlContentUpdateList);
	REGHDL(MsgContentDeleteList, CltMsgHdlContentDeleteList);
	REGHDL(MsgContentFilePart, CltMsgHdlContentFilePart);

	// entities
	REGHDL(MsgEntityCreate, CltMsgHdlEntityCreate);
	REGHDL(MsgEntityMove, CltMsgHdlEntityMove);
	REGHDL(MsgEntityDestroy, CltMsgHdlEntityDestroy);

	// inventory
	REGHDL(MsgInventoryListing, CltMsgHdlInventoryListing);
	REGHDL(MsgInventoryAdd, CltMsgHdlInventoryAdd);
	REGHDL(MsgInventoryDel, CltMsgHdlInventoryDel);

	// player data
	REGHDL(MsgPlayerData, CltMsgHdlPlayerData);

	// time messages
	REGHDL(MsgTimeMinute, CltMsgHdlTimeMinute);

	// combat
	REGHDL(MsgCombat, CltMsgHdlCombat);
	REGHDL(MsgCombatResult, CltMsgHdlCombatResult);
}

bool CltNetworkMgr::isConnected() const
{
	return mSocketLayer.isConnected();
}

bool CltNetworkMgr::connectToServer(const char* host, int port)
{
	if (mSocketLayer.isConnected()) {
		LogWRN("Already connected, refusing to connect again");
		return false;
	} else {
		return mSocketLayer.connectToServer(host, port);
	}
}

void CltNetworkMgr::disconnect()
{
	mSocketLayer.disconnect();
}

void CltNetworkMgr::sendToServer(MsgBase& msg)
{
	MsgChat consoleMsg;
	consoleMsg.origin = "Client";
	consoleMsg.type = MsgChat::SYSTEM;

	if (!mSocketLayer.isConnected()) {
		consoleMsg.text = "No connection, a message was dropped";
		LogERR("%s", consoleMsg.text.c_str());

		/// \todo mafm: CEGUI not working, disable to avoid segfaults
		// CltCEGUIConsole::instance().printMessage(consoleMsg);

		return;
	}

	bool result = mNetlink.sendMsg(msg);
	if (!result) {
		consoleMsg.text = "Message could not be sent to server";
		LogERR("%s", consoleMsg.text.c_str());

		/// \todo mafm: CEGUI not working, disable to avoid segfaults
		// CltCEGUIConsole::instance().printMessage(consoleMsg);

		return;
	}
}

void CltNetworkMgr::processIncomingMsgs()
{
	if (!mSocketLayer.isConnected()) {
		// we just return without printing anything because otherwise it
		// floods the client log until we get connected for the first
		// time
		return;
	}

	// it will try to send queued messages
	mNetlink.processOutgoingMsgs();

	// this processes the messages alone internally, we just need to worry
	// if the server disconnects
	bool result = mNetlink.processIncomingMsgs(mMsgHdlFactory);
	if (!result) {
		MsgChat consoleMsg;
		consoleMsg.origin = "Client";
		consoleMsg.type = MsgChat::SYSTEM;
		consoleMsg.text = "Connection to server lost";
		LogNTC("%s", consoleMsg.text.c_str());
		CltCEGUIConsole::instance().printMessage(consoleMsg);

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
