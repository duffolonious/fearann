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

#ifndef __FEARANN_BOT_INVENTORY_H__
#define __FEARANN_BOT_INVENTORY_H__


#include "common/patterns/singleton.h"

#include <vector>
#include <string>


class InventoryItem;
class BotInventory;


class ocInventoryObject
{
public:
	/// Enumeration for the tabs of the inventory
	enum TYPE { VOID = 0, EQUIP, CONSUM, RAW, OTHER };

private:
	/// Item identifier
	unsigned int itemID;
	/// Item mesh type
	std::string meshType;
	/// Item mesh subtype
	std::string meshSubtype;
	/// Item load
	float load;

	/// Inventory type of the object
	TYPE type;

public:
	ocInventoryObject(unsigned int id,
			  std::string _meshType, std::string _meshSubtype,
			  float l, TYPE t)
	{
		itemID = id;
		meshType = _meshType;
		meshSubtype = _meshSubtype;
		load = l;
		type = t;
	}

	friend class BotInventory;
};


/** Class contains and manages objects
 */
class BotInventory : public Singleton<BotInventory>
{
public:
	/// Add item to inventory - return false if can't add.
	bool AddItem(InventoryItem* item);

	/// Remove an item from the inventory
	bool RemoveItem(unsigned int itemID);

	/// Remove an item from the inventory
	InventoryItem* getItem(unsigned int itemID);

	/// Remove an item from the inventory
	bool hasItem(unsigned int itemID);

	/// List items in inventory
	void ListInventory();

	void clear();

private:
	/** Singleton friend access */
	friend class Singleton<BotInventory>;


	~BotInventory();

	/// The structure containing the entities in the inventory
	std::vector<ocInventoryObject*> inventory;

	/// Get the code tye of the object
	ocInventoryObject::TYPE GetTypeCode(const char* objectType);

	float mLoad;
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
