/*
 * botmsghdls.cpp
 * Copyright (C) 2006 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *						Bryan Duff <duff0097@umn.edu>
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
#include "bot/botconfig.h"

#include <stdio.h>

#include "common/net/netlayer.h"
#include "common/net/msgs.h"
#include "common/configmgr.h"

#include "bot/bot.h"
#include "bot/botinventory.h"
#include "bot/action/bottradeinv.h"
#include "bot/action/botcombat.h"
#include "bot/action/botmove.h"
#include "bot/npc/dialog.h"

#include "botnetmgr.h"
#include "botmsghdls.h"

//---------------------------------------------------------------
// Test
//---------------------------------------------------------------

//------------------ MsgHdlTestDataTypes --------------------------
MsgType MsgHdlTestDataTypes::getMsgType() const
{
	return MsgTestDataTypes::mType;
}

void MsgHdlTestDataTypes::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	// MsgTestDataTypes* msg = dynamic_cast<MsgTestDataTypes*>(&baseMsg);
	// don't do anything, the data gets printed automatically when
	// deserializing
}


//---------------------------------------------------------------
// Connections
//---------------------------------------------------------------

//------------------ MsgHdlConnectReply --------------------------
MsgType MsgHdlConnectReply::getMsgType() const
{
	return MsgConnectReply::mType;
}

void MsgHdlConnectReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgConnectReply* msg = dynamic_cast<MsgConnectReply*>(&baseMsg);

	if (msg->resultCode == MsgUtils::Errors::SUCCESS)
	{
		LogNTC("Uptime - %s\nPlayers - %u\nUsers - %u\n-Chars - %u\n", 
					msg->uptime.c_str(),
					msg->currentPlayers,
					msg->totalUsers,
					msg->totalChars);

		if (Bot->getAutoLogin() == true)
		{
			LogNTC("Auto logging in...");
			MsgLogin login;
			login.username = ConfigMgr::instance().getConfigVar("Bot.Settings.User", "");
            login.pw_md5sum = ConfigMgr::instance().getConfigVar("Bot.Settings.Password", "");
            Bot->getNetworkMgr()->sendToServer(login);
		}
	}
	else
	{
		LogERR("Reply msg error: %s", MsgUtils::Errors::getDescription(msg->resultCode));
	}
}


//------------------ MsgHdlLoginReply --------------------------
MsgType MsgHdlLoginReply::getMsgType() const
{
	return MsgLoginReply::mType;
}

void MsgHdlLoginReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgLoginReply* msg = dynamic_cast<MsgLoginReply*>(&baseMsg);

	if (msg->resultCode == MsgUtils::Errors::SUCCESS)
	{
		if (Bot->getAutoJoin() == true)
		{
			LogNTC("Auto joining in...");
			MsgJoin join;
			join.charname = ConfigMgr::instance().getConfigVar("Bot.Settings.Character", "-");
			if (join.charname != "-")
		    Bot->setName( join.charname.c_str() );
	            Bot->getNetworkMgr()->sendToServer(join);
			return;
		}

		int count = 0;
		vector<MsgLoginReply::CharacterListEntry>& charListEntries = msg->charList;
		while (!charListEntries.empty())
		{
			count++;
			MsgLoginReply::CharacterListEntry entry = charListEntries.back();
			charListEntries.pop_back();
			printf("%d ----\nName: %s\nRace: %s\nGender: %s\nArea: %s\n",
					count,
					entry.name.c_str(),
					entry.race.c_str(),
					entry.gender.c_str(),
					entry.area.c_str());
		}
	}
	else
	{
		LogERR("Failed to login: %s", MsgUtils::Errors::getDescription(msg->resultCode));
	}
}

//------------------ MsgHdlNewUserReply --------------------------
MsgType MsgHdlNewUserReply::getMsgType() const
{
	return MsgNewUserReply::mType;
}

void MsgHdlNewUserReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgNewUserReply* msg = dynamic_cast<MsgNewUserReply*>(&baseMsg);

	if (msg->resultCode == MsgUtils::Errors::SUCCESS)
	{
		LogNTC("New user created");
	}
	else
	{
		LogERR("Failed create new user: %s", MsgUtils::Errors::getDescription(msg->resultCode));
	}
}

//------------------ MsgHdlNewCharReply --------------------------
MsgType MsgHdlNewCharReply::getMsgType() const
{
	return MsgNewCharReply::mType;
}

void MsgHdlNewCharReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgNewCharReply* msg = dynamic_cast<MsgNewCharReply*>(&baseMsg);

	if (msg->resultCode == MsgUtils::Errors::SUCCESS)
	{
		printf("Name: %s\nRace: %s\nGender: %s\n Area: %s\n",
				msg->charname.c_str(),
				msg->race.c_str(),
				msg->gender.c_str(),
				msg->area.c_str());
	}
	else
	{
		LogERR("Failed to create character: %s", MsgUtils::Errors::getDescription(msg->resultCode));
	}
}

//------------------ MsgHdlDelCharReply --------------------------
MsgType MsgHdlDelCharReply::getMsgType() const
{
	return MsgDelCharReply::mType;
}

void MsgHdlDelCharReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgDelCharReply* msg = dynamic_cast<MsgDelCharReply*>(&baseMsg);

	if (msg->resultCode == MsgUtils::Errors::SUCCESS)
	{
		LogNTC("'%s' removed.\n", msg->charname.c_str());
	}
	else
	{
		LogERR("Failed to delete character: %s", MsgUtils::Errors::getDescription(msg->resultCode));
	}
}

//------------------ MsgHdlJoinReply --------------------------
MsgType MsgHdlJoinReply::getMsgType() const
{
	return MsgJoinReply::mType;
}

void MsgHdlJoinReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgJoinReply* msg = dynamic_cast<MsgJoinReply*>(&baseMsg);

	if (msg->resultCode == MsgUtils::Errors::SUCCESS) {
		LogNTC("You have successfully joined the world");
	} else {
		LogERR("Failed to join: %s", MsgUtils::Errors::getDescription(msg->resultCode));
	}
}


//---------------------------------------------------------------
// Content
//---------------------------------------------------------------

//---------------------------- MsgHdlContentFilePart --------------------------
MsgType MsgHdlContentFilePart::getMsgType() const
{
	return MsgContentFilePart::mType;
}

void MsgHdlContentFilePart::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	//MsgContentFilePart* msg = dynamic_cast<MsgContentFilePart*>(&baseMsg));
}


//----------------------- MsgHdlContentDeleteList ----------------------
MsgType MsgHdlContentDeleteList::getMsgType() const
{
	return MsgContentDeleteList::mType;
}

void MsgHdlContentDeleteList::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	//MsgContentDeleteList* msg = dynamic_cast<MsgContentDeleteList*>(&baseMsg);
}


//------------------------ MsgHdlContentUpdateList  ----------------------
MsgType MsgHdlContentUpdateList::getMsgType() const
{
	return MsgContentUpdateList::mType;
}

void MsgHdlContentUpdateList::handleMsg(MsgBase& /* baseMsg */, Netlink* /* netlink */)
{
	//MsgContentUpdateList* msg = dynamic_cast<MsgContentUpdateList*>(&baseMsg);
}


