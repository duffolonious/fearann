/*
 * cltmsghdls.cpp
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
#include "client/cltconfig.h"

#include <osg/Matrix>

#include "common/net/netlayer.h"
#include "common/net/msgs.h"
#include "common/configmgr.h"

#include "client/cltentitymgr.h"
#include "client/cltviewer.h"
#include "client/action/cltcombatmgr.h"
#include "client/cegui/cltceguimgr.h"
#include "client/cegui/cltceguiconsole.h"
#include "client/cegui/cltceguidialog.h"
#include "client/cegui/cltceguiinitial.h"
#include "client/cegui/cltceguiinventory.h"
#include "client/content/cltcontentmgr.h"
#include "client/entity/cltentitymainplayer.h"

#include "cltmsghdls.h"


//---------------------------------------------------------------
// Test
//---------------------------------------------------------------

//------------------ CltMsgHdlTestDataTypes --------------------------
MsgType CltMsgHdlTestDataTypes::getMsgType() const
{
	return MsgTestDataTypes::mType;
}

void CltMsgHdlTestDataTypes::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	// MsgTestDataTypes* msg = dynamic_cast<MsgTestDataTypes*>(&baseMsg);

	// don't do anything, the data gets printed automatically when
	// deserializing
}


//---------------------------------------------------------------
// Connections
//---------------------------------------------------------------

//------------------ CltMsgHdlConnectReply --------------------------
MsgType CltMsgHdlConnectReply::getMsgType() const
{
	return MsgConnectReply::mType;
}

void CltMsgHdlConnectReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgConnectReply* msg = dynamic_cast<MsgConnectReply*>(&baseMsg);
	return;

	string clientVersion = ConfigMgr::instance().getConfigVar("Client.ProtocolVersion", "");
	if (msg->resultCode == MsgUtils::Errors::SUCCESS
	    && msg->protocolVersion != clientVersion) {
		string errmsg = StrFmt("Protocol versions of client (%s) and server (%s) differ",
				       clientVersion.c_str(), msg->protocolVersion.c_str());
		CltCEGUIMgr::instance().Notification_DisplayMessage(errmsg.c_str());
		LogERR(errmsg.c_str());
	} else if (msg->resultCode == MsgUtils::Errors::SUCCESS) {
		CltCEGUIInitial::instance().Connect_to_Login(msg->uptime.c_str(),
							     StrFmt("%u", msg->currentPlayers),
							     StrFmt("%u", msg->totalUsers),
							     StrFmt("%u", msg->totalChars));
	} else {
		string errmsg = StrFmt("Failed to connect: %s",
				       MsgUtils::Errors::getDescription(msg->resultCode));
		CltCEGUIMgr::instance().Notification_DisplayMessage(errmsg.c_str());
		LogERR(errmsg.c_str());
	}
}


//------------------ CltMsgHdlLoginReply --------------------------
MsgType CltMsgHdlLoginReply::getMsgType() const
{
	return MsgLoginReply::mType;
}

void CltMsgHdlLoginReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgLoginReply* msg = dynamic_cast<MsgLoginReply*>(&baseMsg);
	return;

	if (msg->resultCode == MsgUtils::Errors::SUCCESS) {
		CltCEGUIInitial::instance().Login_to_Join(msg->charList);
	} else {
		string errmsg = StrFmt("Failed login: %s",
				       MsgUtils::Errors::getDescription(msg->resultCode));
		CltCEGUIMgr::instance().Notification_DisplayMessage(errmsg.c_str());
		CltCEGUIInitial::instance().Login_Failed();
		LogERR(errmsg.c_str());
	}
}

//------------------ CltMsgHdlNewUserReply --------------------------
MsgType CltMsgHdlNewUserReply::getMsgType() const
{
	return MsgNewUserReply::mType;
}

void CltMsgHdlNewUserReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgNewUserReply* msg = dynamic_cast<MsgNewUserReply*>(&baseMsg);
	return;

	if (msg->resultCode == MsgUtils::Errors::SUCCESS) {
		CltCEGUIInitial::instance().NewUser_to_Login();
	} else {
		string errmsg = StrFmt("Failed to create new user: %s",
				       MsgUtils::Errors::getDescription(msg->resultCode));
		CltCEGUIMgr::instance().Notification_DisplayMessage(errmsg.c_str());
		LogERR(errmsg.c_str());
	}
}

//------------------ CltMsgHdlNewCharReply --------------------------
MsgType CltMsgHdlNewCharReply::getMsgType() const
{
	return MsgNewCharReply::mType;
}

void CltMsgHdlNewCharReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgNewCharReply* msg = dynamic_cast<MsgNewCharReply*>(&baseMsg);
	return;

	if (msg->resultCode == MsgUtils::Errors::SUCCESS) {
		CltCEGUIInitial::instance().Join_FillNewChar(msg->charname,
							     msg->race,
							     msg->gender,
							     msg->playerClass,
							     msg->area);
		CltCEGUIInitial::instance().NewChar_to_Join();
	} else {
		string errmsg = StrFmt("Failed to create new char: %s",
				       MsgUtils::Errors::getDescription(msg->resultCode));
		CltCEGUIMgr::instance().Notification_DisplayMessage(errmsg.c_str());
		LogERR(errmsg.c_str());
	}
}

//------------------ CltMsgHdlDelCharReply --------------------------
MsgType CltMsgHdlDelCharReply::getMsgType() const
{
	return MsgDelCharReply::mType;
}

void CltMsgHdlDelCharReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgDelCharReply* msg = dynamic_cast<MsgDelCharReply*>(&baseMsg);

	if (msg->resultCode == MsgUtils::Errors::SUCCESS) {
		CltCEGUIInitial::instance().Join_RemoveDelChar(msg->charname);
	} else {
		string errmsg = StrFmt("Failed to delete character: %s",
				       MsgUtils::Errors::getDescription(msg->resultCode));
		CltCEGUIMgr::instance().Notification_DisplayMessage(errmsg.c_str());
		LogERR(errmsg.c_str());
	}
}

//------------------ CltMsgHdlJoinReply --------------------------
MsgType CltMsgHdlJoinReply::getMsgType() const
{
	return MsgJoinReply::mType;
}

void CltMsgHdlJoinReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgJoinReply* msg = dynamic_cast<MsgJoinReply*>(&baseMsg);
	return;

	if (msg->resultCode == MsgUtils::Errors::SUCCESS) {
		CltCEGUIInitial::instance().LoadingGame_to_Game();
		CltCEGUIMgr::instance().StartPlaying();

		MsgChat msgReply;
		msgReply.origin = "Client";
		msgReply.type = MsgChat::SYSTEM;
		msgReply.text = "You are now playing in the world";
		CltCEGUIConsole::instance().printMessage(msgReply);
	} else {
		string errmsg = StrFmt("Failed to join: %s",
				       MsgUtils::Errors::getDescription(msg->resultCode));
		CltCEGUIMgr::instance().Notification_DisplayMessage(errmsg.c_str());
		LogERR(errmsg.c_str());
	}
}


//---------------------------------------------------------------
// Content
//---------------------------------------------------------------

//---------------------------- CltMsgHdlContentFilePart --------------------------
MsgType CltMsgHdlContentFilePart::getMsgType() const
{
	return MsgContentFilePart::mType;
}

void CltMsgHdlContentFilePart::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	CltContentMgr::instance().addFilePart(dynamic_cast<MsgContentFilePart*>(&baseMsg));
}


//----------------------- CltMsgHdlContentDeleteList ----------------------
MsgType CltMsgHdlContentDeleteList::getMsgType() const
{
	return MsgContentDeleteList::mType;
}

void CltMsgHdlContentDeleteList::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgContentDeleteList* msg = dynamic_cast<MsgContentDeleteList*>(&baseMsg);
	CltContentMgr::instance().deleteOutdatedFiles(msg->deleteList);
}


//------------------------ CltMsgHdlContentUpdateList  ----------------------
MsgType CltMsgHdlContentUpdateList::getMsgType() const
{
	return MsgContentUpdateList::mType;
}

void CltMsgHdlContentUpdateList::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	CltContentMgr::instance().addUpdatedFiles(dynamic_cast<MsgContentUpdateList*>(&baseMsg));
}


//---------------------------------------------------------------
// Console
//---------------------------------------------------------------

//------------------ CltMsgHdlChat --------------------------
MsgType CltMsgHdlChat::getMsgType() const
{
	return MsgChat::mType;
}

void CltMsgHdlChat::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgChat& msg = dynamic_cast<MsgChat&>(baseMsg);
	return;

	CltCEGUIConsole::instance().printMessage(msg);
}

//---------------------------------------------------------------
// Dialog
//---------------------------------------------------------------

//------------------ CltMsgHdlNPCDialog --------------------------
MsgType CltMsgHdlNPCDialog::getMsgType() const
{
	return MsgNPCDialog::mType;
}

void CltMsgHdlNPCDialog::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgNPCDialog& msg = dynamic_cast<MsgNPCDialog&>(baseMsg);
	LogDBG("handling NPC dialog message.");
	CltCEGUIDialog::instance().updateDialog(&msg);
}

//---------------------------------------------------------------
// Contact list
//---------------------------------------------------------------

//------------------ CltMsgHdlContactStatus --------------------------
MsgType CltMsgHdlContactStatus::getMsgType() const
{
	return MsgContactStatus::mType;
}

void CltMsgHdlContactStatus::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgContactStatus* msg = dynamic_cast<MsgContactStatus*>(&baseMsg);
	return;

	CltCEGUIMgr::instance().Contacts_AddToList(msg->charname.c_str(),
						   msg->type,
						   msg->status,
						   msg->lastLogin.c_str(),
						   msg->comment.c_str());
}


//---------------------------------------------------------------
// Entities
//---------------------------------------------------------------

//-------------------------- CltMsgHdlEntityCreate --------------------------
MsgType CltMsgHdlEntityCreate::getMsgType() const
{
	return MsgEntityCreate::mType;
}

void CltMsgHdlEntityCreate::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgEntityCreate* msg = dynamic_cast<MsgEntityCreate*>(&baseMsg);
	CltEntityMgr::instance().entityCreate(msg);
}


//------------------------ CltMsgHdlEntityMove ---------------------------
MsgType CltMsgHdlEntityMove::getMsgType() const
{
	return MsgEntityMove::mType;
}

void CltMsgHdlEntityMove::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgEntityMove* msg = dynamic_cast<MsgEntityMove*>(&baseMsg);
	CltEntityMgr::instance().entityMove(msg);
}


//----------------------- CltMsgHdlEntityDestroy ------------------------------
MsgType CltMsgHdlEntityDestroy::getMsgType() const
{ 
	return MsgEntityDestroy::mType;
}

void CltMsgHdlEntityDestroy::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgEntityDestroy* msg = dynamic_cast<MsgEntityDestroy*>(&baseMsg);
	CltEntityMgr::instance().entityDestroy(msg);
}


//---------------------------------------------------------------
// Inventory
//---------------------------------------------------------------

//------------------ CltMsgHdlInventoryListingMsg --------------------------
MsgType CltMsgHdlInventoryListing::getMsgType() const
{
	return MsgInventoryListing::mType;
}

void CltMsgHdlInventoryListing::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgInventoryListing* msg = dynamic_cast<MsgInventoryListing*>(&baseMsg);

	LogDBG("InventoryListing received");

	for (size_t i = 0; i < msg->invListing.size(); ++i) {
		CltEntityMainPlayer::instance().addToInventory(&(msg->invListing[i]));
	}
}


//------------------ CltMsgHdlInventoryItemAdd --------------------------
MsgType CltMsgHdlInventoryAdd::getMsgType() const
{
	return MsgInventoryAdd::mType;
}

void CltMsgHdlInventoryAdd::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgInventoryAdd* msg = dynamic_cast<MsgInventoryAdd*>(&baseMsg);

	LogDBG("InventoryItemAdded received (itemID: %s)", msg->item.getItemID());

	CltEntityMainPlayer::instance().addToInventory(&(msg->item));
}


//------------------ CltMsgHdlInventoryDel --------------------------
MsgType CltMsgHdlInventoryDel::getMsgType() const
{
	return MsgInventoryDel::mType;
}

void CltMsgHdlInventoryDel::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgInventoryDel* msg = dynamic_cast<MsgInventoryDel*>(&baseMsg);

	LogDBG("InventoryDel received (itemID: %llu)", msg->itemID);

	CltEntityMainPlayer::instance().removeFromInventory(msg->itemID);
}


//---------------------------------------------------------------
// Player data
//---------------------------------------------------------------

//------------------ CltMsgHdlPlayerDataMsg --------------------------
MsgType CltMsgHdlPlayerData::getMsgType() const
{
	return MsgPlayerData::mType;
}

void CltMsgHdlPlayerData::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgPlayerData* msg = dynamic_cast<MsgPlayerData*>(&baseMsg);
	return;

	// player itself
	CltEntityMainPlayer::instance().setPlayerData(msg);
	// PlayerStats window
	CltCEGUIMgr::instance().PlayerStats_Update(msg);
	// Inventory window
	CltCEGUIInventory::instance().updateStats(msg);
}


//---------------------------------------------------------------
// Time
//---------------------------------------------------------------

//------------------ CltMsgHdlTimeMinuteMsg --------------------------
MsgType CltMsgHdlTimeMinute::getMsgType() const
{
	return MsgTimeMinute::mType;
}

void CltMsgHdlTimeMinute::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgTimeMinute* msg = dynamic_cast<MsgTimeMinute*>(&baseMsg);

	// static values corresponding with in-game calendar
	const int YEAR_DIVISOR = 4*4*23*24*60;
	const int SEASON_DIVISOR = 4*23*24*60;
	const int MOON_DIVISOR = 23*24*60;
	const int DAY_DIVISOR = 24*60;
	const int HOUR_DIVISOR = 60;

	// converting time
	int numeric_uptime = msg->gametime;

	int year = numeric_uptime / YEAR_DIVISOR;
	int year_rest = numeric_uptime % YEAR_DIVISOR;
	int season = year_rest / SEASON_DIVISOR;
	int season_rest = year_rest % SEASON_DIVISOR;
	int moon = season_rest / MOON_DIVISOR;
	int moon_rest = season_rest % MOON_DIVISOR;
	int day = moon_rest / DAY_DIVISOR;
	int day_rest = moon_rest % DAY_DIVISOR;
	int hour = day_rest / HOUR_DIVISOR;
	int hour_rest = day_rest % HOUR_DIVISOR;
	int minute = hour_rest;

	/* mafm: uncomment this if you want to debug the time values
	LogDBG("*-> %du %dy %ds %dm %02dd %02dh:%02dm",
	       numeric_uptime, year, season, moon, day, hour, minute);
	*/

	// update environment in the client viewer
	CltViewer::instance().setEnvironment(day_rest);
	return;

	// setting time in the calendar applet
	string prettyTime = StrFmt("%02dh%02d", hour, minute);
	char prettySeason = '!';
	switch (season) {
	case 0: prettySeason = 'D'; break;
	case 1: prettySeason = 'E'; break;
	case 2: prettySeason = 'S'; break;
	case 3: prettySeason = 'R'; break;
	}
	string prettyMoon = StrFmt("%dm", moon+1);
	string prettyDate = StrFmt("%c %s %d", prettySeason, prettyMoon.c_str(), day);
	string prettyYear = StrFmt("%d", year);
	CltCEGUIMgr::instance().Calendar_SetTime(prettyTime, prettyDate, prettyYear);

	// changing the moon in the date applet, if needed
	int moonPic = 15;
	switch (day)
	{
	case 0: case 1: moonPic = 1; break;
	case 2: case 3: moonPic = 2; break;
	case 4: moonPic = 3; break;
	case 5: case 6: moonPic = 4; break;
	case 7: case 8: case 9: moonPic = 5; break;
	case 10: moonPic = 6; break;
	case 11: case 12: case 13: moonPic = 7; break;
	case 14: case 15: moonPic = 8; break;
	case 16: moonPic = 9; break;
	case 17: case 18: moonPic = 10; break;
	case 19: case 20: case 21: moonPic = 11; break;
	case 22: moonPic = 0; break;
	}
	string moonPictureName = StrFmt("Moon_%02d", moonPic);
	CltCEGUIMgr::instance().Calendar_SetMoonPicture(moonPictureName);
}


