/*
 * srvloginmgr.cpp
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

#include "srvloginmgr.h"

#include "common/net/netlayer.h"
#include "common/net/msgs.h"
#include "common/configmgr.h"
#include "common/tablemgr.h"
#include "common/stats.h"
#include "common/util.h"

#include "server/srvmain.h"
#include "server/db/srvdbmgr.h"
#include "server/content/srvcontentmgr.h"
#include "server/entity/srventityplayer.h"
#include "server/net/srvnetworkmgr.h"
#include "server/world/srvworldmgr.h"


/** Small internal class to serve as container when throwing errors, so we can
 * pass several parameters.
 *
 * @author mafm
 */
class SrvLoginError
{
	friend class SrvLoginMgr;
private:
	SrvLoginError(MsgUtils::Errors::Codes e, const std::string& msg) :
		errorCode(e), logMsg(msg) { }
	int errorCode;
	string logMsg;
};


/*******************************************************************************
 * LoginData
 ******************************************************************************/
LoginData::LoginData(Netlink* n) :
	netlink(n),
	username("<none>"), uid("<none>"), charname("<none>"), cid("<none>"),
	entity(0),
	permLevel(PermLevel::NOTSET),
	downloadingContent(false), playingGame(false)
{
}      

const char* LoginData::getUserName() const
{
	return username.c_str();
}

const char* LoginData::getPlayerName() const
{
	return charname.c_str();
}

const char* LoginData::getPlayerID() const
{
	return cid.c_str();
}

SrvEntityPlayer* LoginData::getPlayerEntity() const
{
	return entity;
}

void LoginData::setPlayerEntity(SrvEntityPlayer* e)
{
	entity = e;
}

const char* LoginData::getIP() const
{
	return netlink->getIP();
}

Netlink* LoginData::getNetlink() const
{
	return netlink;
}

bool LoginData::isPlaying() const
{
	return playingGame;
}

void LoginData::setPlaying(bool playing)
{
	playingGame = playing;
}

bool LoginData::isDownloadingContent() const
{
	return downloadingContent;
}

void LoginData::setDownloadingContent(bool dl)
{
	downloadingContent = dl;
}

PermLevel::LEVEL LoginData::getPermissionLevel() const
{
	return permLevel;
}

void LoginData::setPermissionLevel(PermLevel::LEVEL lvl)
{
	permLevel = lvl;
}


/*******************************************************************************
 * SrvLoginMgr
 ******************************************************************************/
template <> SrvLoginMgr* Singleton<SrvLoginMgr>::INSTANCE = 0;

SrvLoginMgr::SrvLoginMgr()
{
	mNetMgr = &SrvNetworkMgr::instance();
	mDBMgr = &SrvDBMgr::instance();

	// fetching vars from config file
	mProtocolVersion = ConfigMgr::instance().getConfigVar("Server.Network.ProtocolVersion", "99");
	if (mProtocolVersion == "99") {
		LogERR("Couldn't get ProtocolVersion variable");
	}
	mMaxCharsPerAccount = atoi(ConfigMgr::instance().
				   getConfigVar("Server.Characters.MaxCharactersPerAccount", "-1"));
	mNewCharArea = ConfigMgr::instance().getConfigVar("Server.Characters.NewCharArea", "-1");
	mNewCharPosX = ConfigMgr::instance().getConfigVar("Server.Characters.NewCharPosX", "-1");
	mNewCharPosY = ConfigMgr::instance().getConfigVar("Server.Characters.NewCharPosY", "-1");
	mNewCharPosZ = ConfigMgr::instance().getConfigVar("Server.Characters.NewCharPosZ", "-1");
	if (mMaxCharsPerAccount == -1
	    || mNewCharArea == "-1" || mNewCharPosX == "-1"
	    || mNewCharPosY == "-1"  || mNewCharPosZ == "-1") {
		LogERR("Couldn't fetch variables for new characters");
	}
}

void SrvLoginMgr::finalize()
{
	while (!mPlayerList.empty()) {
		LoginData* elem = mPlayerList.back();
		mPlayerList.pop_back();
		delete elem;
	}
}

