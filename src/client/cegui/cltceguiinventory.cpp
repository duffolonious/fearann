/*
 * cltceguiinventory.cpp
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
#include "client/cltconfig.h"

#include <cstdlib>

#include <CEGUI/WindowManager.h>
#include <CEGUI/widgets/Editbox.h>
#include <CEGUI/widgets/MultiColumnList.h>
#include <CEGUI/widgets/PushButton.h>
#include <CEGUI/widgets/Scrollbar.h>
#include <CEGUI/widgets/TabControl.h>
#include <CEGUI/widgets/ListboxItem.h>
#include <CEGUI/widgets/ListboxTextItem.h>

#include "common/net/msgs.h"

#include "client/entity/cltentitymainplayer.h"
#include "client/content/cltcontentloader.h"

#include "cltceguimgr.h"

#include "cltceguiinventory.h"


//----------------------- CltCEGUIInventory ----------------------------
template <> CltCEGUIInventory* Singleton<CltCEGUIInventory>::INSTANCE = 0;

CltCEGUIInventory::CltCEGUIInventory()
{
	// installing events
	CEGUI_EVENT("Inventory",
		    CEGUI::Window::EventShown,
		    CltCEGUIInventory::Event_Shown);
	CEGUI_EVENT("Inventory/Tabs",
		    CEGUI::TabControl::EventSelectionChanged,
		    CltCEGUIInventory::Event_TabSelectionChanged);
	CEGUI_EVENT("Inventory/Drop",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInventory::Event_Drop);
#if 0 //FIXME: tabs auto pane?
	// set list as child of the autopane
	CEGUI::Window* autoPane = CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Inventory/Tabs__auto_TabPane__");
        PERM_ASSERT(autoPane);
	CEGUI::Window* invList = CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Inventory/List");
        PERM_ASSERT(invList);
	autoPane->addChild(invList);
#endif
	// select the initial tab
	updateWindow("All");
}

CltCEGUIInventory::~CltCEGUIInventory()
{
        for (size_t i = 0; i < mInventory.size(); ++i) {
		delete mInventory[i];
        }
}

bool CltCEGUIInventory::Event_TabSelectionChanged(const CEGUI::EventArgs& e)
{
	CEGUI::TabControl* tab = static_cast<CEGUI::TabControl*>
                (CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Inventory/Tabs"));
        PERM_ASSERT(tab);

	// making the necessary changes to show the new tab
        string tabName = tab->getTabContentsAtIndex(tab->getSelectedTabIndex())->getName().c_str();
        string baseName = "Inventory/Tab";
        tabName.erase(0, baseName.size());

	updateWindow(tabName.c_str());

	return true;
}

bool CltCEGUIInventory::Event_Shown(const CEGUI::EventArgs& e)
{
	// we can rely on what TabSelectionChange does at the moment
	return CltCEGUIInventory::Event_TabSelectionChanged(e);
}

bool CltCEGUIInventory::Event_Drop(const CEGUI::EventArgs& e)
{
	CEGUI::MultiColumnList* invList = static_cast<CEGUI::MultiColumnList*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Inventory/List"));
        PERM_ASSERT(invList);

	// check that there's something selected
        if (invList->getSelectedCount() == 0) {
                LogWRN("No item selected to drop, ignoring request");
                return true;
        }

	// get the ID of the item (necessary for the drop message)
	CEGUI::ListboxItem* item = invList->getFirstSelectedItem();
        PERM_ASSERT(item);
        uint32_t itemID = static_cast<uint32_t>(atoi(item->getText().c_str()));

	// ask to drop it
	CltEntityMainPlayer::instance().drop(itemID);

        return true;
}

void CltCEGUIInventory::createSlots()
{
	/// \todo mafm: not working

//	CEGUI::Window* tabWdg = CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Inventory/Tabs__auto_TabPane__");
	CEGUI::Window* tabWdg = CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Inventory/Pane");
	PERM_ASSERT(tabWdg);

	int serial = 0;
        for (size_t r = 0; r < 5; ++r)
	{
		for (size_t c = 0; c < 1; ++c)
		{
			string slotName = StrFmt("Inventory/Slot_%d", serial++);
			CEGUI::Window* newSlot = CEGUI::WindowManager::getSingleton().createWindow("OnceInGameTheme/StaticText",
								      slotName.c_str());
			tabWdg->addChild(newSlot);
			newSlot->setText(slotName.c_str());
			newSlot->setVisible(true);
			/*CEGUI::Point pos(float(r*10.f), float(c*10.0f));
			  newSlot->setPosition(pos);*/
		}
	}
}

CltInventoryObject::TYPE CltCEGUIInventory::getTypeCode(const char* objectType)
{
	string type = objectType;

	if (type == "Equipment"
	    || type == "Tool" || type == "Weapon")
		return CltInventoryObject::EQUIP;
	else if (type == "Consumables"
		|| type == "Food")
		return CltInventoryObject::CONSUM;
	else if (type == "Raw"
		 || type == "Resource")
		return CltInventoryObject::RAW;
	else if (type == "Other"
		 || type == "Other")
		return CltInventoryObject::OTHER;
	else
		return CltInventoryObject::VOID;
}

