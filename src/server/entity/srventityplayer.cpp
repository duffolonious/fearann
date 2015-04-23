/*
 * srventityplayer.cpp
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

#include "srventityplayer.h"

#include "server/db/srvdbmgr.h"
#include "server/login/srvloginmgr.h"
#include "server/net/srvnetworkmgr.h"
#include "srventityobject.h"



//--------------------- SrvPlayerInventory ---------------------------
bool SrvPlayerInventory::addItem(InventoryItem& item)
{
	LogDBG("Load - cur: '%.1f' item: '%.1f' max: '%.1f'", 
		mLoadCurrent, item.getLoad(), mLoadMax);
	if (mLoadCurrent + item.getLoad() > mLoadMax) {
		return false;
	} else {
		mLoadCurrent += item.getLoad();
		mInventory.push_back(item);
		return true;
	}
}

bool SrvPlayerInventory::removeItem(uint32_t itemID)
{
	for (vector<InventoryItem>::iterator it = mInventory.begin();
	     it != mInventory.end(); ++it) {
		if (itemID == unsigned(atoi(it->getItemID()))) {
			LogDBG("Removed item from inventory vector: %s '%s' '%s' %g",
			       it->getItemID(), it->getType(),
			       it->getSubtype(), it->getLoad());
			mLoadCurrent -= it->getLoad();
			mInventory.erase(it);
			return true;
		}
	}
	return false;
}

InventoryItem* SrvPlayerInventory::getItem(uint32_t itemID)
{
	for (size_t i = 0; i < mInventory.size(); ++i) {
		InventoryItem* item = &(mInventory[i]);
		if (itemID == unsigned(atoi(item->getItemID()))) {
			LogDBG("Found item: %s '%s' '%s' %g",
			       item->getItemID(), item->getType(),
			       item->getSubtype(), item->getLoad());
			return item;
		}
	}
	LogDBG("Cannot get item from inventory: not found");
	return 0;
}

InventoryItem* SrvPlayerInventory::getItemByIndex(size_t index)
{
	if (index >= mInventory.size()) {
		LogDBG("Cannot get item from inventory: index out of bounds");
		return 0;
	} else {
		return &(mInventory[index]);
	}
}

size_t SrvPlayerInventory::getCount()
{
	return mInventory.size();
}

void SrvPlayerInventory::setMaxLoad(float load)
{
	mLoadMax = load;
}


//--------------------------- SrvEntityPlayer --------------------------
SrvEntityPlayer::SrvEntityPlayer(MsgEntityCreate& basic,
				     MsgEntityMove& mov,
				     MsgPlayerData& data,
				     LoginData* loginData) :
	SrvEntityBase(basic, mov),
	Observer(basic.entityName),
	mPlayerData(data),
	mLoginData(loginData)
{
	mLoginData->setPlaying(true);
	mLoginData->setPlayerEntity(this);

	// self -- needs to send it after the area is created but before the
	// reply message
	MsgEntityCreate msgself(mBasic);
	msgself.entityClass = "MainPlayer";
	SrvNetworkMgr::instance().sendToPlayer(msgself,  mLoginData);

	// join reply message
	LogDBG("Sending join reply message");
	MsgJoinReply joinreply;
	joinreply.resultCode = MsgUtils::Errors::SUCCESS;
	SrvNetworkMgr::instance().sendToPlayer(joinreply, mLoginData);

	// player data message
	MsgPlayerData msgPD(mPlayerData);
	SrvNetworkMgr::instance().sendToPlayer(msgPD, mLoginData);

	// inventory
	string itemID, itemType, itemSubtype;
	{
		// Set the max load given from player data
		mInventory.setMaxLoad( msgPD.load_max );

		SrvDBQuery query;
		query.setTables("entities");
		string condition = "owner='" + mBasic.entityName + "'";
		query.setCondition(condition);
		query.addColumnWithoutValue("id");
		query.addColumnWithoutValue("type");
		query.addColumnWithoutValue("subtype");
		int numresults = SrvDBMgr::instance().querySelect(&query);
		for (int row = 0; row < numresults; ++row) {
			itemID = "0";
			itemType = "<none>";
			itemSubtype = "<none>";
			query.getResult()->getValue(row, "id", itemID);
			query.getResult()->getValue(row, "type", itemType);
			query.getResult()->getValue(row, "subtype", itemSubtype);

			/// \todo mafm: adapt
			/*
			  ocInfoBase* itemInfo = Server->getLoader()->getInfoItem(itemType);
			  PERM_ASSERT(itemInfo);
			  float load = static_cast<ocInfoEquipable*>(itemInfo)->getLoad();
			*/
			float load = 1.0f;
			InventoryItem item(itemID, itemType, itemSubtype, load);
			mInventory.addItem(item);
		}
	}
	MsgInventoryListing invmsg;
	for (size_t i = 0; i < mInventory.getCount(); ++i) {
		InventoryItem* item = mInventory.getItemByIndex(i);
		invmsg.addItem(item);
	}

	/// \todo: duffolonious - load player info from db.
	string race, gender, playerClass, level, alignment;
	{
		SrvDBQuery query;
		query.setTables("usr_chars");
		string condition = "charname='" + mBasic.entityName + "'";
		query.setCondition(condition);
		query.addColumnWithoutValue("race");
		query.addColumnWithoutValue("gender");
		query.addColumnWithoutValue("class");
		//query.addColumnWithoutValue("alignment");
		int numresults = SrvDBMgr::instance().querySelect(&query);
		if (numresults != 1) {
			LogERR("SrvEntityPlayer: more than one character selected.");
		}

		int row = 0;

		race = "<none>";
		gender = "?";
		playerClass = "<none>";
		level = "1";
		alignment = "1";
		query.getResult()->getValue(row, "race", race);
		query.getResult()->getValue(row, "gender", gender);
		query.getResult()->getValue(row, "class", playerClass);
		//query.getResult()->getValue(row, "alignment", alignment);
		mPlayerInfo = new PlayerInfo( gender,
				(PlayerInfo::PLAYER_RACE)atoi( race.c_str() ), //assuming integer
				playerClass,
				(PlayerInfo::PLAYER_ALIGNMENT)atoi( alignment.c_str() ),
				mPlayerData.level );

		// Set attributes
		mPlayerInfo->setProperty( "strength", mPlayerData.ab_str );
		mPlayerInfo->setProperty( "constitution", mPlayerData.ab_con );
		mPlayerInfo->setProperty( "dexterity", mPlayerData.ab_dex );
		mPlayerInfo->setProperty( "wisdom", mPlayerData.ab_wis );
		mPlayerInfo->setProperty( "intelligence", mPlayerData.ab_int );
		mPlayerInfo->setProperty( "charisma", mPlayerData.ab_cha );

		// Set health
		mPlayerInfo->setHealth( mPlayerData.health_cur );
	}

	SrvNetworkMgr::instance().sendToPlayer(invmsg, mLoginData);
}