void SrvLoginMgr::addConnection(Netlink* netlink)
{
	// STEPS
	// 1- reload the content, in the case that the admin forgot to do it
	// (useful at least when we are testing a lot of things)
	// 2- Check if already logged in, otherwise register connection
	// 3- get data (server statistics) from the db, and send it

	MsgConnectReply repmsg;

	// 1- reload the content tree
	SrvContentMgr::instance().reloadContentTree();

	// 2- Check if already logged in, otherwise register connection
	LoginData* newConn = new LoginData(netlink);

	/* mafm: disabling only-one-IP restriction, because it's annoying for
	 * testing (can't connect in the same computer...)
	   
	for (vector<LoginData*>::iterator it = mPlayerList.begin();
	     it != mPlayerList.end(); ++it) {
		if ((*it)->ip == newConn->ip) {
			repmsg.resultCode = EALREADYLOGGED; // Login failed (already logged in)
			mNetMgr->sendToPlayer(repmsg, newConn);
			LogWRN("Rejecting new connection, "
			       "already connected from the same IP (%s)",
			       newConn->ip.c_str());
			delete newConn;
			return;
		}
	}
	*/
	mPlayerList.push_back(newConn);

	// 3- get data (server statistics) from the db, and send it
	{
		repmsg.resultCode = MsgUtils::Errors::SUCCESS;
		repmsg.protocolVersion = mProtocolVersion;
		repmsg.uptime = SrvMain::instance().getUptime();
		repmsg.totalUsers = getNumberOfAccounts();
		repmsg.totalChars = getNumberOfCharacters();
		repmsg.currentPlayers = getNumberOfConnectionsPlaying();
		mNetMgr->sendToPlayer(repmsg, newConn);
		LogNTC("Sending connect reply info (IP: '%s')",
		       newConn->netlink->getIP());
	}

	/* test data, to check if the serialization/deserialization process is
	 * done correctly

	{
		MsgTestDataTypes testmsg;
		mNetMgr->sendToPlayer(testmsg, newConn);
	}
	*/
}

void SrvLoginMgr::removeConnection(LoginData* loginData)
{
	removeConnection(loginData->netlink);
}

void SrvLoginMgr::removeConnection(Netlink* netlink)
{
	for (vector<LoginData*>::iterator it = mPlayerList.begin();
	     it != mPlayerList.end(); ++it) {
		if ((*it)->netlink == netlink) {
			LogNTC("Removing dead connection (usr: '%s', char: '%s', IP: '%s')",
			       (*it)->getUserName(),
			       (*it)->getPlayerName(),
			       (*it)->getIP());

			// remove from content manager
			if ((*it)->isDownloadingContent()) {
				LogDBG("Player downloading content, removing from ContentMgr");
				SrvContentMgr::instance().removeConnection(*it);
			}

			// remove from world manager
			if ((*it)->isPlaying()) {
				LogDBG("Player playing, removing from WorldMgr");
				SrvWorldMgr::instance().removePlayer(*it);

				// update time playing
				string charname = (*it)->charname;
				SrvDBQuery query;
				query.setTables("usr_chars");
				query.setCondition("charname='" + charname + "'");
				query.addColumnWithValue("time_playing",
							 "time_playing+CURRENT_TIMESTAMP-last_login",
							 false);
				bool success = mDBMgr->queryUpdate(&query);
				if (!success) {
					LogERR("Couldn't update time_playing for character '%s'",
					       charname.c_str());
				}
			}

			// remove from this one
			delete *it;
			mPlayerList.erase(it);
			return;
		}
	}
}

