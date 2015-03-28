/*
 * srvtrademgr.cpp
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

#include "config.h"

#include "srvtrademgr.h"

#include "common/net/msgs.h"
#include "common/configmgr.h"

#include "server/entity/srventityplayer.h"
#include "server/login/srvloginmgr.h"
#include "server/net/srvnetworkmgr.h"
#include "server/world/srvworldmgr.h"


void SrvTradeSession::setSelectedList(std::vector<int> itemList, 
				      const char* player_name)
{
	if (state != MsgTrade::ACCEPT)
		return;

	if (player.compare(player_name) == 0)
	{
		playerSelectedList.assign( itemList.begin(), itemList.end() );
	}
	else if (target.compare(player_name) == 0)
	{
		targetSelectedList.assign( itemList.begin(), itemList.end() );
	}
	else
		LogERR("player not found");
}


void SrvTradeSession::setList(std::vector<InventoryItem> itemList, 
			      const char* player_name)
{
	if (state != MsgTrade::ACCEPT)
		return;

	if (player.compare(player_name) == 0)
	{
		mPlayer_inv.assign( itemList.begin(), itemList.end() );
	}
	else if (target.compare(player_name) == 0)
	{
		mTarget_inv.assign( itemList.begin(), itemList.end() );
	}
	else
		LogERR("player not found");
}


const char * SrvTradeSession::findOtherPlayer(const char * player_name)
{
	/// This function sends to the trading partner that the "player" is
	/// trading with.
	if (player.compare(player_name) == 0)
	{
		return target.c_str();
	}
	else if (target.compare(player_name) == 0)
	{
		return player.c_str();
	}
	else
		LogERR("player not found");

	return 0;
}


void SrvTradeSession::sendToOtherPlayer(MsgBase& msg, const char* playerName)
{
	try {
		LoginData* targetData = SrvLoginMgr::instance().findPlayer(playerName);
		if (!targetData)
			throw "Login data for target not found.";
		
		SrvNetworkMgr::instance().sendToPlayer(msg, targetData);
	} catch (const char * error) {
		LogERR("SrvTradeMgr::sendToPlayer() failed: %s", error);
	}
}


//---------------------------- SrvTradeMgr ---------------------------
template <> SrvTradeMgr* Singleton<SrvTradeMgr>::INSTANCE = 0;

SrvTradeMgr::SrvTradeMgr()
{
}

void SrvTradeMgr::finalize()
{
	for (size_t i = 0; i < sessions.size(); ++i) {
		delete sessions[i];
	}
	sessions.clear();
}

bool SrvTradeMgr::handleTrade(MsgTrade* msg)
{
	LogDBG("handleTrade: player name is '%s'", msg->player.c_str() );
	LogDBG("handleTrade: target name is '%s'", msg->target.c_str() );

	LoginData* player = SrvLoginMgr::instance().findPlayer(msg->player.c_str());

	if ( !player || !player->isPlaying() )
		LogERR("Can't trade without player sending message.");
	else if ( msg->player == msg->target )
	{
		LogERR("handleTrade: '%s' can't trade with self or without", msg->player.c_str() );
		return false;
	}

	LoginData* target;
	if ( msg->getState() != MsgTrade::START )
	{
		SrvTradeSession* tmpSession = findTrade( msg->player.c_str() );
		target = SrvLoginMgr::instance().findPlayer( 
						tmpSession->findOtherPlayer( msg->player.c_str() ) );
	}
	else
	{	
		target = SrvLoginMgr::instance().findPlayer( msg->target.c_str() );
	}

	if ( !target || !target->isPlaying() ) {
		LogWRN("handleTrade: '%s' is not playing, ignoring message", msg->target.c_str() );
		/// \todo duffolonious: also cleanup started trade sessions
		return false;
	}

	/** Main stages:
	 * 		START - the trade has been initiated
	 *		ACCEPT - trade accept, item lists can be submitted
	 * 		COMMIT - items have been selected, and a commit has been initiated
	 *		END - trading has ended - this can be done at any point.
	 */
	switch(msg->getState())
	{
	case MsgTrade::START:
		return addTrade(msg);
	case MsgTrade::END:
		// send message to other player
		return removeTrade(msg->player.c_str());
	case MsgTrade::ACCEPT:
		/** start trading - allow temporary inventory to accept items for
		    trading */
		return acceptTrade(msg);
	case MsgTrade::UPDATE_LIST:
		// Update lists in trade session.
		// Send update to other player.
		return updateList(msg);
	case MsgTrade::COMMIT:
	case MsgTrade::COMMIT_ACCEPT:
	case MsgTrade::COMMIT_REJECT:
		return commitTrade(msg);
	default:
		return false;
	}
}


