/*
 * cltceguiinitial.h
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

#ifndef __FEARANN_CLIENT_CEGUI_INITIAL_H__
#define __FEARANN_CLIENT_CEGUI_INITIAL_H__


#include "common/patterns/singleton.h"
#include "common/net/msgs.h"

#include <string>
#include <bits/stl_pair.h>


namespace CEGUI {
	class Window;
	class WindowManager;
	class EventArgs;
}
class CharListHelper;
class CltContentListener;
class PingClient;


/** Initial menus, from startup to joining the game
 */
class CltCEGUIInitial : public Singleton<CltCEGUIInitial>
{
public:
	/** Whether to do or not the content update.  It's controlled via
	 * command line options and/or config file, and should be set before
	 * calling setup (otherwise, some initial buttons or windows might be
	 * disabled, etc). */
	void setDoContentUpdate(bool value);
	/** Setup the initial menus.  It can't be called in the constructor,
	 * because CEGUI needs to be set up after the window is created,
	 * otherwise some OpenGL functions are broken; and we need CEGUI up
	 * before we go on with this. */
	void setup();

	/** Process existing replies (it has to do every frame, to not introduce
	 * unneded delays) */
	void Connect_ProcessPingReplies(float timestamp);
	/** Send pings periodically */
	void Connect_SendPings(float timestamp);

	/** Fill the list with the new character created */
	bool Join_FillNewChar(std::string charname,
			      std::string race,
			      std::string gender,
			      std::string playerClass,
			      std::string area);
	/// Remove from the list with the character deleted
	bool Join_RemoveDelChar(std::string charname);

	/// Bridge between Connect and Login screens
	void Connect_to_Login(const char* uptime, const char* players,
			      const char* users, const char* chars);
	/// Bridge between Login and Join screens
	void Login_to_Join(std::vector<MsgLoginReply::CharacterListEntry>& charList);
	/// Bridge between Login and NewUser screens
	void Login_to_NewUser();
	/// Bridge between NewUser and Login screens
	void NewUser_to_Login();
	/// Bridge between Join and NewChar screens
	void Join_to_NewChar();
	/// Bridge between NewChar and Join screens
	void NewChar_to_Join();
	/// Bridge between Join and LoaginGame screens
	void Join_to_LoadingGame();
	/// Bridge between LoaginGame and Game screens
	void LoadingGame_to_Game();

	/// Function to reset the state when login fails
	void Login_Failed();

private:
	/** Singleton friend access */
	friend class Singleton<CltCEGUIInitial>;

	/// Pointer to the window manager
	CEGUI::WindowManager* mWinMgr;

	/// The ping client, to handle or ping queries and replies
	PingClient* mPingClient;

	/// Helper structure to keep track of character info 
	CharListHelper* mCharListHelper;

	/// Total number of points that can be used for new chars
	int pointsNewChar;

	/// Whether we have content update into account (to raise windows, to
	/// not join until complete, etc)
	bool mDoContentUpdate;
	/// Whether the content update is ready or not
	bool mContentUpdateReady;
	/// Helper to communicate the content update manager with the download
	/// window
	CltContentListener* mContentListener;


	/// We allow only this class to access these methods (we need to allow
	/// the window to access them when buttons are clicked)
	friend class CEGUI::Window;

	/** Default constructor */
	CltCEGUIInitial();
	/** Destructor */
	~CltCEGUIInitial();

	/** Update values in the list with pong reply delay */
	void Connect_UpdatePingReply(uint32_t entry,
				     const char* text);
	/** Fill in the server list in the Connect menu */
	void Connect_LoadConnectData();
	/// Callback for the Quit button in the Connect menu 
	bool Connect_Quit(const CEGUI::EventArgs& e);
	/// Callback for the Options button in the Connect menu 
	bool Connect_Options(const CEGUI::EventArgs& e);
	/// Callback for the Connect button in the Connect menu 
	bool Connect_Connect(const CEGUI::EventArgs& e);

	/** Raise download window */
	void Download_RaiseWindow();

	/** Update file stats */
	void Download_UpdateFileStats(const char* fileName, float pct);
	/** Update total stats (when a file is finished) */
	void Download_UpdateTotalStats(const char* fileName, float pct);
	/** Update the window when the update is over */
	void Download_Completed(float sizeKB, float seconds, float avgSpeed);

	/** Fill in the login data in the Login menu */
	void Login_LoadLoginData();
	/// Callback for the Quit button in the Login menu 
	bool Login_Quit(const CEGUI::EventArgs& e);
	/// Callback for the NewUser button in the Login menu 
	bool Login_NewUser(const CEGUI::EventArgs& e);
	/// Callback for the Login button in the Login menu 
	bool Login_Login(const CEGUI::EventArgs& e);

	/// Callback for the Cancel button in the NewUser menu
	bool NewUser_Cancel(const CEGUI::EventArgs& e);
	/// Callback for the Create button in the NewUser menu
	bool NewUser_Create(const CEGUI::EventArgs& e);

	/// Callback for selection changes button in the Join menu
	bool Join_OnSelectionChanged(const CEGUI::EventArgs& e);
	/// Callback for the Quit button in the Join menu
	bool Join_Quit(const CEGUI::EventArgs& e);
	/// Callback for the Join button in the Join menu
	bool Join_Join(const CEGUI::EventArgs& e);
	/// Callback for the NewChar button in the Join menu
	bool Join_NewChar(const CEGUI::EventArgs& e);
	/// Callback for the DelChar button in the Join menu
	bool Join_DelChar(const CEGUI::EventArgs& e);

	/// Fill in the data in the NewChar menu
	void NewChar_LoadNewCharData();
	/// To recalculate the bonus for abilities depending on race
	void NewChar_RecalculateBonusPoints(const char* race);
	/// To recalculate the total scores for abilites
	void NewChar_RecalculateTotalPoints();
	/// Callback for the Cancel button in the NewChar menu
	bool NewChar_Cancel(const CEGUI::EventArgs& e);
	/// Callback for the Create button in the NewChar menu
	bool NewChar_Create(const CEGUI::EventArgs& e);
	/// Callback for the Spinners in the NewChar menu
	bool NewChar_SpinnerValueChanged(const CEGUI::EventArgs& e);
	/// Callback for the Comboboxes in the NewChar menu
	bool NewChar_ComboboxSelectionAccepted(const CEGUI::EventArgs& e);
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
