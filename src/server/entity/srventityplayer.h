/*
 * srventityplayer.h
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

#ifndef __FEARANN_SERVER_ENTITY_PLAYER_H__
#define __FEARANN_SERVER_ENTITY_PLAYER_H__


#include "common/stats.h"
#include "common/net/msgs.h"
#include "common/patterns/observer.h"

#include "srventitybase.h"


class LoginData;
class PlayerInfo;
class SrvEntityObject;


/** Class to manage the player inventory in the server
 *
 * mafm: The CEL class for inventories is not appropriate, since we have to
 * create entities for the objects in the inventory. This is a waste of space
 * most of the time, since an entity needs a lot of properties (mesh, linmove,
 * ...), and the player may have tipically 10 to 50 objects in the inventory,
 * that means an extra of 1000 to 5000 entities in the engine with only 100
 * players, which probably exceeds the rest of the entities present: 100
 * players, a few dozens of NPCs, tenths to hundreds of objects... so it would
 * be much probably less than 1000 in total.
 *
 * @author mafm
 */
class SrvPlayerInventory
{
public:
	/** Add an item */
	bool addItem(InventoryItem& item);
	/** Remove an item, returning false if not found */
	bool removeItem(uint32_t itemID);
	/** Get an item */
	InventoryItem* getItem(uint32_t itemID);
	/** Get item by index */
	InventoryItem* getItemByIndex(size_t index);
	/** Get the number of objects */
	size_t getCount();
	/** Set the load that can be accepted */
	void setMaxLoad(float load);

	SrvPlayerInventory ():
		mLoadMax(0),mLoadCurrent(0) {}

private:
	/// The collection of objects
	std::vector<InventoryItem> mInventory;
	/// Load accepted (will refuse to add more items)
	float mLoadMax;
	/// Current load
	float mLoadCurrent;
};


/** Representation of a player in the server
 *
 * @author mafm
 */
class SrvEntityPlayer : public SrvEntityBase, public Observer
{
public:
	SrvEntityPlayer(MsgEntityCreate& basic,
			MsgEntityMove& mov,
			MsgPlayerData& data,
			LoginData* loginData);
	virtual ~SrvEntityPlayer();

	/** Get the login data */
	LoginData* getLoginData();
	/** Get the player info */
	PlayerInfo * getPlayerInfo();

	/** Update all the movement related stuff with the data provided by the
	 * client */
	void updateMovementFromClient(MsgEntityMove* msg);
	/** Send the info about movement to the player (the client normally
	 * doesn't receive the own position from the server, so it's because the
	 * position has been reseted or something similar) */
	void sendMovementToClient();

	/** Get the description of an item in the inventory */
	InventoryItem* getInventoryItem(uint32_t itemID);
	/** Add entity to the inventory, it returns false if there's some
	 * problem or the inventory is too loaded */
	bool addToInventory(SrvEntityObject* object);
	bool addToInventory(InventoryItem* item);
	/** Remove entity from the inventory */
	void removeFromInventory(uint32_t itemID);

	/** @see Observer::updateFromObservable */
        virtual void updateFromObservable(const ObserverEvent& event);

public:

	/// Get the amount of gold
	uint32_t GetGold();
	/// Add some amount of gold, result MUST be checked
	bool AddGold(int amount);
	/// Substract some amount of gold, result MUST be checked
	bool SubstractGold(int amount);

protected:
	/// Player data
	MsgPlayerData mPlayerData;
	/// Login data for this player
	LoginData* mLoginData;
	/// Inventory
	SrvPlayerInventory mInventory;
	/// Extra player info (class, alignment)
	PlayerInfo * mPlayerInfo;

	/** Virtual function overriden from base class, to treat the specifics
	 * of the player. */
	void saveToDB();
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