//---------------------------------------------------------------
// Console
//---------------------------------------------------------------

//------------------ MsgHdlChat --------------------------
MsgType MsgHdlChat::getMsgType() const
{
	return MsgChat::mType;
}

void MsgHdlChat::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgChat* msg = dynamic_cast<MsgChat*>(&baseMsg);

	string line = StrFmt("<%s> %s", msg->origin.c_str(), msg->text.c_str());
	string type;
	switch(msg->type)
	{
		case MsgChat::SYSTEM:
			type = "SYSTEM";
		break;
		case MsgChat::ACTION:
			type = "ACTION";
		break;
		case MsgChat::CHAT:
			type = "CHAT";
		break;
		case MsgChat::PM:
			type = "PM";
		break;
	}
	LogNTC("%s - %s\n", type.c_str(), line.c_str());

	if ( Bot->isMode( fmBot::NPC ) )
	{
		NPCDialogMgr::instance().handleMsg( msg );
	}
}

//---------------------------------------------------------------
// Dialog
//---------------------------------------------------------------

//------------------ MsgHdlNPCDialogReply --------------------------
MsgType MsgHdlNPCDialogReply::getMsgType() const
{
	return MsgNPCDialogReply::mType;
}

void MsgHdlNPCDialogReply::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgNPCDialogReply* msg = dynamic_cast<MsgNPCDialogReply*>(&baseMsg);

	LogNTC("Handling dialog reply message");

	if ( Bot->isMode( fmBot::NPC ) )
	{
		NPCDialogMgr::instance().handleMsg( msg );
	}
}