bool SrvTradeMgr::addTrade(MsgTrade* msg)
{
	LogDBG("adding trade: ('%s', target: '%s')",
	       msg->player.c_str(),
	       msg->target.c_str());

	// Check if player/target already trading
	if ( findTrade(msg->player.c_str()) || findTrade(msg->player.c_str()) )
	{
		LogERR("addTrade: One of these players is already trading.");
		return false;
	}

	SrvTradeSession* session = new SrvTradeSession();
	session->target = string(msg->target);
	session->player = string(msg->player);
	session->setState( msg->getState() );
	//session->setList( msg->itemList, msg->player.c_str() );
	sessions.push_back(session);

	// Send trade message to other player
	MsgTrade msgtrade;
	msgtrade.type = MsgTrade::START;
	// The target for the player receiving the message
	msgtrade.target = msg->player.c_str();

	const char* target = msg->target.c_str(); ///Send to player to accept.
	LoginData* targetData = SrvLoginMgr::instance().findPlayer( target );
	if ( !targetData )
		return false;
	SrvNetworkMgr::instance().sendToPlayer( msgtrade, targetData );
	return true;
}


bool SrvTradeMgr::acceptTrade(MsgTrade* msg)
{
	// Accept a trade session from another player
	SrvTradeSession* tmpSession = findTrade(msg->player.c_str());
	if (!tmpSession && tmpSession->getState() != MsgTrade::START)
	{
		LogERR("acceptTrade: no trade to accept.");
		return false;
	}

	tmpSession->setState( msg->getState() );

	// Send accept trade msg to target (initiating player)
	MsgTrade msgtrade;
	msgtrade.type = MsgTrade::ACCEPT;

	tmpSession->sendToOtherPlayer( msgtrade, msg->player.c_str() );
	return true;
}

bool SrvTradeMgr::updateList(MsgTrade* msg)
{
	const char * player = msg->player.c_str();

	// Player updates items list - sends updated list to other player
	SrvTradeSession* tmpSession = findTrade( player );

	if (!tmpSession && tmpSession->getState() != MsgTrade::ACCEPT)
		return false;

	// Update internal server list.
	tmpSession->setList( msg->itemList, player);
	LogDBG("Player '%s' set trade list", player );

	// Send message to other player to update list
	// Also this action should deselect any items in this player's list.
	MsgTrade msgtrade;
	msgtrade.type = MsgTrade::UPDATE_LIST;
	msgtrade.target = msg->target;
	msgtrade.itemList.assign( msg->itemList.begin(), msg->itemList.end() );

	tmpSession->sendToOtherPlayer( msgtrade, player );
	LogDBG("Msg sent - UPDATE_LIST");
	return true;
}


