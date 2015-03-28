/*
 * srvtrademgr.h
 * Copyright (C) 2005-2006 by Bryan Duff <duff0097@umn.edu>
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

#ifndef __FEARANN_SERVER_TRADE_TRADE_MGR_H__
#define __FEARANN_SERVER_TRADE_TRADE_MGR_H__


#include "common/patterns/singleton.h"
#include "common/net/msgs.h"


class LoginData;
class MsgCommand;
class SrvEntityPlayer;


/** Class controling the console, deciding what to do with the incoming
 * messages.
 */
class SrvTradeSession
{
public:
	SrvTradeSession() {};
	~SrvTradeSession() {};

	/// Set the inventory list for a player
	void setList(std::vector<InventoryItem> itemList, 
		     const char * player_name);
	/// Set items selected by a player to trade
	void setSelectedList(std::vector<int> itemList, 
			     const char * player_name);
	
	const char* findOtherPlayer(const char* player_name);
	void sendToOtherPlayer( MsgBase& msg, const char* player );

	void setState( MsgTrade::MESSAGE_TYPE _state ) { state = _state; };
	MsgTrade::MESSAGE_TYPE getState( void ) { return state; };

	/// Trade lists
	std::vector<InventoryItem> mTarget_inv;
	std::vector<InventoryItem> mPlayer_inv;
	/// Items selected for trade
	std::vector<int> playerSelectedList;
	std::vector<int> targetSelectedList;

	/// Target - NPC or other player
	std::string target;
	//uint64_t targetID;
	/// Player who initiates the trade
	std::string player;
	//uint64_t playerID;

private:
	MsgTrade::MESSAGE_TYPE state;

};


/** Class controling the console, deciding what to do with the incoming
 * messages.
 */
class SrvTradeMgr : public Singleton<SrvTradeMgr>
{
public:
	/** Finalize, do whatever cleanup needed when the server shuts down. */
	void finalize();

	bool handleTrade(MsgTrade* msg);
	bool addTrade(MsgTrade* msg);
	bool acceptTrade(MsgTrade* msg);
	bool updateList(MsgTrade* msg);
	bool commitTrade(MsgTrade* msg);
	bool removeTrade(const char* player);
	SrvTradeSession* findTrade(const char* player);
	void giveSelectedItem( SrvEntityPlayer* playerEntity,\
			       SrvEntityPlayer* targetEntity,\
			       int entityID );
	void swapSelected( SrvTradeSession* trade );
	void listTrades();

private:
	/** Singleton friend access */
	friend class Singleton<SrvTradeMgr>;

	std::vector<SrvTradeSession*> sessions;


	/** Default constructor */
	SrvTradeMgr();
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
