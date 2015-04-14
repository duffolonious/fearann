/*
 * cltceguimgr.h
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

#ifndef __FEARANN_CLIENT_CEGUI_MGR_H__
#define __FEARANN_CLIENT_CEGUI_MGR_H__


#include "common/patterns/singleton.h"


class MsgPlayerData;
class CltCameraListener;
namespace CEGUI {
  	class GUIContext;
	class Window;
	class WindowManager;
	class EventArgs;
	class System;
}


#define CEGUI_EVENT(window,event,handler)				\
  CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild(window)->subscribeEvent(event, CEGUI::Event::Subscriber(&handler, this));


/** CEGUI Manager is a kind of interface manager, but most of the important
 * elements (inventory, initial menus, etc) have their own class. This is to
 * avoid huge files, and also because some of them need helper classes not
 * related at all with interface elements.
 */
class CltCEGUIMgr : public Singleton<CltCEGUIMgr>
{
public:
	/** Setup the CEGUI system.  It can't be called in the constructor,
	 * because it needs to be set up after the window is created, otherwise
	 * some OpenGL functions are broken. */
	void setup(int screenWidth, int screenHeight);

	/** CEGUI has been initialized and is ready to use (for input as an example) */
	bool isReady();

	/** Called in the main render loop every frame, to render the CEGUI
	 * windows above the rest of the scene. */
	void render();
	/** Apparently we have to disable by hand the focus, because otherwise
	 * CEGUI maitains the focus of the last active window/widget, even if we
	 * click out of it (e.g., the rendering area outside window).  This
	 * should be called whenever a Mouse-Push event is not catched by
	 * CEGUI. */
	void disableFocus();

	/// Get GUIContext (instead of mWinMgr)
	CEGUI::GUIContext* getGUIContext() { return mGUIContext; };

	/// Display a warning/notification message
	bool Notification_DisplayMessage(const char* msg);

	/// Display Quit dialog
	void Quit_DisplayDialog();

	/** Make the necessary changes to start playing (show in-game windows,
	 * destroy initial menus, etc). */
	void StartPlaying();

	/// Toggle the inventory window
	void ToggleWindow_Inventory();

	/// Add a contact to the list
	void Contacts_AddToList(const char* name, char type, char status,
				const char* last_login, const char* comment);

	/// Display combat accept/cancel message.
	void Combat_DisplayMessage( const char * msg );

	/** Update player statistics */
	void PlayerStats_Update(MsgPlayerData* msg);
	/** Update the text of camera mode */
	static void PlayerStats_SetCameraModeName(const char* name);

	/** Update the time */
	void Calendar_SetTime(const std::string& time,
			      const std::string& date,
			      const std::string& year);
	/** Update the picture of the moon */
	void Calendar_SetMoonPicture(const std::string& moonPictureName);

	/** Convert OSG normalized mouse coord to CEGUI coord. */
	float convertXToCEGUI( float norm_x );
	float convertYToCEGUI( float norm_y );

private:
	/** Singleton friend access */
	friend class Singleton<CltCEGUIMgr>;

	/// We let only this class to access these methods (we need to allow the
	/// window to access them when buttons are clicked)
	friend class CEGUI::Window;

	/// Pointer to the CEGUI system (to delete it when exiting)
	CEGUI::System* mCEGUISystem;

	/// Pointer to the window manager
	CEGUI::WindowManager* mWinMgr;

	/// Pointer to GUIContext
	CEGUI::GUIContext* mGUIContext;

	/// Listener for camera events
	CltCameraListener* mCameraListener;

	/// True if CEGUI is ready to use
	bool mReady;

	/** Default constructor */
	CltCEGUIMgr();
	/** Default destructor */
	~CltCEGUIMgr();

	/// Callback for the Close button in the Notification dialog
	bool Notification_Close(const CEGUI::EventArgs& e);

	/// Callback for the Cancel button in the Quit dialog
	bool Quit_Cancel(const CEGUI::EventArgs& e);
	/// Callback for the Quit button in the Quit dialog
	bool Quit_Quit(const CEGUI::EventArgs& e);

	/// Callback for the No button on the Combat Dialog
	bool Combat_Cancel(const CEGUI::EventArgs& e);
	/// Callback for the Yes button on the Combat Dialog
	bool Combat_Accept(const CEGUI::EventArgs& e);

	/// Callback for the Inventory button in the Panel window
	bool Panel_InventoryButton(const CEGUI::EventArgs& e);
	/// Callback for the Contacts button in the Panel window
	bool Panel_ContactsButton(const CEGUI::EventArgs& e);
	/// Callback for the Quit button in the Panel window
	bool Panel_QuitButton(const CEGUI::EventArgs& e);

	/// Callback for the Add button in the Contacts window
	bool Contacts_Add(const CEGUI::EventArgs& e);
	/// Callback for the Remove button in the Contacts window
	bool Contacts_Remove(const CEGUI::EventArgs& e);

	/// Callback for the Camera button in the PlayerStats window
	bool PlayerStats_Camera(const CEGUI::EventArgs& e);
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
