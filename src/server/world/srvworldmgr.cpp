/*
 * srvworldmgr.cpp
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

#include "srvworldmgr.h"

#include "common/net/msgs.h"
#include "common/tablemgr.h"

#include "server/action/srvcombatmgr.h"
#include "server/action/srvtrademgr.h"
#include "server/db/srvdbmgr.h"
#include "server/console/srvconsolemgr.h"
#include "server/entity/srventityobject.h"
#include "server/entity/srventitycreature.h"
#include "server/entity/srventityplayer.h"
#include "server/login/srvloginmgr.h"
#include "server/net/srvnetworkmgr.h"
#include "server/world/srvworldcontactmgr.h"
#include "server/world/srvworldtimemgr.h"

#include <iostream>
#include <string>
#include <list>
#include <algorithm>


/// Distance from player to objects to allow him/her to pick the objects up
const float PICKUP_DISTANCE = 10.0f;


/*******************************************************************************
 * SrvWorldMgr
 ******************************************************************************/
template <> SrvWorldMgr* Singleton<SrvWorldMgr>::INSTANCE = 0;

SrvWorldMgr::SrvWorldMgr()
{
}

void SrvWorldMgr::finalize()
{
	// clear players
	for (size_t i = 0; i < mPlayerList.size(); ++i) {
		removePlayer(mPlayerList[i]->getLoginData());
	}
	mPlayerList.clear();

	// clear creatures
	for (size_t i = 0; i < mCreatureList.size(); ++i) {
		removeEntity(mCreatureList[i]);
	}
	mCreatureList.clear();

	// clear objects
	for (size_t i = 0; i < mObjectList.size(); ++i) {
		removeEntity(mObjectList[i]);
	}
	mObjectList.clear();

	// clear areas
	mAreaList.clear();
}

bool SrvWorldMgr::isAreaLoaded(const std::string& name) const
{
	for (size_t i = 0; i < mAreaList.size(); ++i) {
		if (mAreaList[i] == name) {
			return true;
		}
	}
	return false;
}

bool SrvWorldMgr::loadArea(const std::string& name)
{
	if (isAreaLoaded(name)) {
		return false;
	} else {
		// area not loaded, we can go on
		mAreaList.push_back(name);
		return true;
	}
}

bool SrvWorldMgr::loadObjectsFromDB(const std::string& area)
{
	// STEPS
	// 1- get data from the db
	// 2- fill the data in the in-game structure

	// 1- get data from the db
	MsgEntityCreate msgBasic;
	MsgEntityMove msgMove;
	string id, pos1, pos2, pos3, rot;
	Vector3 position;

	SrvDBQuery query;
	query.setTables("entities");
	query.setCondition("owner ISNULL AND area='" + area + "'");
	query.setOrder("id");
	query.addColumnWithoutValue("id");
	query.addColumnWithoutValue("pos1");
	query.addColumnWithoutValue("pos2");
	query.addColumnWithoutValue("pos3");
	query.addColumnWithoutValue("rot");
	query.addColumnWithoutValue("type");
	query.addColumnWithoutValue("subtype");
	int numresults = SrvDBMgr::instance().querySelect(&query);
	if (numresults == 0) {
		LogWRN("There are no unowned entities in the DB in area '%s'",
		       area.c_str());
		return true;
	} else if (numresults < 0) {
		LogERR("Error loading entities from the DB");
		return false;
	}

	for (int row = 0; row < numresults; ++row) {
		query.getResult()->getValue(row, "id", id);
		query.getResult()->getValue(row, "pos1", pos1);
		query.getResult()->getValue(row, "pos2", pos2);
		query.getResult()->getValue(row, "pos3", pos3);
		query.getResult()->getValue(row, "rot", rot);
		query.getResult()->getValue(row, "type", msgBasic.meshType);
		query.getResult()->getValue(row, "subtype", msgBasic.meshSubtype);

		msgBasic.entityID = StrToUInt64(id);
		msgMove.area = area;
		msgMove.position = Vector3(atof(pos1.c_str()), atof(pos2.c_str()), atof(pos3.c_str()));
		msgMove.rot = atof(rot.c_str());
		msgBasic.entityClass = "Object";
		msgBasic.entityName = msgBasic.meshType;

		SrvEntityObject* object = new SrvEntityObject(msgBasic, msgMove);
		float load = TableMgr::instance().getTable("objects")->getValueAsInt(msgBasic.meshType, "load");
		object->setLoad(load);
		addEntity(object);
	}

	return true;
}