bool SrvTradeMgr::commitTrade(MsgTrade* msg)
{
	const char * player = msg->player.c_str();
	//Player updates items list - sends updated list to other player
	SrvTradeSession* tmpSession = findTrade( player );
	if (!tmpSession)
		return false;

	//Asks to commit a trade - sends out the items to be traded to
	//the other Player.
	switch(msg->getState())
	{
	case MsgTrade::COMMIT:
		//Send commit - selected items you want to trade.
	{
		if (tmpSession->getState() == MsgTrade::ACCEPT)
			tmpSession->setState( MsgTrade::COMMIT );
		else
		{
			LogERR("COMMIT - Trade not in accept stage.");
			return false;
		}

		//select commit items.
		if ( tmpSession->player == msg->player )
		{
			tmpSession->playerSelectedList.assign( msg->playerSelectedList.begin(),
						       msg->playerSelectedList.end() );
			tmpSession->targetSelectedList.assign( msg->targetSelectedList.begin(),
						       msg->targetSelectedList.end() );
		}
		else
		{
			tmpSession->playerSelectedList.assign( msg->targetSelectedList.begin(),
						       msg->targetSelectedList.end() );
			tmpSession->targetSelectedList.assign( msg->playerSelectedList.begin(),
						       msg->playerSelectedList.end() );
		}

		//Setup and send commit to target
		MsgTrade msgtrade;
		msgtrade.type = MsgTrade::COMMIT;
		msgtrade.playerSelectedList.assign( msg->targetSelectedList.begin(),
						    msg->targetSelectedList.end() );
		msgtrade.targetSelectedList.assign( msg->playerSelectedList.begin(),
						    msg->playerSelectedList.end() );

		LogDBG("Player '%s' is committing trade", player);
		tmpSession->sendToOtherPlayer( msgtrade, player );
		return true;
	}
	break;
	case MsgTrade::COMMIT_ACCEPT:
		///Accept commit - swap selected items.
	{
		if (tmpSession->getState() != MsgTrade::COMMIT)
			return false;
		else
			tmpSession->setState( MsgTrade::ACCEPT );

		///relay msg to target
		MsgTrade msgtrade;
		msgtrade.type = MsgTrade::COMMIT_ACCEPT;

		LogDBG("Player '%s' accepting commit", player);
		tmpSession->sendToOtherPlayer( msgtrade, player );

		/// Commit has been accepted, swap items...
		swapSelected(tmpSession);

		///Set it back to pre-commit status.
		tmpSession->playerSelectedList.clear();
		tmpSession->targetSelectedList.clear();

		return true;
	}
	break;
	case MsgTrade::COMMIT_REJECT:
		///Reject commit - deselect items, and send other player msg.
	{
		if (tmpSession->getState() != MsgTrade::COMMIT)
			return true;
		else
			tmpSession->setState( MsgTrade::ACCEPT );

		///No other action is really necessary, besides send a message 
		///to the other player saying the trade has been rejected.
		MsgTrade msgtrade;
		msgtrade.type = MsgTrade::COMMIT_REJECT;

		LogDBG("Player '%s' rejecting commit", player);
		tmpSession->sendToOtherPlayer( msgtrade, player );

		///Set it back to pre-commit status.
		tmpSession->playerSelectedList.clear();
		tmpSession->targetSelectedList.clear();

		return true;
	}
	break;
	default:
		break;
	}

	return false;
}


bool SrvTradeMgr::removeTrade(const char* player)
{
	if (sessions.size() == 0)
	{
		return true;
	}

	bool tradeDeleted = false;

	SrvTradeSession* trade;
	std::vector<SrvTradeSession*>::iterator it;
	for (it = sessions.begin(); it != sessions.end(); ++it)
	{
		if ((*it)->player == player || (*it)->target == player)
		{
			trade = (*it);
			MsgTrade msgtrade;
			msgtrade.type = MsgTrade::END;
			trade->sendToOtherPlayer( msgtrade, player );
			sessions.erase(it);
			delete trade; trade = 0;
			tradeDeleted = true;
			if ( sessions.empty() )
				break;
		}
	}	
	return tradeDeleted;
}


void SrvTradeMgr::listTrades(void)
{
	if (sessions.size() == 0)
	{
		LogNTC("There are no trades");
		return;
	}

	std::vector<SrvTradeSession*>::iterator it;
	for (it = sessions.begin(); it != sessions.end(); ++it)
	{
		LogNTC("player: '%s', target: '%s'",
		       (*it)->player.c_str(),
		       (*it)->target.c_str());
	}
}


