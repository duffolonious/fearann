/*
 * srvworldcontactmgr.cpp
 * Copyright (C) 2005-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#include "srvworldcontactmgr.h"

#include "common/net/msgs.h"

#include "server/db/srvdbmgr.h"
#include "server/login/srvloginmgr.h"
#include "server/net/srvnetworkmgr.h"


/*******************************************************************************
 * SrvWorldContactMgr
 ******************************************************************************/
void SrvWorldContactMgr::playerStatusChange(const LoginData* player,
					    bool connected)
{
	// STEPS
	// 1- get the list of contacts of the new player, see if they're
	// connected and send a message with the status of each
	// 2- tell other people the new status of this contact

	if (connected) {
		notifyPlayerOfContacts(player);
	}

	notifyOtherPlayers(player, connected);
}

void SrvWorldContactMgr::addContact(const LoginData* player,
				    std::string& otherCharname,
				    char otherTypeChar,
				    std::string& otherComment)
{
	LogNTC("Player '%s' requested to add this contact: '%s',"
	       " type %c, comment '%s'",
	       player->getPlayerName(), otherCharname.c_str(),
	       otherTypeChar, otherComment.c_str());

	// STEPS
	// 1- check if player already has this contact
	// 2- get the data of the contact player (cid, last login)
	// 3- insert into the database
	// 4- notify about the status of the recently added contact

	string charname = player->getPlayerName();
	string cid = player->getPlayerID();
	otherCharname = SrvDBMgr::instance().escape(otherCharname);
	otherComment = SrvDBMgr::instance().escape(otherComment);
	string otherType = StrFmt("%c", otherTypeChar);
  
	// 1- check if player already has this contact
	{
		SrvDBQuery query;
		query.setTables("contact_list");
		string condition = "cid='" + cid +
			+ "' AND contact_charname='" + otherCharname + "'";
		query.setCondition(condition);
		bool matches = SrvDBMgr::instance().queryMatch(&query);
		if (matches) {
			LogERR("Player already has this contact (player '%s', "
			       "contact player '%s', type '%s', comment '%s')",
			       player->getPlayerName(), otherCharname.c_str(),
			       otherType.c_str(), otherComment.c_str());
			sendConsoleReply(player, "Add contact failed: "
					 "You already have this player in your contact list");
			return;
		}
	}

	// 2- get the data of the contact player (cid, last login)
	string otherCid;
	string otherLastLogin;
	{
		SrvDBQuery query;
		query.setTables("usr_chars");
		string condition = "charname='" + otherCharname + "'";
		query.setCondition(condition);
		query.addColumnWithoutValue("cid");
		query.addColumnWithoutValue("last_login");
		int numresults = SrvDBMgr::instance().querySelect(&query);
		if (numresults != 1) {
			LogERR("Add contact failed: no such character (%s)", 
			       otherCharname.c_str());
			sendConsoleReply(player, "Add contact failed "
					 "(bad contact character name).");
			return;
		}

		query.getResult()->getValue(0, "cid", otherCid);
		query.getResult()->getValue(0, "last_login", otherLastLogin);
	}

	// 3- insert into the database
	{
		SrvDBQuery query;
		query.setTables("contact_list");
		query.addColumnWithValue("cid", cid);
		query.addColumnWithValue("charname", charname);
		query.addColumnWithValue("contact_cid", otherCid);
		query.addColumnWithValue("contact_charname", otherCharname);
		query.addColumnWithValue("type", otherType);
		query.addColumnWithValue("comments", otherComment);
		//query.addColumnWithValue("creation_date", "CURRENT_TIMESTAMP", false);
		bool success = SrvDBMgr::instance().queryInsert(&query);
		if (success) {
			LogNTC("Success adding contact (player '%s', "
			       "contact player '%s', type '%s', comment '%s')",
			       player->getPlayerName(), otherCharname.c_str(),
			       otherType.c_str(), otherComment.c_str());

			sendConsoleReply(player, "Info: Successfully added contact");
		} else {
			LogERR("Add contact failed, unknown DB error (player '%s', "
			       "contact player '%s', type '%s', comment '%s')",
			       player->getPlayerName(), otherCharname.c_str(),
			       otherType.c_str(), otherComment.c_str());
			sendConsoleReply(player, "Add contact failed: "
					 "There was a DB error trying to add it");
			return;
		}
	}

	// 4- notify about the status of the recently added contact
	notifyPlayerOfOneContact(player,
				 otherCharname, otherTypeChar,
				 otherComment, otherLastLogin,
				 isContactPlaying(otherCharname));
}

void SrvWorldContactMgr::removeContact(const LoginData* player,
				       std::string& otherCharname)
{
	LogDBG("Player '%s' requested to remove contact '%s'",
	       player->getPlayerName(), otherCharname.c_str());

	// STEPS
	// 1- remove the contact, complain if there's an error

	otherCharname = SrvDBMgr::instance().escape(otherCharname);

	SrvDBQuery query;
	query.setTables("contact_list");
	string condition = "cid='" + string(player->getPlayerID()) +
		"' AND contact_charname='" + otherCharname + "'";
	query.setCondition(condition);
	bool success = SrvDBMgr::instance().queryDelete(&query);
	if (success) {
		sendConsoleReply(player, "Successfully removed contact");
		LogNTC("Success removing contact (player '%s', contact player '%s'",
		       player->getPlayerName(), otherCharname.c_str());
	} else {
		sendConsoleReply(player, "No such player in your contact list");
		LogERR("Error removing contact (player '%s', contact player '%s')",
			player->getPlayerName(), otherCharname.c_str());
	}
}