SrvEntityPlayer::~SrvEntityPlayer()
{
	saveToDB();
	delete mPlayerInfo; mPlayerInfo = 0;
}

void SrvEntityPlayer::saveToDB()
{
	// mafm: sometimes works bad with the "exact" data, +=0.5 for pos3=z
	// (height)
	string charname = mBasic.entityName;
	string area = mMov.area;
	string pos1 = StrFmt("%.1f", mMov.position.x);
	string pos2 = StrFmt("%.1f", mMov.position.y);
	string pos3 = StrFmt("%.1f", mMov.position.z + 0.5f);
	string rot = StrFmt("%.3f", mMov.rot);

	SrvDBQuery query;
	query.setTables("usr_chars");
	query.setCondition("charname='" + charname + "'");
	query.addColumnWithValue("area", area);
	query.addColumnWithValue("pos1", pos1);
	query.addColumnWithValue("pos2", pos2);
	query.addColumnWithValue("pos3", pos3);
	query.addColumnWithValue("rot", rot);
	bool success = SrvDBMgr::instance().queryUpdate(&query);
	if (!success) {
		LogERR("Error while saving position for player '%s'",
		       charname.c_str());
		return;
	}

	// player stats
	string health = StrFmt("%d", mPlayerData.health_cur);
	string magic = StrFmt("%d", mPlayerData.magic_cur);
	string stamina = StrFmt("%d", mPlayerData.stamina);
	string gold = StrFmt("%d", mPlayerData.gold);

	SrvDBQuery query2;
	query2.setTables("player_stats");
	query2.setCondition("charname='" + charname + "'");
	query2.addColumnWithValue("health", health);
	query2.addColumnWithValue("magic", magic);
	query2.addColumnWithValue("stamina", stamina);
	query2.addColumnWithValue("gold", gold);
	success = SrvDBMgr::instance().queryUpdate(&query2);
	if (!success) {
		LogERR("Error while saving stats for character '%s'",
		       charname.c_str());
		return;
	}

	LogDBG("Saving position for player '%s' in DB: '%s' (%s, %s, %s) rot=%s"
	       " and data: H=%s M=%s S=%s G=%s",
	       charname.c_str(), mMov.area.c_str(),
	       pos1.c_str(), pos2.c_str(), pos3.c_str(), rot.c_str(),
	       health.c_str(), magic.c_str(), stamina.c_str(), gold.c_str());
}

