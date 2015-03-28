/*
 * cltceguiinventory.h
 * Copyright (C) 2005-2006 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#ifndef __FEARANN_CLIENT_CEGUI_INVENTORY_H__
#define __FEARANN_CLIENT_CEGUI_INVENTORY_H__


#include "common/patterns/singleton.h"

#include <vector>


namespace CEGUI {
	class EventArgs;
	class WindowManager;
	class Window;
}
class InventoryItem;
class MsgPlayerData;


/** Class to represent an object in the inventory, with the interesting
 * properties only
 */
class CltInventoryObject
{
public:
	/// Enumeration for the tabs of the inventory
	enum TYPE { VOID = 0, EQUIP, CONSUM, RAW, OTHER };

public:
	CltInventoryObject(uint32_t id, std::string _meshType, std::string _meshSubtype, float l, TYPE t) :
		itemID(id), meshType(_meshType), meshSubtype(_meshSubtype), load(l), type(t) { }

private:
	friend class CltCEGUIInventory;

	/// Item identifier
	uint32_t itemID;
	/// Item mesh type
	std::string meshType;
	/// Item mesh subtype
	std::string meshSubtype;
	/// Item load
	float load;
	/// Inventory type of the object
	TYPE type;
};


/** Class to manage the objects in the inventory, control what to display and so
 * on
 */
class CltCEGUIInventory : public Singleton<CltCEGUIInventory>
{
public:
	/** Add item to the inventory, should only return false if can't hold it
	 * because of heavy load */
	void addItem(InventoryItem* item);
	/** Remove an item from the inventory */
	void removeItem(uint32_t itemID);

	/** Update stats such as gold and load, that depend on the state of the
	 * player. */
	void updateStats(const MsgPlayerData* msg);

	/** Update window, showing the objects accordingly with selected tab */
	void updateWindow(const char* tab);

private:
	/** Singleton friend access */
	friend class Singleton<CltCEGUIInventory>;

	/// We let only this class to access these methods (we need to allow the
	/// window to access them when buttons are clicked)
	friend class CEGUI::Window;

	/// The structure containing the entities in the inventory
	std::vector<CltInventoryObject*> mInventory;

	std::vector<CltInventoryObject*> mTradeList;
	std::vector<CltInventoryObject*> mTargetTradeList;

	/** Default constructor */
	CltCEGUIInventory();
	/** Default destructor */
	~CltCEGUIInventory();

	/// Get the code tye of the object
	CltInventoryObject::TYPE getTypeCode(const char* objectType);

	/// Create the slots (programatically is more flexible)
	void createSlots();

	/// Event handler for the tab selection
	bool Event_TabSelectionChanged(const CEGUI::EventArgs& e);

	/// Event handler to update the window when shown
	bool Event_Shown(const CEGUI::EventArgs& e);

	/// Event handler for the drop button
	bool Event_Drop(const CEGUI::EventArgs& e);

	/// Commits selected items.
	bool Event_Commit(const CEGUI::EventArgs& e);
	/// Accepts commit.
	bool Event_CommitAccept(const CEGUI::EventArgs& e);
	/// Denies commit.
	bool Event_CommitDeny(const CEGUI::EventArgs& e);
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
