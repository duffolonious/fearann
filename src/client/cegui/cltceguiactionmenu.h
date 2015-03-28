/*
 * cltceguiactionmenu.h
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

#ifndef __FEARANN_CLIENT_CEGUI_ACTION_MENU_H__
#define __FEARANN_CLIENT_CEGUI_ACTION_MENU_H__


#include "common/patterns/singleton.h"


namespace CEGUI {
	class EventArgs;
	class WindowManager;
	class Window;
	class PopupMenu;
}
class MsgPlayerData;


/** Class to manage the objects in the inventory, control what to display and so
 * on
 */
class CltCEGUIActionMenu : public Singleton<CltCEGUIActionMenu>
{
public:
	enum ACTION_TYPE { NONE=0, PICKUP, INFO, TRADE, FIGHT };

	/** Update stats such as gold and load, that depend on the state of the
	 * player. */
	void buildMenu( CltEntityBase* entity);
	void setupButton( const char * buttonStr, uint32_t pos );

	/// show window on click...
	bool ShowAt( float norm_x, float norm_y);
	//bool Hide();

private:
	/** Singleton friend access */
	friend class Singleton<CltCEGUIActionMenu>;

	/// We let only this class to access these methods (we need to allow the
	/// window to access them when buttons are clicked)
	friend class CEGUI::Window;

	/** Default constructor */
	CltCEGUIActionMenu();
	/** Destructor */
	~CltCEGUIActionMenu();

	/// Event handler to update the window when shown
	bool Event_Shown(const CEGUI::EventArgs& e);

	/// Perform the menu action
	bool Info_Action(const CEGUI::EventArgs& e);
	bool Pickup_Action(const CEGUI::EventArgs& e);
	bool Fight_Action(const CEGUI::EventArgs& e);
	bool Dialog_Action(const CEGUI::EventArgs& e);
	bool Trade_Action(const CEGUI::EventArgs& e);

	uint64_t mSelectedEntityID;
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