SrvTradeSession* SrvTradeMgr::findTrade(const char* player)
{
	if (sessions.size() == 0)
	{
		return 0;
	}

	std::vector<SrvTradeSession*>::iterator it;
	for (it = sessions.begin(); it != sessions.end(); ++it)
	{
		if ((*it)->player.compare(player) == 0 
		    || (*it)->target.compare(player) == 0)
		{
			LogDBG("Trade found.");
			return (*it);
		}
	}
	LogWRN("findTrade: Trade not found.");
	return 0;
}

void SrvTradeMgr::giveSelectedItem( SrvEntityPlayer* playerEntity,
									SrvEntityPlayer* targetEntity,
									int entityID )
{
	try {
		InventoryItem* item = playerEntity->getInventoryItem( entityID );
	
		LogNTC("player: '%s' gives item '%s' to target: '%s'",
		       playerEntity->getLoginData()->getPlayerName(),
	    	   item->getItemID(),
		       targetEntity->getLoginData()->getPlayerName() );

		if (!item) {
			LogERR("player: no item");
			throw "EntityID not found or not an object";
		} else if (!targetEntity->addToInventory(item)) {
			LogERR("target: cannot add item");
			throw "Cannot put the object into the inventory";
		} else if (!SrvWorldMgr::instance().changeObjectOwner(entityID, targetEntity->getName())) {
			LogERR("target: cannot add item - db");
			throw "Cannot save the object status to the DB";
		}
		///Now that it's in the other player's inventory, remove it...
		LogDBG("player '%s': remove from inv", playerEntity->getLoginData()->getPlayerName() );
		playerEntity->removeFromInventory( entityID );
		
		///Send message update
		MsgInventoryDel msgdel;
		msgdel.itemID = entityID;
		LogDBG("send msgdel to player: %s", playerEntity->getLoginData()->getPlayerName());
		SrvNetworkMgr::instance().sendToPlayer(msgdel, playerEntity->getLoginData());

		MsgInventoryAdd msgadded;
		msgadded.item = *targetEntity->getInventoryItem( entityID );
		LogDBG("send msgadd to target: %s", targetEntity->getLoginData()->getPlayerName() );
		SrvNetworkMgr::instance().sendToPlayer(msgadded, targetEntity->getLoginData());
	} catch ( const char * error ) {
		LogERR("swapping items failed: %s", error);
		///\todo: duffolonious: Send error to players
	}
}

void SrvTradeMgr::swapSelected( SrvTradeSession * trade )
{
	/** Swap items in inventories
	 * \todo: duffolonious: will probably require a dry run before
	 * actually swapping, because we can't have partial trades...
	 * so this needs updating.  
	 */
	LogNTC("*swapping selected items");
	LogDBG("player ('%s') has %zu items selected", trade->player.c_str(), trade->playerSelectedList.size() );
	LogDBG("target ('%s') has %zu items selected", trade->target.c_str(), trade->targetSelectedList.size() );

	SrvEntityPlayer* playerEntity = SrvLoginMgr::instance().findPlayer( trade->player.c_str() )->getPlayerEntity();
	SrvEntityPlayer* targetEntity = SrvLoginMgr::instance().findPlayer( trade->target.c_str() )->getPlayerEntity();
	if ( !playerEntity || !targetEntity )
		return;

	// Give players selected items to target
	/// Loop through selected.
	uint64_t entityID;
	std::vector<int>::iterator it;
	///Give player's selected items to target
	for ( it = trade->playerSelectedList.begin(); it!=trade->playerSelectedList.end(); ++it )
	{
		entityID = *it;
		LogDBG("Giving %llu to %s", entityID, trade->target.c_str() );
		giveSelectedItem( playerEntity, targetEntity, entityID );
	}
	// Give targets selected items to player
	for ( it = trade->targetSelectedList.begin(); it!=trade->targetSelectedList.end(); ++it )
	{
		entityID = *it;
		LogDBG("Giving %llu to %s", entityID, trade->player.c_str() );
		giveSelectedItem( targetEntity, playerEntity, entityID );
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