//---------------------------------------------------------------
// Trade
//---------------------------------------------------------------

//------------------ CltMsgHdlTimeMinuteMsg --------------------------
MsgType CltMsgHdlTrade::getMsgType() const
{
	return MsgTrade::mType;
}

void CltMsgHdlTrade::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	// MsgTrade* msg = dynamic_cast<MsgTrade*>(&baseMsg);
	LogDBG("Received trade message... no action taken");
}


//---------------------------------------------------------------
// Combat
//---------------------------------------------------------------

//------------------ CltMsgHdlCombat ----------------------------
MsgType CltMsgHdlCombat::getMsgType() const
{
	return MsgCombat::mType;
}

void CltMsgHdlCombat::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgCombat* msg = dynamic_cast<MsgCombat*>(&baseMsg);
	LogDBG("Received combat message... handling");
	CltCombatMgr::instance().handleMsg(msg);
}


//------------------ CltMsgHdlCombatResult -----------------------
MsgType CltMsgHdlCombatResult::getMsgType() const
{
	return MsgCombatResult::mType;
}

void CltMsgHdlCombatResult::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgCombatResult* msg = dynamic_cast<MsgCombatResult*>(&baseMsg);
	LogDBG("Received combat result message... handling");
	CltCombatMgr::instance().handleResultMsg(msg);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
