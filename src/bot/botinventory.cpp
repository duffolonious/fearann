/*
 * botinventory.cpp
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

#include "common/util.h"

#include "botinventory.h"

#include <cstdlib>

//----------------------- BotInventory ----------------------------
template <> BotInventory* Singleton<BotInventory>::INSTANCE = 0;

BotInventory::~BotInventory()
{
	clear();
}

void BotInventory::clear()
{
    std::vector<ocInventoryObject*>::iterator it;
    for (size_t i = 0; i < inventory.size(); i++)
    {
        delete inventory[i];
    }
	inventory.clear();
}

ocInventoryObject::TYPE BotInventory::GetTypeCode(const char* objectType)
{
    std::string type = objectType;

    if (type == "Equipment"
        || type == "Tool" || type == "Weapon")
        return ocInventoryObject::EQUIP;
    else if (type == "Consumables"
        || type == "Food")
        return ocInventoryObject::CONSUM;
    else if (type == "Raw"
         || type == "Resource")
        return ocInventoryObject::RAW;
	else if (type == "Other"
         || type == "Other")
        return ocInventoryObject::OTHER;
    else
        return ocInventoryObject::VOID;
}

bool BotInventory::AddItem(InventoryItem* item)
{
    PERM_ASSERT (item);

    LogDBG("Adding entity ('%s', '%s', '%s', '%f') to the inventory",
           item->getItemID(), item->getType(),
           item->getSubtype(), item->getLoad());

    // adding this entity to the inventory list
    unsigned int itemID = unsigned(atoi(item->getItemID()));
    ocInventoryObject* elem = new ocInventoryObject(itemID,
                            item->getType(),
                            item->getSubtype(),
                            item->getLoad(),
                            ocInventoryObject::VOID);
	mLoad += item->getLoad();
    inventory.push_back(elem);

	return true;
}

bool BotInventory::RemoveItem(unsigned int itemID)
{
	LogDBG("Removing entity %u from the inventory",
           itemID);

    if (inventory.size() <= 0)
    {
        LogERR("Inventory is empty, cannot remove anything");
        return false;
    }

	// removing this entity from the inventory list
    vector<ocInventoryObject*>::iterator it;
    for (it = inventory.begin(); it != inventory.end(); it++)
	{
        if (itemID == (*it)->itemID)
        {
			mLoad -= (*it)->load;
            inventory.erase(it);
            return true;
        }
    }

    LogERR("Entity not found in inventory when trying to remove it");
	return false;
}


InventoryItem* BotInventory::getItem(unsigned int itemID)
{
	LogDBG("Getting entity %u from the inventory",
           itemID);

    if (inventory.size() <= 0)
    {
        LogERR("Inventory is empty, cannot get anything");
        return 0;
    }

	// removing this entity from the inventory list
    vector<ocInventoryObject*>::iterator it;
    for (it = inventory.begin(); it != inventory.end(); it++)
	{
        if (itemID == (*it)->itemID)
        {
			InventoryItem* tmp = new InventoryItem( (*it)->itemID, 
						(*it)->meshType, 
						(*it)->meshSubtype, 
						(*it)->load );
            return tmp;
        }
    }

    LogERR("Entity not found in inventory when trying to remove it");
	return 0;
}


bool BotInventory::hasItem(unsigned int itemID)
{
	LogDBG("Checking if entity %u is in the inventory",
           itemID);

    if (inventory.size() <= 0)
    {
        LogERR("Inventory is empty, cannot get anything");
        return false;
    }

	// removing this entity from the inventory list
    vector<ocInventoryObject*>::iterator it;
    for (it = inventory.begin(); it != inventory.end(); it++)
	{
        if (itemID == (*it)->itemID)
        {
			return true;
        }
    }

    LogERR("Entity not found in inventory when trying to remove it");
	return false;
}


void BotInventory::ListInventory()
{
    std::vector<ocInventoryObject*>::iterator it;

	if (inventory.size() == 0)
	{
		LogNTC("Inventory empty");
		return;
	}

    for (it = inventory.begin(); it != inventory.end(); it++)
	{
		LogNTC("id: %u type: %s sub: %s load: %f",
								(*it)->itemID,
								(*it)->meshType.c_str(),
								(*it)->meshSubtype.c_str(),
								(*it)->load);
	}

	return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