void SrvLoginMgr::login(LoginData* loginData, string username, string pwd)
{
	// STEPS
	// 1- get data from the client form, check if the usr exists in db
	// 2- check password 
	// 3- update user data (last_login, etc)
	// 4- send back data to user (chars available)

	MsgLoginReply repmsg;

	try {
		// 1- get username data
		username = mDBMgr->escape(username);
		string uid, passworddb;
		{
			SrvDBQuery query;
			query.addColumnWithoutValue("uid");
			query.addColumnWithoutValue("password");
			query.setTables("usr_accts");
			query.setCondition("username='" + username + "'");
			int numresults = mDBMgr->querySelect(&query);
			if (numresults != 1) {
				string logMsg = StrFmt("No such user '%s', numresults %d",
						       username.c_str(), numresults);
				throw SrvLoginError(MsgUtils::Errors::EBADLOGIN, logMsg);
			}
			query.getResult()->getValue(0, "uid", uid);
			query.getResult()->getValue(0, "password", passworddb);
		}

		// 2- password cheching
		if (passworddb != pwd) {
			string logMsg = StrFmt("Wrong password for '%s'", username.c_str());
			throw SrvLoginError(MsgUtils::Errors::EBADLOGIN, logMsg);
		}
		loginData->username = username;
		loginData->uid = uid;

		// 3- updating last login
		{
			SrvDBQuery query;
			query.setTables("usr_accts");
			query.setCondition("uid='" + uid + "'");
			query.addColumnWithValue("last_login", "CURRENT_TIMESTAMP", false);
			query.addColumnWithValue("number_logins", "number_logins+1", false);
			query.addColumnWithValue("last_login_ip", loginData->getIP());
			bool success = mDBMgr->queryUpdate(&query);
			if (!success) {
				string logMsg = StrFmt("Couldn't update last_login for '%s'",
						       username.c_str());
				throw SrvLoginError(MsgUtils::Errors::EDATABASE, logMsg);
			}
		}

		// 4. sending back data to user (chars available)
		{
			SrvDBQuery query;
			query.setTables("usr_chars");
			query.setCondition("uid='" + uid + "' AND status='0'");
			query.addColumnWithoutValue("charname");
			query.addColumnWithoutValue("race");
			query.addColumnWithoutValue("gender");
			query.addColumnWithoutValue("class");
			query.addColumnWithoutValue("area");
			int numresults = mDBMgr->querySelect(&query);
			if (numresults > mMaxCharsPerAccount) {
				string logMsg = StrFmt("This user has more than %d characters ('%s', num. chars %d)",
							    mMaxCharsPerAccount, username.c_str(), numresults);
				throw SrvLoginError(MsgUtils::Errors::ECHARCORRUPT, logMsg);
			}

			repmsg.resultCode = MsgUtils::Errors::SUCCESS;
			repmsg.charNumber = numresults;
			for (int row = 0; row < numresults; ++row) {
				string name, race, gender, playerClass, area;
				query.getResult()->getValue(row, "charname", name);
				query.getResult()->getValue(row, "race", race);
				query.getResult()->getValue(row, "gender", gender);
				query.getResult()->getValue(row, "class", playerClass);
				query.getResult()->getValue(row, "area", area);
				repmsg.addCharacter(name, race, gender, playerClass, area);
			}
			mNetMgr->sendToPlayer(repmsg, loginData);
			LogNTC("User login successful: '%s'", username.c_str());
		}
	} catch (SrvLoginError& e) {
		repmsg.resultCode = e.errorCode;
		mNetMgr->sendToPlayer(repmsg, loginData);
		LogWRN("%s", e.logMsg.c_str());
		return;
	}
}

void SrvLoginMgr::createUser(LoginData* loginData,
			     string username,
			     string pwd,
			     string email,
			     string realname)
{
	// STEPS
	// 1- get data from the client form
	// 2- check if username already exists
	// 3- create the user account

	MsgNewUserReply repmsg;

	try {
		// 1- get and prepare the data from the client
		username = mDBMgr->escape(username);
		pwd = mDBMgr->escape(pwd);
		email = mDBMgr->escape(email);
		realname = mDBMgr->escape(realname);

		// 2- check if username already exists
		{
			SrvDBQuery query;
			query.setTables("usr_accts");
			query.setCondition("username='" + username + "'");
			bool matches = mDBMgr->queryMatch(&query);
			if (matches) {
				string logMsg = StrFmt("Create new user: already exists ('%s')",
						       username.c_str());
				throw SrvLoginError(MsgUtils::Errors::EUSERALREADYEXIST, logMsg);
			}
		}

		// 3- insert new data into the usr_accts
		{
			SrvDBQuery query;
			query.setTables("usr_accts");
			query.addColumnWithValue("username", username);
			query.addColumnWithValue("password", pwd);
			query.addColumnWithValue("email", email);
			query.addColumnWithValue("realname", realname);
			query.addColumnWithValue("roles",
						 StrFmt("%d", static_cast<int>(PermLevel::PLAYER)));
			//query.addColumnWithValue("creation_date", "CURRENT_TIMESTAMP", false);
			bool success = mDBMgr->queryInsert(&query);
			if (!success) {
				string logMsg = StrFmt("Create new user: failed because of DB problems ('%s')",
						       username.c_str());
				throw SrvLoginError(MsgUtils::Errors::EDATABASE, logMsg);
			}

			repmsg.resultCode = MsgUtils::Errors::SUCCESS;
			mNetMgr->sendToPlayer(repmsg, loginData);
			LogNTC("New user created successfully: '%s'", username.c_str());
		}
	} catch (SrvLoginError& e) {
		repmsg.resultCode = e.errorCode;
		mNetMgr->sendToPlayer(repmsg, loginData);
		LogWRN("%s", e.logMsg.c_str());
		return;
	}
}

