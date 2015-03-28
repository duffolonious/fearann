/*
 * srvloginmgr.h
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

/**
 * \file srvloginmgr.h
 *
 * This file contains classes managing everything related with the connections
 * to the server
 *
 * @author mafm
 */
  
#ifndef __FEARANN_SERVER_LOGIN_MGR_H__
#define __FEARANN_SERVER_LOGIN_MGR_H__


#include "common/patterns/singleton.h"
#include "common/command.h"

#include <vector>
#include <list>


class Netlink;
class SrvNetworkMgr;
class SrvDBMgr;
class SrvEntityPlayer;


/** Structure for a player connected to the server, containing some accounting
 * data.
 *
 * There are other related structures, such as the network link itself at lower
 * level, and at higher level the entity monitoring the behaviour of the player,
 * with game statistics and so on.
 *
 * @author mafm
 */
class LoginData
{
	friend class SrvLoginMgr;

public:
	/** Get the username */
	const char* getUserName() const;
	/** Get the player name (character name) */
	const char* getPlayerName() const;
	/** Get the player ID */
	const char* getPlayerID() const;
	/** Get the entity (game object for a player) of this player
	 * connection */
	SrvEntityPlayer* getPlayerEntity() const;
	/** Set the entity (game object for a player) of this player
	 * connection */
	void setPlayerEntity(SrvEntityPlayer* e);
	/** Get IP address */
	const char* getIP() const;
	/** Get network object of this player connection */
	Netlink* getNetlink() const;
	/** Whether this player connection is already playing or not */
	bool isPlaying() const;
	/** Set playing state */
	void setPlaying(bool playing);
	/** Whether this player connection is downloading content */
	bool isDownloadingContent() const;
	/** Set downloading state */
	void setDownloadingContent(bool dl);
	/** Get the permission level of this player (to know if is able to
	 * perform some commands) */
	PermLevel::LEVEL getPermissionLevel() const;
	/** Set the permission level of this player (to know if is able to
	 * perform some commands) */
	void setPermissionLevel(PermLevel::LEVEL lvl);

private:
	/// Representation of a connection
	Netlink* netlink;
	/// Username (when the connection logged in as user)
	std::string username;
	/// User id (same as above, but the id in the DB)
	std::string uid;
	/// Charname (when the user has chosen a character to play)
	std::string charname;
	/// Character id (same as above, but the id in the DB)
	std::string cid;
	/// Player behaviour, the representation of a player in the game
	SrvEntityPlayer* entity;
	/// Permission level (admin, plain player, ...), to check permissions to
	/// execute commands
	PermLevel::LEVEL permLevel;
	/// Flag to know if this connection is in the content manager
	bool downloadingContent;
	/// Flag to know if this connection is in the world manager
	bool playingGame;


	/** Constructor */
	LoginData(Netlink* n);
};


/** Manager for all the connections (and disconnections) to the server
 *
 * @author mafm
 */
class SrvLoginMgr : public Singleton<SrvLoginMgr>
{
public:
	/** Finalize, do whatever cleanup needed when the server shuts down. */
	void finalize();

	/** Add a connection (will eventually become a player) */
	void addConnection(Netlink* netlink);
	/** Remove a connection (whatever the state: playing, logged in, etc) */
	void removeConnection(LoginData* loginData);
	/** Remove a dead connection (whatever the state: playing, logged in,
	 * etc), due to the socket being closed */
	void removeConnection(Netlink* netlink);
	
	/** Attend a login request (from plain connection to user) */
	void login(LoginData* loginData, string userName, string pwd);
	/** Attend a request to create a new user */
	void createUser(LoginData* loginData,
			string userName,
			string pwd,
			string eMail,
			string realName);
	/** Attend a request to create a new character */
	void createCharacter(LoginData* loginData,
			     string charName,
			     string race,
			     string gender,
			     string playerClass,
			     uint8_t points_con,
			     uint8_t points_str,
			     uint8_t points_dex,
			     uint8_t points_int,
			     uint8_t points_wis,
			     uint8_t points_cha);
	/** Attend a request to delete a character */
	void deleteCharacter(LoginData* loginData, string charName);
	/** Attend a request to join the game */
	void joinGame(LoginData* loginData, string charName);

	/** Find a LoginData object based on the connection object */
	LoginData* findPlayer(const Netlink* netlink) const;
	/** Find a LoginData object based on the name of the player */
	LoginData* findPlayer(const std::string& playerName) const;
	/** Find a LoginData object based on the id of the player */
	LoginData* findPlayer(EntityID id) const;
	/** Returns the number of accounts */
	int getNumberOfAccounts() const;
	/** Returns the number of characters */
	int getNumberOfCharacters() const;
	/** Returns the number of connections */
	size_t getNumberOfConnections() const;
	/** Returns the number of players */
	size_t getNumberOfConnectionsPlaying() const;
	/** Returns the connection list of all connections (broadcasting) */
	void getAllConnections(std::vector<LoginData*>& allConnections) const;
	/** Returns the connection list of all players (for broadcasting) */
	void getAllConnectionsPlaying(std::vector<LoginData*>& allPlayers) const;
	/** Returns a player name list (used for /who command) */
	void getPlayerNameList(std::list<std::string>& nameList) const;

private:
	/** Singleton friend access */
	friend class Singleton<SrvLoginMgr>;


	/// Player (connection) list
	std::vector<LoginData*> mPlayerList;
	/// Pointer to the network manager
	SrvNetworkMgr* mNetMgr;
	/// Pointer to the database manager
	SrvDBMgr* mDBMgr;

	/// Protocol version for new connections
	std::string mProtocolVersion;
	/// Var to use in several classes, fetched only once
	int mMaxCharsPerAccount;
	/// New area for new characters
	std::string mNewCharArea;
	/// Position for new characters
	std::string mNewCharPosX;
	/// Position for new characters
	std::string mNewCharPosY;
	/// Position for new characters
	std::string mNewCharPosZ;


	/** Default constructor */
	SrvLoginMgr();
};


#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