//---------------------------------------------------------------
// Contact list
//---------------------------------------------------------------

//------------------ MsgHdlContactStatus --------------------------
MsgType MsgHdlContactStatus::getMsgType() const
{
	return MsgContactStatus::mType;
}

void MsgHdlContactStatus::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgContactStatus* msg = dynamic_cast<MsgContactStatus*>(&baseMsg);

	LogDBG("Receiving contact list message: %s:%c:%c:%s:%s",
	       msg->charname.c_str(),
	       msg->type,
	       msg->status,
	       msg->lastLogin.c_str(),
	       msg->comment.c_str());
}


//---------------------------------------------------------------
// Entities
//---------------------------------------------------------------

//-------------------------- MsgHdlEntityCreate --------------------------
MsgType MsgHdlEntityCreate::getMsgType() const
{
	return MsgEntityCreate::mType;
}

void MsgHdlEntityCreate::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgEntityCreate* msg = dynamic_cast<MsgEntityCreate*>(&baseMsg);

  BotMoveMgr::instance().handleCreateMsg(msg);
}


//------------------------ MsgHdlEntityMove ---------------------------
MsgType MsgHdlEntityMove::getMsgType() const
{
	return MsgEntityMove::mType;
}

void MsgHdlEntityMove::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgEntityMove* msg = dynamic_cast<MsgEntityMove*>(&baseMsg);

  BotMoveMgr::instance().handleMoveMsg(msg);
}


//----------------------- MsgHdlEntityDestroy ------------------------------
MsgType MsgHdlEntityDestroy::getMsgType() const
{ 
	return MsgEntityDestroy::mType;
}

void MsgHdlEntityDestroy::handleMsg(MsgBase& /* baseMsg */, Netlink* /* netlink */)
{
	//MsgEntityDestroy* msg = dynamic_cast<MsgEntityDestroy*>(&baseMsg);
}


//---------------------------------------------------------------
// Inventory
//---------------------------------------------------------------

//------------------ MsgHdlInventoryListingMsg --------------------------
MsgType MsgHdlInventoryListing::getMsgType() const
{
	return MsgInventoryListing::mType;
}

void MsgHdlInventoryListing::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgInventoryListing* msg = dynamic_cast<MsgInventoryListing*>(&baseMsg);

	LogDBG("InventoryListing received");
	/** duffolonious: this should be replaced with some bot version
	* of an inventory. */
	for (size_t i = 0; i < msg->invListing.size(); i++)
	{
		BotInventory::instance().AddItem(&(msg->invListing[i]));
	}
}


//------------------ MsgHdlInventoryItemAdd --------------------------
MsgType MsgHdlInventoryAdd::getMsgType() const
{
	return MsgInventoryAdd::mType;
}

void MsgHdlInventoryAdd::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgInventoryAdd* msg = dynamic_cast<MsgInventoryAdd*>(&baseMsg);

	LogDBG("InventoryItemAdded received (itemID: %s)",
	       msg->item.getItemID());

	BotInventory::instance().AddItem(&(msg->item));
}


//------------------ MsgHdlInventoryDel --------------------------
MsgType MsgHdlInventoryDel::getMsgType() const
{
	return MsgInventoryDel::mType;
}