void SrvLoginMgr::createCharacter(LoginData* loginData,
				  string charname,
				  string race,
				  string gender,
				  string playerClass,
				  uint8_t points_con,
				  uint8_t points_str,
				  uint8_t points_dex,
				  uint8_t points_int,
				  uint8_t points_wis,
				  uint8_t points_cha)
{
	// STEPS
	// 1- get data from the client form
	// 2- check if the character already exists
	// 3- check if the user has <= 8 active chars
	// 4- check if race, name and gender (and other data?) is appropriate
	// 5- create the new character

	MsgNewCharReply repmsg;

	try {
		// 1- get data from the client form
		charname = mDBMgr->escape(charname);
		race = mDBMgr->escape(race);
		gender = mDBMgr->escape(gender);
		playerClass = mDBMgr->escape(playerClass);

		// 2- check if the character already exists
		{
			SrvDBQuery query;
			query.setTables("usr_chars");
			query.setCondition("charname='" + charname + "'");
			bool matches = mDBMgr->queryMatch(&query);
			if (matches) {
				string logMsg = StrFmt("Create new character: already exists ('%s')",
						       charname.c_str());
				throw SrvLoginError(MsgUtils::Errors::ECHARALREADYEXIST, logMsg);
			}
		}
	

		// 3- check if the user has <ALLOWED active chars
		{
			SrvDBQuery query;
			query.setTables("usr_chars");
			query.setCondition("uid='" + loginData->uid + "' AND status='0'");
			int numresults = mDBMgr->queryMatchNumber(&query);
			if (numresults >= mMaxCharsPerAccount) {
				string logMsg = StrFmt("Create new character: too many chars (user '%s', %d)",
						       loginData->username.c_str(), numresults);
				throw SrvLoginError(MsgUtils::Errors::EMAXCHARS, logMsg);
			}
		}

		// 4- check if race, name and gender (and other data?) is appropriate
		if (!(race == "dwarf" || race == "elf" || race == "human")
		    || !(gender == "f" || gender == "m")
		    || !(playerClass == "fighter" || playerClass == "sorcerer") ) {
			string logMsg = StrFmt("Create new character: bad data "
					       "(user %s, race '%s', gender '%s', class '%s')",
					       loginData->username.c_str(), race.c_str(), 
					       gender.c_str(), playerClass.c_str());
			throw SrvLoginError(MsgUtils::Errors::ENEWCHARBADDATA, logMsg);
		}

		// abilities must be greater than 3 and less than 18, and if all
		// points are used, the total should equal 78 (avg 13 per
		// ability)

		int abilities_sum = points_con + points_str + points_dex +
			points_int + points_wis + points_cha;
		if (78 != abilities_sum
		    || points_str < 3 || points_str > 18
		    || points_dex < 3 || points_dex > 18
		    || points_con < 3 || points_con > 18
		    || points_int < 3 || points_int > 18
		    || points_wis < 3 || points_wis > 18
		    || points_cha < 3 || points_cha > 18) {
			string logMsg = StrFmt("Create new character: bad data (user '%s', points %d,"
					       " con=%u str=%u agi=%u int=%u wis=%u cha=%u)",
					       loginData->username.c_str(), abilities_sum,
					       points_con, points_str, points_dex,
					       points_int, points_wis, points_cha);
			throw SrvLoginError(MsgUtils::Errors::ENEWCHARBADDATA, logMsg);
		}

		// 5- create the new character
		{
			SrvDBQuery query;
			query.setTables("usr_chars");
			query.addColumnWithValue("uid", loginData->uid);
			//query.addColumnWithValue("cid", "DEFAULT", false);
			query.addColumnWithValue("charname", charname);
			query.addColumnWithValue("area", mNewCharArea);
			query.addColumnWithValue("pos1", mNewCharPosX, false);
			query.addColumnWithValue("pos2", mNewCharPosY, false);
			query.addColumnWithValue("pos3", mNewCharPosZ, false);
			query.addColumnWithValue("rot", "0.0", false);
			query.addColumnWithValue("race", race);
			query.addColumnWithValue("gender", gender);
			query.addColumnWithValue("class", playerClass);
			bool success = mDBMgr->queryInsert(&query);

			// sum base points plus player selected
			int tot_con = points_con;
			int tot_str = points_str;
			int tot_dex = points_dex;
			int tot_int = points_int;
			int tot_wis = points_wis;
			int tot_cha = points_cha;

			// add race bonus - using d20 race modifiers
			if (race == "elf") {
				tot_con -= 2;
				tot_dex += 2;
			} else if (race == "dwarf") {
				tot_con += 2;
				tot_dex -= 2;
			} else if (race == "human") {
				// no change
			} else {
				throw SrvLoginError(MsgUtils::Errors::ENEWCHARBADDATA, "unknown race");
			}

			string ab_con = StrFmt("%i", tot_con);
			string ab_str = StrFmt("%i", tot_str);
			string ab_dex = StrFmt("%i", tot_dex);
			string ab_int = StrFmt("%i", tot_int);
			string ab_wis = StrFmt("%i", tot_wis);
			string ab_cha = StrFmt("%i", tot_cha);

			// health: look at stats.h
			int health = (10
				      + Stats::getAbilityModifier(tot_con)
				      + Stats::getAbilityModifier(tot_str));

			/** \todo: duffolonious: handle magic differently.  The
			 * magic score will be calculated as such: (memorized
			 * spells)/(total spells) = magic.  Also, the system is
			 * weighted, so a level three spell is worth 4 points
			 * (because level 0 spells need to be worth a point)
			 * This is low priority.
			 */
			string magic = "0";

			SrvDBQuery query2;
			query2.setTables("player_stats");
			query2.addColumnWithValue("charname", charname);
			query2.addColumnWithValue("health", StrFmt("%i", health));
			query2.addColumnWithValue("magic", magic);
			query2.addColumnWithValue("stamina", "'100'", false);
			query2.addColumnWithValue("ab_con", ab_con);
			query2.addColumnWithValue("ab_str", ab_str);
			query2.addColumnWithValue("ab_dex", ab_dex);
			query2.addColumnWithValue("ab_int", ab_int);
			query2.addColumnWithValue("ab_wis", ab_wis);
			query2.addColumnWithValue("ab_cha", ab_cha);
			query2.addColumnWithValue("level", "1");
			success = mDBMgr->queryInsert(&query2);
			if (!success) {
				string logMsg = StrFmt("Create new character: DB failure (user '%s', char '%s')",
							    loginData->username.c_str(), charname.c_str());
				throw SrvLoginError(MsgUtils::Errors::EDATABASE, logMsg);
			}

			repmsg.resultCode = MsgUtils::Errors::SUCCESS; // success
			repmsg.charname = charname;
			repmsg.race = race;
			repmsg.gender = gender;
			repmsg.area = mNewCharArea;
			mNetMgr->sendToPlayer(repmsg, loginData);
			LogNTC("New character created successfully (user '%s', char '%s')",
			       loginData->username.c_str(), charname.c_str());
		}
	} catch (SrvLoginError& e) {
		repmsg.resultCode = e.errorCode;
		mNetMgr->sendToPlayer(repmsg, loginData);
		LogWRN("%s", e.logMsg.c_str());
		return;
	}
}