bool SrvWorldMgr::loadCreaturesFromDB(const std::string& area)
{
	// STEPS
	// 1- get data from the db
	// 2- fill the data in the in-game structure

	// 1- get data from the db
	MsgEntityCreate msgBasic;
	MsgEntityMove msgMove;
	MsgPlayerData msgPlayer;
	string id, pos1, pos2, pos3, rot;
	Vector3 position;

	SrvDBQuery query;
	query.setTables("creatures");
	query.setCondition("owner ISNULL AND area='" + area + "'");
	query.setOrder("id");
	query.addColumnWithoutValue("id");
	query.addColumnWithoutValue("pos1");
	query.addColumnWithoutValue("pos2");
	query.addColumnWithoutValue("pos3");
	query.addColumnWithoutValue("rot");
	query.addColumnWithoutValue("type");
	query.addColumnWithoutValue("subtype");
	int numresults = SrvDBMgr::instance().querySelect(&query);
	if (numresults == 0) {
		LogWRN("There are no creatures in the DB in area '%s'",
		       area.c_str());
		return true;
	} else if (numresults < 0) {
		LogERR("Error loading creatures from the DB");
		return false;
	}

	for (int row = 0; row < numresults; ++row) {
		query.getResult()->getValue(row, "id", id);
		query.getResult()->getValue(row, "pos1", pos1);
		query.getResult()->getValue(row, "pos2", pos2);
		query.getResult()->getValue(row, "pos3", pos3);
		query.getResult()->getValue(row, "rot", rot);
		query.getResult()->getValue(row, "type", msgBasic.meshType);
		query.getResult()->getValue(row, "subtype", msgBasic.meshSubtype);

		msgBasic.entityID = StrToUInt64(id);
		msgMove.area = area;
		msgMove.position = Vector3(atof(pos1.c_str()), atof(pos2.c_str()), atof(pos3.c_str()));
		msgMove.rot = atof(rot.c_str());
		msgBasic.entityClass = "Creature";
		msgBasic.entityName = msgBasic.meshType;

		msgPlayer.ab_con = TableMgr::instance().getTable("creatures")->getValueAsInt(msgBasic.meshType, "con");
		msgPlayer.ab_str = TableMgr::instance().getTable("creatures")->getValueAsInt(msgBasic.meshType, "str");
		msgPlayer.ab_dex = TableMgr::instance().getTable("creatures")->getValueAsInt(msgBasic.meshType, "dex");
		msgPlayer.ab_int = TableMgr::instance().getTable("creatures")->getValueAsInt(msgBasic.meshType, "int");
		msgPlayer.ab_wis = TableMgr::instance().getTable("creatures")->getValueAsInt(msgBasic.meshType, "wis");
		msgPlayer.ab_cha = TableMgr::instance().getTable("creatures")->getValueAsInt(msgBasic.meshType, "cha");

		std::string hd = TableMgr::instance().getTable("creatures")->getValue(msgBasic.meshType, "hd");
		msgPlayer.health_max = RollDie::instance().roll(hd);
		msgPlayer.health_cur = msgPlayer.health_max;

		SrvEntityCreature* creature = new SrvEntityCreature(msgBasic, msgMove, msgPlayer);
		addEntity(creature);
	}

	return true;
}

SrvEntityPlayer* SrvWorldMgr::findPlayer(const LoginData* loginData) const
{
	// mafm: We can use a hash for the player list, so the performance
	// should be better (O(1) instead of O(n)).  Since STL doesn't seem to
	// have hash functions as standard, and maps/set are logaritmic but you
	// don't seem to be able to remove a key after inserted (which gives a
	// lot of problems and additional test after player logged off), we'll
	// use plain vectors for now.

	for (size_t i = 0; i < mPlayerList.size(); ++i) {
		if (loginData->getPlayerEntity() == mPlayerList[i]) {
			return mPlayerList[i];
		}
	}
	// not found
	return 0;
}