void MsgHdlInventoryDel::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgInventoryDel* msg = dynamic_cast<MsgInventoryDel*>(&baseMsg);

	LogDBG("InventoryDel received (itemID: %lu)",
	       msg->itemID);

	BotInventory::instance().RemoveItem(msg->itemID);
}


//---------------------------------------------------------------
// Player data
//---------------------------------------------------------------

//------------------ MsgHdlPlayerDataMsg --------------------------
MsgType MsgHdlPlayerData::getMsgType() const
{
	return MsgPlayerData::mType;
}

void MsgHdlPlayerData::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgPlayerData* msg = dynamic_cast<MsgPlayerData*>(&baseMsg);

	// player stats window
	LogNTC("Health: %d/%d\n", msg->health_cur, msg->health_max);
	LogNTC("Magic: %f\n", float(msg->magic_cur)/float(msg->magic_max));
	LogNTC("Stamina: %f\n", float(msg->stamina)/100.0f);
	LogNTC("Load: %d/%d\n", msg->load_cur, msg->load_max);
	LogNTC("Gold: %d\n", msg->gold);
}


//---------------------------------------------------------------
// Time
//---------------------------------------------------------------

//------------------ MsgHdlTimeMinuteMsg --------------------------
#define YEAR_DIVISOR	529920	//(4*4*23*24*60)
#define SEASON_DIVISOR	132480	//(4*23*24*60)
#define MOON_DIVISOR	33120	//(23*24*60)
#define DAY_DIVISOR	1440	//(24*60)
#define HOUR_DIVISOR	60	//(60)

MsgType MsgHdlTimeMinute::getMsgType() const
{
	return MsgTimeMinute::mType;
}

void MsgHdlTimeMinute::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
/*
	MsgTimeMinute* msg = dynamic_cast<MsgTimeMinute*>(&baseMsg);

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

	LogDBG("*-> %du %dy %ds %dm %02dd %02dh:%02dm",
		   numeric_uptime,
	       year,
	       season,
	       moon,
	       day,
	       hour,
	       minute);
*/
}


//---------------------------------------------------------------
// Trading
//---------------------------------------------------------------

//------------------ MsgTrade --------------------------
MsgType MsgHdlTrade::getMsgType() const
{
	return MsgTrade::mType;
}

void MsgHdlTrade::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgTrade* msg = dynamic_cast<MsgTrade*>(&baseMsg);
	/// hand trade message to trade inventory.

	//if (isMode( Bot::NPC ) )
	//	dialogMgr::instance().handleMsg( msg );
	//else
	BotTradeInv::instance().handleMsg( msg );
}


//---------------------------------------------------------------
// Combat
//---------------------------------------------------------------

//------------------ MsgCombat --------------------------
MsgType MsgHdlCombat::getMsgType() const
{
	return MsgCombat::mType;
}

void MsgHdlCombat::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgCombat* msg = dynamic_cast<MsgCombat*>(&baseMsg);
	/// hand trade message to trade inventory.
	BotCombatMgr::instance().handleMsg( msg );
}


//------------------ MsgCombatAction --------------------------
MsgType MsgHdlCombatAction::getMsgType() const
{
	return MsgCombatAction::mType;
}

void MsgHdlCombatAction::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgCombatAction* msg = dynamic_cast<MsgCombatAction*>(&baseMsg);
	/// hand trade message to trade inventory.
	BotCombatMgr::instance().handleActionMsg( msg );
}


//------------------ MsgCombatResult --------------------------
MsgType MsgHdlCombatResult::getMsgType() const
{
	return MsgCombatResult::mType;
}

void MsgHdlCombatResult::handleMsg(MsgBase& baseMsg, Netlink* /* netlink */)
{
	MsgCombatResult* msg = dynamic_cast<MsgCombatResult*>(&baseMsg);
	/// hand trade message to trade inventory.
	BotCombatMgr::instance().handleResultMsg( msg );
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
