/*
 * cltceguiconsole.h
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

#ifndef __FEARANN_CLIENT_CEGUI_CONSOLE_H__
#define __FEARANN_CLIENT_CEGUI_CONSOLE_H__


#include "common/patterns/singleton.h"


namespace CEGUI {
        class GUIContext;
	class Window;
	class WindowManager;
	class EventArgs;
}
class MsgChat;


/** Manager for the console window
 */
class CltCEGUIConsole : public Singleton<CltCEGUIConsole>
{
public:
	/** Print a message in the graphical console */
	void printMessage(const MsgChat& msg);

private:
	/** Singleton friend access */
	friend class Singleton<CltCEGUIConsole>;

	/// We let only this class to access these methods (we need to allow the
	/// window to access them when buttons are clicked)
	friend class CEGUI::Window;

	/// Callback for the Tab Selection in the Console window
	bool Event_TabSelectionChanged(const CEGUI::EventArgs& e);

	/// Callback for the Editbox KeyPressed event in the Console window
	bool Event_KeyPressed(const CEGUI::EventArgs& e);

	/// Callback for the Editbox in the Console window
	bool Event_EditTextAccepted(const CEGUI::EventArgs& e);

	/// Pointer to the window manager
	CEGUI::WindowManager* mWinMgr;

	/** This stores the text the player has entered into the entrybox of the
	 * console/chat window.  It keeps all the commands that the player has
	 * entered in sequential order, either chat phrases or /commands.
	 */
	class ChatHistory {
	public:
		/** Default constructor */
		ChatHistory();
		/** Insert a new string, the last one typed */
		void insert(const char* str);
		/** Get a string from the history */
		const char* getByIndex(size_t i);
		/** Return the next command from history (empty for
		 * the last) */
		const char* getNext();
		/** Return the previous command from history */
		const char* getPrev();
	private:
		/// Array of strings to store our history
		std::vector<std::string> mLineList;
		/// Pointer to the current line
		size_t mIndex;
	} mConsoleHistory;


	/** Default constructor */
	CltCEGUIConsole();
	/** Default destructor */
	~CltCEGUIConsole();

	/** Helper for the Tab Selection, to put the text related with that
	 * tab */
	void changeTab(const std::string& tabName);
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