SrvEntityCreature* SrvWorldMgr::findCreature(EntityID entityID) const
{
	// mafm: Read the comment for players about performance.
	for (size_t i = 0; i < mCreatureList.size(); ++i) {
		if (entityID == mCreatureList[i]->getID()) {
			return mCreatureList[i];
		}
	}
	// not found
	return 0;
}

SrvEntityObject* SrvWorldMgr::findObject(EntityID entityID) const
{
	// mafm: Read the comment for players about performance.
	for (size_t i = 0; i < mObjectList.size(); ++i) {
		if (entityID == mObjectList[i]->getID()) {
			return mObjectList[i];
		}
	}
	// not found
	return 0;
}

void SrvWorldMgr::addPlayer(LoginData* loginData)
{
	LogNTC("Adding player '%s' to world manager",
	       loginData->getPlayerName());

	SrvEntityPlayer* player = loginData->getPlayerEntity();
	loginData->setPlaying(true);

	// mafm: just subscribing everybody, do something more elaborate in the
	// future if needed
	for (size_t i = 0; i < mPlayerList.size(); ++i) {
		// mafm: we can do a small optimization avoiding this
		// comparison, but only if we're sure that player is always
		// added after being subscribed (to do the comparison elsewhere,
		// in example in the subscription class, doesn't help)
		if (player == mPlayerList[i]) {
			LogWRN("Player '%s' added to the player list before being subscribed",
			       player->getName());
			continue;
		}
		mPlayerList[i]->attachObserver(player);
		player->attachObserver(mPlayerList[i]);
	}

	// notifying client about game time, to set up environment lights and
	// whatever it might need
	SrvWorldTimeMgr::instance().sendTimeToPlayer(loginData);

	// subscribing to entities 
	for (size_t i = 0; i < mCreatureList.size(); ++i) {
		mCreatureList[i]->attachObserver(player);
	}
	for (size_t i = 0; i < mObjectList.size(); ++i) {
		mObjectList[i]->attachObserver(player);
	}

	// contact notification
	SrvWorldContactMgr::playerStatusChange(loginData, true);

	// now after subscribing, add to the list
	mPlayerList.push_back(player);

	// broadcasting message
	MsgChat msg;
	msg.origin = "Server";
	msg.type = MsgChat::SYSTEM;
	msg.text = StrFmt("'%s' joining the game (in '%s')",
			  player->getName(), player->getArea());
	SrvNetworkMgr::instance().sendToAllButPlayer(msg, loginData);
	LogNTC("Player joined the game: '%s' (entity id: %llu)",
	       player->getName(), player->getID());
}

void SrvWorldMgr::removePlayer(LoginData* loginData)
{
	LogNTC("Removing player '%s' from world manager",
	       loginData->getPlayerName());

	SrvEntityPlayer* player = loginData->getPlayerEntity();
	loginData->setPlaying(false);

	// remove from the list -- unsubscription works automatically from other
	// players and creatures
	vector<SrvEntityPlayer*>::iterator itFound = mPlayerList.end();
	for (vector<SrvEntityPlayer*>::iterator it = mPlayerList.begin();
	     it != mPlayerList.end(); ++it) {
		if (*it == player) {
			itFound = it;
		}
	}
	if (itFound == mPlayerList.end()) {
		LogERR("Player '%s' not found in list while trying to remove it",
		       player->getName());
		return;
	} else {
		mPlayerList.erase(itFound);
	}

	// contact status
	SrvWorldContactMgr::playerStatusChange(loginData, false);

	// combat status
	SrvCombatMgr::instance().removePlayerFromBattle(loginData);

	// end all trades with this player
	SrvTradeMgr::instance().removeTrade(loginData->getPlayerName());

	// broadcasting message
	MsgChat msg;
	msg.origin = "Server";
	msg.type = MsgChat::SYSTEM;
	msg.text = StrFmt("'%s' leaving the game (in '%s')",
			  player->getName(), player->getArea());
	SrvNetworkMgr::instance().sendToAllButPlayer(msg, loginData);

	// removing entity
	delete player;
}

void SrvWorldMgr::getNearbyPlayers(const LoginData* player,
				   float radius,
				   vector<LoginData*>& nearbyPlayers) const
{
	for (size_t i = 0; i < mPlayerList.size(); ++i) {
		float distance = player->getPlayerEntity()->getDistanceToEntity(*mPlayerList[i]);
		if (distance <= radius) {
			nearbyPlayers.push_back(mPlayerList[i]->getLoginData());
		}
	}
}