void CltCEGUIInventory::addItem(InventoryItem* item)
{
	PERM_ASSERT (item);
	const char* invType;
	const ContentInfoBase* contentInfo = CltContentLoader::instance().getInfoItem(item->getType());
	if ( !contentInfo )
		invType = "Other";
	else
		invType = contentInfo->getEntityClass();

	LogDBG("Adding entity (%s, %s, %s, %g) to the inventory",
	       item->getItemID(), item->getType(),
	       item->getSubtype(), item->getLoad());

	// adding this entity to the inventory list
	uint32_t itemID = static_cast<uint32_t>(atoi(item->getItemID()));
	CltInventoryObject* elem = new CltInventoryObject(itemID,
							  item->getType(),
							  item->getSubtype(),
							  item->getLoad(),
							  getTypeCode(invType));
	mInventory.push_back(elem);

	// update the window, otherwise the item won't appear
	updateWindow("All");
}

void CltCEGUIInventory::removeItem(uint32_t itemID)
{
	LogDBG("Removing entity %u from the inventory", itemID);

	// removing this entity from the inventory list
	for (vector<CltInventoryObject*>::iterator it = mInventory.begin(); it != mInventory.end(); ++it) {
		if (itemID == (*it)->itemID) {
			mInventory.erase(it);

			// if not updated the item sits there
			updateWindow("All");

			return;
		}
	}

	LogERR("Entity not found in inventory when trying to remove it");
}

void CltCEGUIInventory::updateStats(const MsgPlayerData* msg)
{
	CEGUI::WindowManager* winMgr = &CEGUI::WindowManager::getSingleton();
	string text;

	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	text = StrFmt("Load: %d/%d", msg->load_cur, msg->load_max);
	root->getChild("Inventory/Load_Lbl")->setText(text.c_str());

	text = StrFmt("Gold: %5d", msg->gold);
	root->getChild("Inventory/Gold_Lbl")->setText(text.c_str());
}

void CltCEGUIInventory::updateWindow(const char* tab)
{
	LogDBG("UpdateWindow: tab %s", tab);

	// mafm: It could be better (from the point of view of efficiency) to
	// have different list and update them only when adding or removing
	// objects. But on the other hand, this requires to use permanently
	// memory for several lists (some of them duplicated, at least with the
	// "All" tab), and the lists could get outdated if we forget to update
	// with every object that we add or remove, or if we change the lists
	// available. So at least until we discover noticeable delays when
	// changing tabs, it's probably safer to do it in this way, and we save
	// at least coding time.

	CEGUI::MultiColumnList* invList = static_cast<CEGUI::MultiColumnList*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Inventory/List"));
        PERM_ASSERT(invList);

	// only update if visible, otherwise it will be updated later
	if (!invList->isVisible())
		return;

	// clear the list
	invList->resetList();

	// getting objects of this type
	int typeFilter = getTypeCode(tab);
        for (size_t i = 0; i < mInventory.size(); ++i) {
		CltInventoryObject* elem = mInventory[i];
		PERM_ASSERT(elem);
		if (typeFilter != 0 && typeFilter != elem->type)
			continue;

		const char* invType, *prettyName;
		const ContentInfoBase* contentInfo = CltContentLoader::instance().getInfoItem(elem->meshType.c_str());
		if ( !contentInfo )
		{
			invType = "Other";
			prettyName = elem->meshType.c_str();
		}
		else
		{
			invType = contentInfo->getEntityClass();
			prettyName = contentInfo->getPrettyName();
		}

		LogDBG(" - id: %d, meshType: %s, meshSubtype: %s, "
		       "type: %d, otype: %s",
		       elem->itemID,
		       elem->meshType.c_str(), elem->meshSubtype.c_str(),
		       elem->type, invType);

		CEGUI::String text = StrFmt("%d", elem->itemID);
		CEGUI::ListboxTextItem* elemID = new CEGUI::ListboxTextItem(text);
		text = prettyName;
		CEGUI::ListboxTextItem* elemName = new CEGUI::ListboxTextItem(text);
		text = StrFmt("%.2f", elem->load);
		CEGUI::ListboxTextItem* elemLoad = new CEGUI::ListboxTextItem(text);

		// we have to set up this in order to get them
		// highlighted
		elemID->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
		elemName->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
		elemLoad->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");

		unsigned int index = invList->addRow();
		invList->setItem(elemID, 0, index);
		invList->setItem(elemName, 1, index);
		invList->setItem(elemLoad, 2, index);
        }

        // sorting by name
        invList->setSortColumnByID(1);
        invList->setSortDirection(CEGUI::ListHeaderSegment::Descending);

/* mafm: useful when we have graphical slots

	CEGUI::StaticImage* slotImg = static_cast<CEGUI::StaticImage*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Slot_0"));
	PERM_ASSERT(slotImg);
	slotImg->setImage("InventoryIcons", "stick_0");
	slotImg->show();
	slotImg->requestRedraw();
	LogDBG(" - child win: %s -- %s -- %d",
	       slotImg->getName().c_str(),
	       slotImg->getType().c_str(),
	       slotImg->isVisible());
*/
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
