/*
 * botinventory.h
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

#ifndef __FEARANN_BOT_ACTION_TRADEINVENTORY_H__
#define __FEARANN_BOT_ACTION_TRADEINVENTORY_H__


#include "common/patterns/singleton.h"
#include "common/net/msgs.h"

#include <vector>
#include <string>


class InventoryItem;


/** Class contains and manages objects
 */
class BotTradeInv : public Singleton<BotTradeInv>
{
public:
	std::vector<int> playerSelectedInventory;
	std::vector<int> targetSelectedInventory;


	/// Handle incoming trade message
	bool handleMsg(MsgTrade* msg);

	/// Add item to inventory - return false if can't add.
	bool AddItem(InventoryItem* item, bool target);

	/// Remove an item from the inventory (only on your side)
	bool RemoveItem(unsigned int itemID);

	/// Get an item from the inventory
	bool getItem(unsigned int itemID);

	/// Item in the inventory?
	bool hasItem(unsigned int itemID);

	/// Select item to be traded.
	void selectItem(unsigned int itemID);

	/// Is item selected
	bool isSelected( unsigned int itemID );

	/// List items in inventory
	void ListInventory();

	/// Trading partner stuff...
	std::string getTarget(void) { return target; };
	void setTarget(std::string _target) { target = _target; };

	///Get/Set state
	MsgTrade::MESSAGE_TYPE getState( void ) { return state; };
	void setState( MsgTrade::MESSAGE_TYPE _state ) { state = _state; };

	///Clean out from previous usage.
	void clear();

private:
	/** Singleton friend access */
	friend class Singleton<BotTradeInv>;


	/// The structure containing the entities in the inventory
	std::string target;
	std::vector<InventoryItem> playerInventory;
	std::vector<InventoryItem> targetInventory;

	MsgTrade::MESSAGE_TYPE state;

	~BotTradeInv();
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