void SrvLoginMgr::deleteCharacter(LoginData* loginData, string charname)
{
	// STEPS
	// 1- get data from the client form
	// 2- check if the character already exists and belongs to the user
	// 3- delete the new character

	MsgDelCharReply repmsg;

	try {
		// 1- get data from the client form
		charname = mDBMgr->escape(charname);

		// 2- check if already exists and belongs to the user
		{
			SrvDBQuery query;
			query.setTables("usr_chars");
			query.setCondition("uid='" + loginData->uid
					   + "' AND charname='" + charname + "'");
			bool matches = mDBMgr->queryMatch(&query);
			if (!matches) {
				string logMsg = StrFmt("Delete new character: doesn't exist or doesn't belong to"
						       " (user '%s', char '%s')",
						       loginData->username.c_str(), charname.c_str());
				throw SrvLoginError(MsgUtils::Errors::ENOSUCHCHAR, logMsg);
			}
		}

		// 3- delete the character
		{
			SrvDBQuery query;
			query.setTables("usr_chars");
			query.setCondition("charname='" + charname + "'");
			query.addColumnWithValue("status", "1", false);
			int numresults = mDBMgr->queryUpdate(&query);
			if (numresults != 1) {
				string logMsg = StrFmt("Delete new character: DB failure (user '%s', char '%s')",
						       loginData->username.c_str(), charname.c_str());
				throw SrvLoginError(MsgUtils::Errors::ENOSUCHCHAR, logMsg);
			}

			repmsg.resultCode = MsgUtils::Errors::SUCCESS;
			repmsg.charname = charname;
			mNetMgr->sendToPlayer(repmsg, loginData);
			LogNTC("Character deleted successfully (user '%s', char '%s')",
			       loginData->username.c_str(), charname.c_str());
		}
	} catch (SrvLoginError& e) {
                repmsg.resultCode = e.errorCode;
                mNetMgr->sendToPlayer(repmsg, loginData);
                LogWRN("%s", e.logMsg.c_str());
                return;
        }
}