void SrvEntityPlayer::updateMovementFromClient(MsgEntityMove* msg)
{
	/** \todo mafm: try to implement a more robust system in combination
	 * with the client, in example using the updates when not moving to
	 * check that there were no changes, and not distribute that message to
	 * the other players.
	 *
	 * With the help of this scheme may be possible to do further checks,
	 * even if not very accurate, to check that players aren't cheating
	 * much */


	// the client (in the current implementation) is not aware of its ID,
	// and we can't trust it anyway
	msg->entityID = mBasic.entityID;

	// we don't trust in the area either, and at the moment we don't have
	// more than one anyway...
	msg->area = mBasic.area;

	// "adopt" the message, and notify the subscribers
	mMov = *msg;
	SrvEntityBaseObserverEvent event(SrvEntityBaseObserverEvent::ENTITY_CREATE, *msg);
	notifyObservers(event);

	LogDBG("Updating position: '%s' id=%lu, pos (%.1f, %.1f, %.1f) rot=%.1f"
	       " RUN=%d FW=%d BW=%d RL=%d RR=%d",
	       getName(), mMov.entityID,
	       mMov.position.x, mMov.position.y, mMov.position.z, mMov.rot,
	       mMov.run, mMov.mov_fwd, mMov.mov_bwd, mMov.rot_left, mMov.rot_right);
}

void SrvEntityPlayer::sendMovementToClient()
{
	SrvNetworkMgr::instance().sendToPlayer(mMov, mLoginData);
}

LoginData* SrvEntityPlayer::getLoginData()
{
	return mLoginData;
}

PlayerInfo * SrvEntityPlayer::getPlayerInfo()
{
	return mPlayerInfo;
};

InventoryItem* SrvEntityPlayer::getInventoryItem(uint32_t itemID)
{
	return mInventory.getItem(itemID);
}

bool SrvEntityPlayer::addToInventory(SrvEntityObject* object)
{
	// add the inventory in the runtime structure and the DB, update load
	InventoryItem item(object->getID(),
			     object->getType(),
			     object->getSubtype(),
			     object->getLoad());
	return addToInventory(&item);
}

bool SrvEntityPlayer::addToInventory(InventoryItem* item)
{
	// add the inventory in the runtime structure and the DB, update load
	bool success = mInventory.addItem(*item);
	if (!success) {
		LogNTC("Player %s can't add item to inventory because of load: %g+%g > %g",
		       getName(),
		       float(mPlayerData.load_cur),
		       item->getLoad(),
		       float(mPlayerData.load_max));
		return false;
	} else {
		mPlayerData.load_cur += static_cast<uint16_t>(item->getLoad());
	}

	// send a message with updated data (at least load changed)
	MsgPlayerData msg(mPlayerData);
	SrvNetworkMgr::instance().sendToPlayer(msg, getLoginData());

	return success;
}

void SrvEntityPlayer::removeFromInventory(uint32_t itemID)
{
	// remove the item from the inventory and update the load
	InventoryItem item = *(mInventory.getItem(itemID));
	mInventory.removeItem(itemID);
	mPlayerData.load_cur -= static_cast<uint16_t>(item.getLoad());

	// send a message with updated data (at least load changed)
	MsgPlayerData msg(mPlayerData);
	SrvNetworkMgr::instance().sendToPlayer(msg, getLoginData());
}

/** \todo mafm: do something with this


int ocPlayerBehaviour::GetGold()
{
	return PlayerData.gold;
}

bool ocPlayerBehaviour::SetGold(int variation)
{
	LogDBG("Player %s updating gold: %d + %d = %d",
	       GetPlayerName(),
	       PlayerData.gold, variation, PlayerData.gold + variation);
	PlayerData.gold += variation;

	return ocEntityStorage::SaveGold(this);
}

bool ocPlayerBehaviour::AddGold(int amount)
{
	if (amount <= 0)
	{
		LogWRN("Refusing to add %d gold to player %s",
		       amount, GetPlayerName());
		return false;
	}
	else
		return SetGold(amount);
}

bool ocPlayerBehaviour::SubstractGold(int amount)
{
	if (amount <= 0)
	{
		LogWRN("Refusing to substract %d gold from player %s",
		       amount, GetPlayerName());
		return false;
	}
	else if (GetGold() - amount < 0)
	{
		LogWRN("Cannot substract %d gold from player %s (has %d in total)",
		       amount, GetPlayerName(), GetGold());
		return false;
	}
	else
	{
		// notice that we convert the amount to a variation
		// related with the current gold
		return SetGold(-amount);
	}
}

*/

void SrvEntityPlayer::updateFromObservable(const ObserverEvent& event)
{
	try {
		// SrvEntityBase events
		{
			const SrvEntityBaseObserverEvent* e =
				dynamic_cast<const SrvEntityBaseObserverEvent*>(&event);
			if (e) {
				switch (e->_actionId) {
				case SrvEntityBaseObserverEvent::ENTITY_CREATE:
				case SrvEntityBaseObserverEvent::ENTITY_DESTROY:
					SrvNetworkMgr::instance().sendToPlayer(e->_msg,
									       mLoginData);
					break;
				default:
					throw "Action not understood by Observer";
				}
				return;
			}
		}

		// event not processed before
		throw "Event type not expected by Observer";
	} catch (const char* error) {
		LogWRN("SrvEntityPlayer: '%s' event: %s", event._className.c_str(), error);
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