void SrvWorldMgr::addEntity(SrvEntityBase* entity)
{
	LogNTC("Adding entity '%llu' to world manager, class '%s'",
	       entity->getID(), entity->getEntityClass());

	// mafm: just subscribing everybody, do something more elaborate in the
	// future if needed
	for (size_t i = 0; i < mPlayerList.size(); ++i) {
		entity->attachObserver(mPlayerList[i]);
	}

	// add to the list
	if (SrvEntityCreature* c = dynamic_cast<SrvEntityCreature*>(entity)) {
		mCreatureList.push_back(c);
	} else if (SrvEntityObject* o = dynamic_cast<SrvEntityObject*>(entity)) {
		mObjectList.push_back(o);
	} else {
		LogERR("Class '%s' is unknown, for entity '%llu'",
		       entity->getEntityClass(), entity->getID());
	}
}

void SrvWorldMgr::removeEntity(SrvEntityBase* entity)
{
	LogNTC("Removing entity '%llu' from world manager, class '%s'",
	       entity->getID(), entity->getEntityClass());

	// subscribers: the entity already informs the subscribers in the
	// destructor

	// removing entity from our list
	if (dynamic_cast<SrvEntityCreature*>(entity)) {
		mCreatureList.erase(std::remove(mCreatureList.begin(), mCreatureList.end(), entity),
				    mCreatureList.end());
	} else if (dynamic_cast<SrvEntityObject*>(entity)) {
		mObjectList.erase(std::remove(mObjectList.begin(), mObjectList.end(), entity),
				    mObjectList.end());
	} else {
		LogERR("Class '%s' is unknown, for entity '%llu'",
		       entity->getEntityClass(), entity->getID());
	}

	// delete, at last
	delete entity;
}

void SrvWorldMgr::playerGetItem(SrvEntityPlayer* player, EntityID entityID)
{
	// mafm: We destroy the structure representing the item when it's added
	// to the inventory.  We want to do this instead of having all the
	// objects loaded as entities in the engine, because in this way we save
	// a lot of memory and probably overhead in the server (when the engine
	// has to check all the entities present for whatever purposes),
	// although this depends on the implementation.  So the data objects
	// instantiated as SrvEntityObject or similar represent an object in the
	// world not owned by anybody: the "free" objects standing out there in
	// the world.

	try {
		// check if the entity exists and it's an object, and can be
		// added
		SrvEntityObject* object = findObject(entityID);
		if (!object) {
			throw "EntityID not found or not an object";
		} else if (player->getDistanceToEntity(*object) > PICKUP_DISTANCE) {
			throw "Object too far away";
		} else if (!player->addToInventory(object)) {
			throw "Cannot put the object into the inventory";
		} else if (!getObjectToInventory(entityID, player->getName())) {
			throw "Cannot save the object status to the DB";
		}

		// everything ok, send notifications
		string text = StrFmt("'%s' getting item (name: '%s', id %llu)",
				     player->getName(),
				     object->getName(), object->getID());
		LogDBG("%s", text.c_str());
		MsgChat msgInfo;
		msgInfo.origin = "Server";
		msgInfo.type = MsgChat::ACTION;
		msgInfo.text = text;
		SrvNetworkMgr::instance().sendToAllPlayers(msgInfo);

		MsgInventoryAdd msgAdded;
		msgAdded.item = InventoryItem(object->getID(),
					      object->getType(),
					      object->getSubtype(),
					      object->getLoad());
		SrvNetworkMgr::instance().sendToPlayer(msgAdded,
						       player->getLoginData());

		// finally, destroy the object entity/behaviour
		removeEntity(object);

	} catch (const char* error) {
		MsgChat msg;
		msg.origin = "Server";
		msg.type = MsgChat::ACTION;
		msg.text = StrFmt("Cannot get item: '%s'", error);
		SrvNetworkMgr::instance().sendToPlayer(msg,
						       player->getLoginData());

		LogDBG("Player '%s' attempted to get an item but failed: '%s'",
		       player->getName(), error);
	}
}