void SrvLoginMgr::joinGame(LoginData* loginData, string charname)
{
	// STEPS
	// 1- get data from the client form
	// 2- check if the usr/char already joined the game with any character
	// 3- check if the character belongs to the user, and get the data
	// 4- join game

	MsgJoinReply repmsg;

	try {
		// 1- get data from the client form
		charname = mDBMgr->escape(charname);
		string playerClass, cid;

		// 2- check if the usr/char already joined the game with any character
		if (loginData->charname != "<none>") {
			string logMsg = StrFmt("Joining game: already playing (char '%s')",
					       loginData->charname.c_str());
			throw SrvLoginError(MsgUtils::Errors::EALREADYPLAYING, logMsg);
		}

		// 3- check if the character belongs to the user, and get the data
		MsgEntityCreate msgBasic;
		MsgEntityMove msgMove;
		MsgPlayerData msgPlayer;
		{
			LogNTC("uid: %s; charname '%s'", loginData->uid.c_str(), charname.c_str());
			SrvDBQuery query;
			query.setTables("usr_chars,usr_accts");
			string condition = "charname='" + charname
				+ "' AND usr_accts.uid='" + loginData->uid
				+ "' AND usr_accts.uid=usr_chars.uid";
			query.setCondition(condition);
			query.addColumnWithoutValue("cid");
			query.addColumnWithoutValue("area");
			query.addColumnWithoutValue("pos1");
			query.addColumnWithoutValue("pos2");
			query.addColumnWithoutValue("pos3");
			query.addColumnWithoutValue("rot");
			query.addColumnWithoutValue("race");
			query.addColumnWithoutValue("gender");
			query.addColumnWithoutValue("class");
			query.addColumnWithoutValue("roles");
			int numresults = mDBMgr->querySelect(&query);
			if (numresults != 1) {
				string logMsg = StrFmt("Joining game: no such char (char '%s', numresults %d)",
						       loginData->charname.c_str(), numresults);
				throw SrvLoginError(MsgUtils::Errors::ENOSUCHCHAR, logMsg);
			}

			float pos1 = 0.0f, pos2 = 0.0f, pos3 = 0.0f, rot = 0.0f;
			int roles = 0;
			query.getResult()->getValue(0, "cid", cid);
			query.getResult()->getValue(0, "area", msgMove.area);
			query.getResult()->getValue(0, "pos1", pos1);
			query.getResult()->getValue(0, "pos2", pos2);
			query.getResult()->getValue(0, "pos3", pos3);
			query.getResult()->getValue(0, "rot", rot);
			query.getResult()->getValue(0, "race", msgBasic.meshType);
			query.getResult()->getValue(0, "gender", msgBasic.meshSubtype);
			query.getResult()->getValue(0, "class", playerClass);
			query.getResult()->getValue(0, "roles", roles);

			msgMove.position = Vector3(pos1, pos2, pos3);
			msgMove.rot = rot;
			msgMove.entityID = StrToUInt64(cid.c_str());
			msgBasic.entityID = msgMove.entityID;
			msgBasic.area = msgMove.area;
			msgBasic.entityName = charname;
			msgBasic.entityClass = "Player";
			msgBasic.position = msgMove.position;
			msgBasic.rot = msgMove.rot;
			if (roles == static_cast<int>(PermLevel::PLAYER)) {
				loginData->setPermissionLevel(PermLevel::PLAYER);
			} else if (roles == static_cast<int>(PermLevel::ADMIN)) {
				loginData->setPermissionLevel(PermLevel::ADMIN);
			} else {
				LogERR("The permission stored in the DB for player '%s'"
				       " is not recognized: %d",
				       charname.c_str(), roles);
			}

			// get player statistics
			SrvDBQuery query2;
			query2.setTables("player_stats");
			query2.setCondition("charname='" + charname + "'");
			query2.addColumnWithoutValue("health");
			query2.addColumnWithoutValue("magic");
			query2.addColumnWithoutValue("stamina");
			query2.addColumnWithoutValue("gold");
			query2.addColumnWithoutValue("level");
			query2.addColumnWithoutValue("ab_con");
			query2.addColumnWithoutValue("ab_str");
			query2.addColumnWithoutValue("ab_dex");
			query2.addColumnWithoutValue("ab_int");
			query2.addColumnWithoutValue("ab_wis");
			query2.addColumnWithoutValue("ab_cha");
			numresults = mDBMgr->querySelect(&query2);
			if (numresults != 1) {
				string logMsg = StrFmt("Joining game: no such char stats (char '%s', numresults %d)",
						       loginData->charname.c_str(), numresults);
				throw SrvLoginError(MsgUtils::Errors::ENOSUCHCHAR, logMsg);
			}

			int health_cur = 0, magic_cur = 0, stamina = 0, gold = 0, level = 0;
			int ab_con = 0, ab_str = 0, ab_dex = 0, ab_int = 0, ab_wis = 0, ab_cha = 0;
			query2.getResult()->getValue(0, "health", health_cur);
			query2.getResult()->getValue(0, "magic", magic_cur);
			query2.getResult()->getValue(0, "stamina", stamina);
			query2.getResult()->getValue(0, "gold", gold);
			query2.getResult()->getValue(0, "level", level);
			query2.getResult()->getValue(0, "ab_con", ab_con);
			query2.getResult()->getValue(0, "ab_str", ab_str);
			query2.getResult()->getValue(0, "ab_dex", ab_dex);
			query2.getResult()->getValue(0, "ab_int", ab_int);
			query2.getResult()->getValue(0, "ab_wis", ab_wis);
			query2.getResult()->getValue(0, "ab_cha", ab_cha);

			msgPlayer.health_cur = health_cur;
			msgPlayer.magic_cur = magic_cur;
			msgPlayer.stamina = stamina;
			msgPlayer.gold = gold;
			msgPlayer.level = level;
			msgPlayer.ab_con = ab_con;
			msgPlayer.ab_str = ab_str;
			msgPlayer.ab_dex = ab_dex;
			msgPlayer.ab_int = ab_int;
			msgPlayer.ab_wis = ab_wis;
			msgPlayer.ab_cha = ab_cha;

			/// \todo duffolonious: Here is a hack to set max health
			/// (soon load) currectly.
			PlayerInfo tmp;
			tmp.setProperty("strength", ab_str);
			tmp.setProperty("constitution", ab_con);
			msgPlayer.health_max = tmp.getHP();
			msgPlayer.magic_max = 100;
			string strength = StrFmt("%d", ab_str);
			msgPlayer.load_max = TableMgr::instance().getTable("load")->getValueAsInt(strength, "light");
			msgPlayer.load_cur = 0;
		}

		// 4- join game
		{
			SrvDBQuery query;
			query.setTables("usr_chars");
			query.setCondition("charname='" + charname + "'");
			query.addColumnWithValue("last_login", "CURRENT_TIMESTAMP", false);
			query.addColumnWithValue("number_logins", "number_logins+1", false);
			bool success = mDBMgr->queryUpdate(&query);
			if (!success) {
				string logMsg = StrFmt("Joining game: DB failure (char '%s')",
						       loginData->charname.c_str());
				throw SrvLoginError(MsgUtils::Errors::EDATABASE, logMsg);
			}
		}

		LogDBG("charname '%s' joining ...", charname.c_str());
		loginData->charname = charname;
		loginData->cid = cid;
		SrvEntityPlayer* player = new SrvEntityPlayer(msgBasic, msgMove, msgPlayer, loginData);
		player->getPlayerInfo()->setClass(playerClass);
		loginData->setPlayerEntity(player);
		SrvWorldMgr::instance().addPlayer(loginData);

	} catch (SrvLoginError& e) {
                repmsg.resultCode = e.errorCode;
                mNetMgr->sendToPlayer(repmsg, loginData);
                LogWRN("%s", e.logMsg.c_str());
                return;
       }
}

