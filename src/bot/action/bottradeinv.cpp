/*
 * bottradeinv.cpp
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

#include "bottradeinv.h"

#include <cstdlib>

//----------------------- BotTradeInv ----------------------------
template <> BotTradeInv* Singleton<BotTradeInv>::INSTANCE = 0;

BotTradeInv::~BotTradeInv()
{
	clear();
}


void BotTradeInv::clear()
{
    std::vector<InventoryItem>::iterator it;
	for (it = playerInventory.begin();
         it != playerInventory.end(); it++)
    {
        //playerInventory.erase(it);
    }
	playerInventory.clear();

	for (it = targetInventory.begin();
         it != targetInventory.end(); it++)
    {
        //targetInventory.erase(it);
    }
	targetInventory.clear();

	playerSelectedInventory.clear();
	targetSelectedInventory.clear();
}

bool BotTradeInv::handleMsg(MsgTrade* msg)
{
	MsgTrade::MESSAGE_TYPE type = msg->getState();
	std::vector<InventoryItem>::iterator it;

	switch(msg->type)
    {
        case MsgTrade::START:
			/// other player has initiated trade.
			/// need to accept or end.
			target = msg->target;
			LogNTC("Will you accept a trade from '%s'?",
                                                        msg->target.c_str());
			return true;
        case MsgTrade::END:
			state = type;
			///Clear all lists.
			target = "";
			for (it = playerInventory.begin();
		         it != playerInventory.end(); it++)
			{
		        playerInventory.erase(it);
			}
			playerInventory.clear();

			for (it = targetInventory.begin();
		         it != targetInventory.end(); it++)
			{
		        targetInventory.erase(it);
			}
			targetInventory.clear();
    		playerSelectedInventory.clear();
    		targetSelectedInventory.clear();
			return true;
        case MsgTrade::ACCEPT:
			state = type;
			LogNTC("Trade accepted from '%s'.", target.c_str());
			return true;
        case MsgTrade::UPDATE_LIST:
			LogDBG("Received update list msg.");
			if ( state != MsgTrade::ACCEPT )
				return true;
			LogDBG("Clear target's inventory.");
			/// Update other players list
			for (it = targetInventory.begin();
			     it != targetInventory.end(); it++)
			{
		        targetInventory.erase(it);
			}
			targetInventory.clear();
			LogDBG("Assign to targets inventory, itemList size: %d.", 
														msg->itemList.size());
			targetInventory.assign( msg->itemList.begin(), msg->itemList.end() );
			return true;
        case MsgTrade::COMMIT:
			state = type;
			/// assign selected vectors
    		playerSelectedInventory.clear();
    		targetSelectedInventory.clear();
    		playerSelectedInventory.assign( msg->playerSelectedList.begin(),
											msg->playerSelectedList.end());
    		targetSelectedInventory.assign( msg->targetSelectedList.begin(),
                                            msg->targetSelectedList.end());

			LogNTC("Trade commit received - to '/trade status' "
								"to see selected, then accept.");
			return true;
        case MsgTrade::COMMIT_ACCEPT:
			state = MsgTrade::ACCEPT;
			///Receive add/del messages after this message...
			return true;
        case MsgTrade::COMMIT_REJECT:
			state = MsgTrade::ACCEPT;
			return true;
        default:
            return false;
    }
}


bool BotTradeInv::AddItem(InventoryItem* item, bool _target = false)
{
    PERM_ASSERT (item);

    LogDBG("Adding entity ('%s', '%s', '%s', '%f') to the inventory",
           item->getItemID(), item->getType(),
           item->getSubtype(), item->getLoad());

    // adding this entity to the inventory list
    unsigned int itemID = unsigned(atoi(item->getItemID()));
    InventoryItem elem(itemID,
						item->getType(),
						item->getSubtype(),
						item->getLoad() );
	if ( _target )
	    targetInventory.push_back(elem);
	else
	    playerInventory.push_back(elem);

	return true;
}


bool BotTradeInv::RemoveItem(unsigned int itemID)
{
	LogDBG("Removing entity %u from the inventory",
           itemID);

    if (playerInventory.size() <= 0 ||
    	targetInventory.size() <= 0)
    {
        LogERR("Inventories are empty, cannot remove anything");
        return false;
    }

	// removing this entity from the inventory list
    vector<InventoryItem>::iterator it;
    for (it = playerInventory.begin(); it != playerInventory.end(); it++)
    {
        if (itemID == (unsigned int)atoi((*it).getItemID()))
        {
            playerInventory.erase(it);
            return true;
        }
    }

    for (it = targetInventory.begin(); it != targetInventory.end(); it++)
    {
        if (itemID == (unsigned int)atoi((*it).getItemID()))
        {
            targetInventory.erase(it);
            return true;
        }
    }

    LogERR("Entity not found in inventory when trying to remove it");
	return false;
}


bool BotTradeInv::hasItem(unsigned int itemID)
{
    //LogDBG("Checking if entity %u is in the inventory",
    //       itemID);

    if (playerInventory.size() <= 0 ||
    	targetInventory.size() <= 0)
    {
        LogERR("Inventory is empty, cannot get anything");
        return false;
    }

    // removing this entity from the inventory list
    vector<InventoryItem>::iterator it;
    for (it = playerInventory.begin(); it != playerInventory.end(); it++)
    {
        if (itemID == (unsigned int)atoi((*it).getItemID()))
        {
            return true;
        }
    }

    for (it = targetInventory.begin(); it != targetInventory.end(); it++)
    {
        if (itemID == (unsigned int)atoi((*it).getItemID()))
        {
            return true;
        }
    }

    LogERR("Entity not in inventory");
    return false;
}


void BotTradeInv::ListInventory()
{
    std::vector<InventoryItem>::iterator it;

    if (playerInventory.size() <= 0 &&
    	targetInventory.size() <= 0)
	{
		LogNTC("Inventories empty");
		return;
	}

	const char * selected;
	int count = 0;
	LogNTC("Player inventory:");
    for (it = playerInventory.begin(); it != playerInventory.end(); it++)
	{
		count++;
		if (isSelected( (unsigned int) atoi( (*it).getItemID() ) ))
		{
			//LogDBG("list inv - player item '%s' is selected", (*it).getItemID() );
			selected = "*";
		} else
		{
			selected = "";
		}

		LogNTC("%s%d - id: %s type: %s sub: %s load: %f",
								selected,
								count,
								(*it).getItemID(),
								(*it).getType(),
								(*it).getSubtype(),
								(*it).getLoad());
	}
	count = 0;
	LogNTC("Target (%s) inventory:", target.c_str());
    for (it = targetInventory.begin(); it != targetInventory.end(); it++)
	{
		count++;
		if (isSelected( (unsigned int) atoi( (*it).getItemID() ) ))
		{
			selected = "*";
		} else
		{
			selected = "";
		}

		LogNTC("%s%d - id: %s type: %s sub: %s load: %f",
								selected,
								count,
								(*it).getItemID(),
								(*it).getType(),
								(*it).getSubtype(),
								(*it).getLoad());
	}
	LogNTC("NOTE: '*' - denote selected entries");

	return;
}


void BotTradeInv::selectItem(unsigned int itemID)
{
    LogDBG("Selecting entity %u in the inventory...",
           itemID);

	if (hasItem( itemID ) )
	{
	    //LogDBG(" - Entity %u in an inventory", itemID);
		//Select item in relevant item.
		vector<InventoryItem>::iterator it;
		for (it = playerInventory.begin(); it != playerInventory.end(); it++)
		{
			//LogDBG("iterating: entity %u, player inv: %s", itemID,
			//											 (*it).getItemID());
			if (itemID == (unsigned int)atoi((*it).getItemID()))
			{
			    LogDBG("Selected entity %u", itemID);
				playerSelectedInventory.push_back( (int)itemID );
			}
		}

		for (it = targetInventory.begin(); it != targetInventory.end(); it++)
		{
			if (itemID == (unsigned int)atoi((*it).getItemID()))
			{
			    LogDBG("Selected entity %u", itemID);
				targetSelectedInventory.push_back( (int)itemID );
			}
		}
	}
}


bool BotTradeInv::isSelected( unsigned int itemID )
{
    //LogDBG("Item entity %u selected?",
    //       itemID);

	//Select item in relevant item.
	vector<int>::iterator it;
	for (it = playerSelectedInventory.begin(); it != playerSelectedInventory.end(); it++)
	{
		//LogDBG("player selected item: %u", (unsigned int)*it );
		if (itemID == (unsigned int)*it)
		{
			return true;
		}
	}

	for (it = targetSelectedInventory.begin(); it != targetSelectedInventory.end(); it++)
	{
		//LogDBG("target selected item: %u", (unsigned int)*it );
		if (itemID == (unsigned int)*it)
		{
			return true;
		}
	}

	return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