void SrvWorldMgr::playerDropItem(SrvEntityPlayer* player, EntityID entityID)
{
	// mafm: read the comment of GetItem

	try {
		// check if the entity belongs to the player and make a local
		// copy before removing
		InventoryItem invItem;
		InventoryItem* found = player->getInventoryItem(entityID);
		if (!found) {
			throw "Cannot find the object in the inventory";
		} else {
			invItem = *found;
			player->removeFromInventory(entityID);
		}

		// create the object into the world again
		Vector3 position;
		player->getPositionWithRelativeOffset(position, Vector3(0, 1, -1));
		if (!dropObjectFromInventory(entityID,
					     player->getArea(),
					     position)) {
			throw "Cannot create the entity";
		}

		// everything ok, send notifications
		string text = StrFmt("'%s' dropping item (type: '%s', id %llu)",
				     player->getName(),
				     invItem.getType(),
				     entityID);
		LogDBG("%s", text.c_str());
		MsgChat msgInfo;
		msgInfo.origin = "Server";
		msgInfo.type = MsgChat::ACTION;
		msgInfo.text = text;
		SrvNetworkMgr::instance().sendToAllPlayers(msgInfo);

		MsgInventoryDel msgRemoved;
		msgRemoved.itemID = entityID;
		SrvNetworkMgr::instance().sendToPlayer(msgRemoved,
						       player->getLoginData());

	} catch (const char* error) {
		MsgChat msg;
		msg.origin = "Server";
		msg.type = MsgChat::ACTION;
		msg.text = StrFmt("Cannot get item: %s", error);
		SrvNetworkMgr::instance().sendToPlayer(msg, player->getLoginData());

		LogDBG("Player '%s' attempted to drop an item but failed: %s",
		       player->getName(), error);
	}
}

bool SrvWorldMgr::changeObjectOwner(EntityID entityID, const std::string& charname)
{
	string id = StrFmt("%llu", entityID);

	SrvDBQuery query;
	query.setTables("entities");
	query.setCondition("id='" + id + "'");
	query.addColumnWithValue("owner", charname);
	bool success = SrvDBMgr::instance().queryUpdate(&query);
	if (!success) {
		LogERR("Error setting object '%llu' ownership to player '%s' in the DB",
		       entityID, charname.c_str());
		return false;
	}

	return true;
}

bool SrvWorldMgr::getObjectToInventory(EntityID entityID, const std::string& charname)
{
	// mafm: same implementation as changing the owner of the object
	return changeObjectOwner(entityID, charname);
}

bool SrvWorldMgr::dropObjectFromInventory(EntityID entityID,
					  const::string& area,
					  const Vector3& position)
{
	// STEPS
	// 1- get data from the db
	// 2- fill the data in the in-game structure
	// 3- mark as removed from inventory, and update area in the
	//    DB (might have changed)


	// 1- get data from the db
	MsgEntityCreate msgBasic;
	MsgEntityMove msgMove;
	string id = StrFmt("%llu", entityID);

	SrvDBQuery query;
	query.setTables("entities");
	query.setCondition("id='" + id + "'");
	query.addColumnWithoutValue("type");
	query.addColumnWithoutValue("subtype");
	int numresults = SrvDBMgr::instance().querySelect(&query);
	if (numresults != 1) {
		LogERR("Error loading entity from the DB");
		return false;
	}

	query.getResult()->getValue(0, "type", msgBasic.meshType);
	query.getResult()->getValue(0, "subtype", msgBasic.meshSubtype);

	// 2- fill the data in the in-game structure
	msgBasic.entityID = entityID;
	msgBasic.entityClass = "Object";
	msgBasic.entityName = msgBasic.meshType;
	msgMove.area = area;
	msgMove.position = position;
	SrvEntityObject* object = new SrvEntityObject(msgBasic, msgMove);
	float load = TableMgr::instance().getTable("objects")->getValueAsInt(msgBasic.meshType, "load");
	object->setLoad(load);
	addEntity(object);

	// 3- mark as removed from inventory
	SrvDBQuery query2;
	query2.setTables("entities");
	query2.setCondition("id='" + id + "'");
	query2.addColumnWithValue("owner", "NULL", false);
	query2.addColumnWithValue("area", area, false);
	bool success = SrvDBMgr::instance().queryUpdate(&query2);
	if (!success) {
		LogERR("Error marking the object as an outside object");
		return false;
	}

	return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
