/*
 * srvmsghdls.cpp
 * Copyright (C) 2006-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *			      Bryan Duff <duff0097@umn.edu>
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

#include "srvmsghdls.h"

#include "common/net/msgs.h"

#include "server/console/srvcommand.h"
#include "server/console/srvconsolemgr.h"
#include "server/content/srvcontentmgr.h"
#include "server/entity/srventityplayer.h"
#include "server/login/srvloginmgr.h"
#include "server/net/srvnetworkmgr.h"
#include "server/world/srvworldcontactmgr.h"
#include "server/world/srvworldmgr.h"
#include "server/action/srvtrademgr.h"
#include "server/action/srvcombatmgr.h"


//---------------------------------------------------------------
// Connections
//---------------------------------------------------------------

//------------------------ MsgHdlConnect ------------------------
MsgType MsgHdlConnect::getMsgType() const
{
	return MsgConnect::mType;
}

void MsgHdlConnect::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	// MsgConnect* msg = dynamic_cast<MsgConnect*>(&baseMsg);
	SrvLoginMgr::instance().addConnection(netlink);
}

//------------------------ MsgHdlLogin ------------------------
MsgType MsgHdlLogin::getMsgType() const
{
	return MsgLogin::mType;
}

void MsgHdlLogin::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgLogin* msg = dynamic_cast<MsgLogin*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);
	SrvLoginMgr::instance().login(player,
				     msg->username,
				     msg->pw_md5sum);
}

//------------------------ MsgHdlJoin ------------------------
MsgType MsgHdlJoin::getMsgType() const
{
	return MsgJoin::mType;
}

void MsgHdlJoin::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgJoin* msg = dynamic_cast<MsgJoin*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);
	SrvLoginMgr::instance().joinGame(player, msg->charname);
}

//------------------------ MsgHdlNewUser ------------------------
MsgType MsgHdlNewUser::getMsgType() const
{
	return MsgNewUser::mType;
}

void MsgHdlNewUser::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgNewUser* msg = dynamic_cast<MsgNewUser*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);
	SrvLoginMgr::instance().createUser(player,
					  msg->username,
					  msg->pw_md5sum,
					  msg->email,
					  msg->realname);

}

//------------------------ MsgHdlNewChar ------------------------
MsgType MsgHdlNewChar::getMsgType() const
{
	return MsgNewChar::mType;
}

void MsgHdlNewChar::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgNewChar* msg = dynamic_cast<MsgNewChar*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);
	SrvLoginMgr::instance().createCharacter(player,
					       msg->charname,
					       msg->race, msg->gender,
					       msg->playerClass,
					       msg->ab_choice_con,
					       msg->ab_choice_str,
					       msg->ab_choice_dex,
					       msg->ab_choice_int,
					       msg->ab_choice_wis,
					       msg->ab_choice_cha);
}

//------------------------ MsgHdlDelChar ------------------------
MsgType MsgHdlDelChar::getMsgType() const
{
	return MsgDelChar::mType;
}

void MsgHdlDelChar::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgDelChar* msg = dynamic_cast<MsgDelChar*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);
	SrvLoginMgr::instance().deleteCharacter(player,
					       msg->charname);
}


//---------------------------------------------------------------
// Content
//---------------------------------------------------------------

//------------------ MsgHdlContentQueryUpdate --------------------------
MsgType MsgHdlContentQueryUpdate::getMsgType() const
{
	return MsgContentQueryUpdate::mType;
}

void MsgHdlContentQueryUpdate::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgContentQueryUpdate* msg = dynamic_cast<MsgContentQueryUpdate*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);
	SrvContentMgr::instance().handleQueryFiles(player, msg); 
}


//---------------------------------------------------------------
// Console
//---------------------------------------------------------------

//------------------ MsgHdlChat --------------------------
MsgType MsgHdlChat::getMsgType() const
{
	return MsgChat::mType;
}

void MsgHdlChat::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgChat* msg = dynamic_cast<MsgChat*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);

	if (!player->isPlaying()) {
		LogERR("Chat message with invalid player: '%s: %s'",
		       player->getPlayerName(),
		       msg->text.c_str());
	} else if (msg->text.empty()) {
		LogWRN("Empty chat message from '%s', ignoring",
		       player->getPlayerName());
	} else {
		SrvConsoleMgr::instance().processChat(msg, player);
	}
}

//------------------ MsgHdlCommand --------------------------
MsgType MsgHdlCommand::getMsgType() const
{
	return MsgCommand::mType;
}

void MsgHdlCommand::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgCommand* msg = dynamic_cast<MsgCommand*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);

	if (!player->isPlaying()) {
		LogERR("Command message with invalid player: '%s: %s'",
		       player->getPlayerName(),
		       msg->command.c_str());
	} else if (msg->command.empty()) {
		LogWRN("Empty command in message from '%s', ignoring",
		       player->getPlayerName());
	} else {
		SrvConsoleMgr::instance().processCommand(msg, player);
	}
}


//---------------------------------------------------------------
// Dialog
//---------------------------------------------------------------

//------------------ MsgHdlNPCDialog --------------------------
MsgType MsgHdlNPCDialog::getMsgType() const
{
	return MsgNPCDialog::mType;
}

void MsgHdlNPCDialog::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgNPCDialog* msg = dynamic_cast<MsgNPCDialog*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);
	LoginData* target = SrvLoginMgr::instance().findPlayer(msg->target.c_str());

	msg->origin = player->getPlayerName();

	LogDBG("npc (%s) sending msg to '%s'", msg->origin.c_str(), msg->target.c_str() );

	SrvNetworkMgr::instance().sendToPlayer( *msg, target );
}


//------------------ MsgHdlNPCDialogReply ---------------------
MsgType MsgHdlNPCDialogReply::getMsgType() const
{
	return MsgNPCDialogReply::mType;
}

void MsgHdlNPCDialogReply::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgNPCDialogReply* msg = dynamic_cast<MsgNPCDialogReply*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);
	LoginData* target = SrvLoginMgr::instance().findPlayer(msg->target);

	msg->origin = player->getPlayerName();

	LogDBG("%s sending msg to '%llu'", msg->origin.c_str(), msg->target);

	SrvNetworkMgr::instance().sendToPlayer( *msg, target );
}


//---------------------------------------------------------------
// Contact list
//---------------------------------------------------------------

//------------------ MsgHdlContactAdd --------------------------
MsgType MsgHdlContactAdd::getMsgType() const
{
	return MsgContactAdd::mType;
}

void MsgHdlContactAdd::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgContactAdd* msg = dynamic_cast<MsgContactAdd*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);

	SrvWorldContactMgr::addContact(player, msg->charname,
				       msg->type, msg->comment);
}


//------------------ MsgHdlContactDel --------------------------
MsgType MsgHdlContactDel::getMsgType() const
{
	return MsgContactDel::mType;
}

void MsgHdlContactDel::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgContactDel* msg = dynamic_cast<MsgContactDel*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);

	SrvWorldContactMgr::removeContact(player, msg->charname);
}


//---------------------------------------------------------------
// Entities
//---------------------------------------------------------------

//------------------------- MsgHdlEntityMove ---------------------
MsgType MsgHdlEntityMove::getMsgType() const
{
	return MsgEntityMove::mType;
}

void MsgHdlEntityMove::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgEntityMove* msg = dynamic_cast<MsgEntityMove*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);

	if (!player || !player->isPlaying()) {
		LogWRN("Player '%s' (IP: %s) sent EntityMove message but not playing, ignoring",
		       player->getUserName(), player->getIP());
		return;
	} else {
		player->getPlayerEntity()->updateMovementFromClient(msg);
	}
}

//---------------------------------------------------------------
// Inventory
//---------------------------------------------------------------

//--------------------- MsgHdlInventoryGet ----------------------
MsgType MsgHdlInventoryGet::getMsgType() const
{
	return MsgInventoryGet::mType;
}

void MsgHdlInventoryGet::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgInventoryGet* msg = dynamic_cast<MsgInventoryGet*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);

	if (!player->isPlaying()) {
		LogWRN("User '%s' (IP: %s) sent EntityMove message but not playing, ignoring",
		       player->getUserName(), player->getIP());
		return;
	} else {
		SrvWorldMgr::instance().playerGetItem(player->getPlayerEntity(),
						      msg->itemID);
	}
}

//--------------------- MsgHdlInventoryDrop ----------------------
MsgType MsgHdlInventoryDrop::getMsgType() const
{
	return MsgInventoryDrop::mType;
}

void MsgHdlInventoryDrop::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgInventoryDrop* msg = dynamic_cast<MsgInventoryDrop*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);

	if (!player->isPlaying()) {
		LogWRN("User '%s' (IP: %s) sent EntityMove message but not playing, ignoring",
		       player->getUserName(), player->getIP());
		return;
	} else {
		SrvWorldMgr::instance().playerDropItem(player->getPlayerEntity(),
						       msg->itemID);
	}
}


//---------------------------------------------------------------
// Trade
//---------------------------------------------------------------

//--------------------- MsgHdlTrade ---------------------------
MsgType MsgHdlTrade::getMsgType() const
{
	return MsgTrade::mType;
}

void MsgHdlTrade::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgTrade* msg = dynamic_cast<MsgTrade*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);

	msg->player = player->getPlayerName();

	if (!SrvTradeMgr::instance().handleTrade(msg))
		LogDBG("Error - Trade not handled");
}


//---------------------------------------------------------------
// Combat
//---------------------------------------------------------------

//--------------------- MsgHdlCombat ---------------------------
MsgType MsgHdlCombat::getMsgType() const
{
	return MsgCombat::mType;
}

void MsgHdlCombat::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgCombat* msg = dynamic_cast<MsgCombat*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);

	LogDBG("Combat manager handling message...");

	msg->player = StrToUInt64( player->getPlayerID() );

	if (!SrvCombatMgr::instance().handleMsg(msg))
		LogERR("Combat msg not handled");
}


//--------------------- MsgHdlCombatAction ---------------------------
MsgType MsgHdlCombatAction::getMsgType() const
{
	return MsgCombatAction::mType;
}

void MsgHdlCombatAction::handleMsg(MsgBase& baseMsg, Netlink* netlink)
{
	MsgCombatAction* msg = dynamic_cast<MsgCombatAction*>(&baseMsg);
	LoginData* player = SrvLoginMgr::instance().findPlayer(netlink);

	LogDBG("Combat manager handling message...");

	msg->player = StrToUInt64( player->getPlayerID() );

	/// Handling this message is pretty simple - alternative that's cleaner?
	if (!SrvCombatMgr::instance().handleActionMsg(msg))
		LogERR("Combat msg not handled");
}




// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
