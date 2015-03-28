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

#ifndef __FEARANN_CLIENT_CEGUI_DIALOG_H__
#define __FEARANN_CLIENT_CEGUI_DIALOG_H__


#include "common/patterns/singleton.h"
#include <list>


namespace CEGUI {
	class EventArgs;
	class WindowManager;
	class Window;
}
class MsgPlayerData;


/** Class to manage the objects in the inventory, control what to display and so
 * on
 */
class CltCEGUIDialog : public Singleton<CltCEGUIDialog>
{
public:
	enum ACTION_TYPE { NONE=0, PICKUP, INFO, TRADE, FIGHT, DIALOG };

	/** start a dialog with an NPC */
	bool initiateDialog( uint64_t entityID );

	/** When a dialog msg is received - populate dialog */
	void updateDialog( MsgNPCDialog* );

private:
	/** Singleton friend access */
	friend class Singleton<CltCEGUIDialog>;

	/// We let only this class to access these methods (we need to allow the
	/// window to access them when buttons are clicked)
	friend class CEGUI::Window;

	/** Default constructor */
	CltCEGUIDialog();
	/** Destructor */
	~CltCEGUIDialog();

	/// Event handler to update the window when shown
	bool Event_Shown(const CEGUI::EventArgs& e);

	/// Perform the menu action
	bool Event_Reply(const CEGUI::EventArgs& e);

	/// Close dialog
	bool Event_Close(const CEGUI::EventArgs& e);

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