LoginData* SrvLoginMgr::findPlayer(const Netlink* netlink) const
{
	for (vector<LoginData*>::const_iterator it = mPlayerList.begin();
	     it != mPlayerList.end(); ++it) {
		if ((*it)->netlink == netlink) {
			return *it;
		}
	}
	LogWRN("Cannot find LoginData for netlink (socket %d, IP '%s')",
	       netlink->getSocket(), netlink->getIP());
	return 0;
}

LoginData* SrvLoginMgr::findPlayer(const std::string& playerName) const
{
	for (vector<LoginData*>::const_iterator it = mPlayerList.begin(); 
	     it != mPlayerList.end(); ++it) {
		if ((*it)->charname == playerName) {
			return *it;
		}
	}
	LogWRN("Cannot find LoginData for player name '%s')",
	       playerName.c_str());
	return 0;
}

LoginData* SrvLoginMgr::findPlayer(EntityID id) const
{
	for (vector<LoginData*>::const_iterator it = mPlayerList.begin(); 
	     it != mPlayerList.end(); ++it) {
		if (id == StrToUInt64((*it)->cid.c_str())) {
			return *it;
		}
	}
	LogWRN("Cannot find LoginData for player id '%lu')", id);
	return 0;
}

int SrvLoginMgr::getNumberOfAccounts() const
{
	SrvDBQuery query;
	query.setTables("usr_accts");
	query.setCondition("1=1");
	return SrvDBMgr::instance().queryMatchNumber(&query);
}