bool SrvWorldContactMgr::isContactPlaying(const std::string& name)
{
	const LoginData* targetPlayer = SrvLoginMgr::instance().findPlayer(name);
	return (targetPlayer && targetPlayer->isPlaying());
}

void SrvWorldContactMgr::sendConsoleReply(const LoginData* loginData,
					  const std::string& msg)
{
	MsgChat textreply;
	textreply.origin = "Server";
	textreply.type = MsgChat::SYSTEM;
	textreply.text = msg;
	SrvNetworkMgr::instance().sendToPlayer(textreply, loginData);
}

void SrvWorldContactMgr::notifyPlayerOfOneContact(const LoginData* player,
						  const std::string& contactName,
						  char contactType,
						  const std::string& contactComment,
						  const std::string& contactLastLogin,
						  bool connected)
{
	// basic data
	MsgContactStatus statusmsg;
	statusmsg.charname = contactName;
	statusmsg.type = contactType;
	statusmsg.comment = contactComment;
	statusmsg.lastLogin = contactLastLogin;

	// whether it's connected or not
	string consoleTxt = "Your contact '" + contactName + "' is ";
	if (connected) {
		statusmsg.status = 'C';
		consoleTxt += "on-line";
	} else {
		statusmsg.status = 'D';
		consoleTxt += "off-line";
	}

	// send messages
	sendConsoleReply(player, consoleTxt);
	SrvNetworkMgr::instance().sendToPlayer(statusmsg, player);
}

void SrvWorldContactMgr::notifyPlayerOfContacts(const LoginData* player)
{
	// STEPS
	// 1- get the list of contacts of the new player
	// 2- see if they're on-line and send a message with the status
	// of each

	// 1- get the list of contacts of the new player
	string charname = player->getPlayerName();
	string cid = player->getPlayerID();
	SrvDBQuery query;
	query.setTables("usr_chars as ch,contact_list as cl");
	string condition = "cl.cid='" + cid +
		"' AND ch.cid=cl.contact_cid";
	query.setCondition(condition);
	query.setOrder("contact_charname ASC");
	query.addColumnWithoutValue("cl.contact_charname");
	query.addColumnWithoutValue("cl.type");
	query.addColumnWithoutValue("cl.comments");
	query.addColumnWithoutValue("ch.last_login");
	int numresults = SrvDBMgr::instance().querySelect(&query);
	if (numresults == 0) {
		LogDBG("No contacts for player '%s'", charname.c_str());
		return;
	} else if (numresults < 0) {
		LogERR("Unknown error happened getting contacts for player '%s'",
		       charname.c_str());
		return;
	}

	// 2- see if they're on-line and send a message with the status
	// of each
	string contactName, contactType, contactComment, contactLastLogin;
	for (int row = 0; row < numresults; ++row) {
		query.getResult()->getValue(row, "cl.contact_charname", contactName);
		query.getResult()->getValue(row, "cl.type", contactType);
		query.getResult()->getValue(row, "cl.comments", contactComment);
		query.getResult()->getValue(row, "ch.last_login", contactLastLogin);

		notifyPlayerOfOneContact(player,
					 contactName, contactType[0],
					 contactComment, contactLastLogin,
					 isContactPlaying(contactName));
	}

	LogDBG("Sent contact status info to '%s' (%d contacts)",
	       charname.c_str(), numresults);
}

void SrvWorldContactMgr::notifyOtherPlayers(const LoginData* player,
					    bool connected)
{
	// STEPS
	// 1- get the list players who have this one as contact
	// 2- see if they're on-line and send them message with the status of
	// this player

	// 1- get the list of player who have this one as contact
	string charname = player->getPlayerName();
	string cid = player->getPlayerID();
	SrvDBQuery query;
	query.setTables("usr_chars as ch,contact_list as cl");
	string condition = "cl.contact_cid='" + cid +
		"' AND ch.cid=cl.contact_cid";
	query.setCondition(condition);
	query.setOrder("cl.charname ASC");
	query.addColumnWithoutValue("cl.charname");
	query.addColumnWithoutValue("cl.type");
	query.addColumnWithoutValue("cl.comments");
	query.addColumnWithoutValue("ch.last_login");
	int numresults = SrvDBMgr::instance().querySelect(&query);
	if (numresults == 0) {
		LogDBG("Nobody has this player ('%s') as a contact",
		       charname.c_str());
		return;
	} else if (numresults < 0) {
		LogERR("Unknown error happened trying to get people who "
		       "have this player ('%s') as contact",
		       charname.c_str());
		return;
	}

	// 2- see if they're on-line and send them message with the status of
	// this player

	// mafm: last login doesn't change, no need to loop
	string lastLogin;
	query.getResult()->getValue(0, "ch.last_login", lastLogin);
	string otherType, otherComment;
	for (int row = 0; row < numresults; ++row) {
		string otherCharname;
		query.getResult()->getValue(row, "cl.charname", otherCharname);
		query.getResult()->getValue(row, "cl.type", otherType);
		query.getResult()->getValue(row, "cl.comments", otherComment);

		// sending only if they're logged in
		const LoginData* otherPlayer = SrvLoginMgr::instance().findPlayer(otherCharname);
		bool otherConnected = (otherPlayer && otherPlayer->isPlaying());
		if (otherConnected) {
			LogDBG("Sending info to '%s' about contact '%s'",
			       otherCharname.c_str(), charname.c_str());

			notifyPlayerOfOneContact(otherPlayer,
						 charname, otherType[0],
						 otherComment, lastLogin,
						 connected);
		}
	}

	LogDBG("Sent contact status info to other players having '%s'"
	       " (%d contacts in total)",
	       charname.c_str(), numresults);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