int SrvLoginMgr::getNumberOfCharacters() const
{
	SrvDBQuery query;
	query.setTables("usr_chars");
	query.setCondition("1=1");
	return SrvDBMgr::instance().queryMatchNumber(&query);
}

size_t SrvLoginMgr::getNumberOfConnections() const
{
	return mPlayerList.size();
}

size_t SrvLoginMgr::getNumberOfConnectionsPlaying() const
{
	size_t count = 0;
	for (size_t i = 0; i < mPlayerList.size(); ++i) {
		if (mPlayerList[i]->isPlaying()) {
			++count;
		}
	}
	return count;
}

void SrvLoginMgr::getAllConnections(vector<LoginData*>& allConnections) const
{
	allConnections = mPlayerList;
}

void SrvLoginMgr::getAllConnectionsPlaying(vector<LoginData*>& allPlayers) const
{
	for (size_t i = 0; i < mPlayerList.size(); ++i) {
		if (mPlayerList[i]->isPlaying()) {
			allPlayers.push_back(mPlayerList[i]);
		}
	}
}

void SrvLoginMgr::getPlayerNameList(list<string>& nameList) const
{
	for (size_t i = 0; i < mPlayerList.size(); ++i) {
		if (mPlayerList[i]->isPlaying()) {
			nameList.push_back(mPlayerList[i]->getPlayerName());
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
