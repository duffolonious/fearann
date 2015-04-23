/*
 * cltceguiconsole.cpp
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

#include <CEGUI/CEGUI.h>
#include <CEGUI/widgets/Editbox.h>
#include <CEGUI/widgets/MultiLineEditbox.h>
#include <CEGUI/widgets/TabButton.h>
#include <CEGUI/widgets/Scrollbar.h>
#include <CEGUI/widgets/TabControl.h>

#include "common/command.h"
#include "common/net/msgs.h"

#include "client/cltcommand.h"

#include "client/cegui/cltceguimgr.h"

#include "cltceguiconsole.h"


//----------------------- CltCEGUIConsole ----------------------------
template <> CltCEGUIConsole* Singleton<CltCEGUIConsole>::INSTANCE = 0;

CltCEGUIConsole::CltCEGUIConsole()
{
	mWinMgr = &CEGUI::WindowManager::getSingleton();

	// initialize window
	changeTab(string("All"));

	// installing events
	CEGUI_EVENT("Console/Tabs",
		    CEGUI::TabControl::EventSelectionChanged,
		    CltCEGUIConsole::Event_TabSelectionChanged);

	CEGUI_EVENT("Console/Entrybox",
		    CEGUI::Editbox::EventTextAccepted,
		    CltCEGUIConsole::Event_EditTextAccepted);

	CEGUI_EVENT("Console/Entrybox",
		    CEGUI::Window::EventKeyDown,
		    CltCEGUIConsole::Event_KeyPressed);
}

CltCEGUIConsole::~CltCEGUIConsole()
{
}

void CltCEGUIConsole::changeTab(const string& tabName)
{
	string tabNameShort(tabName);
	string baseName("Console/Tabs/");
	//tabNameShort.erase(0, baseName.size());

	// sanity check
	if (tabNameShort != "All" &&
	    tabNameShort != "System" &&
	    tabNameShort != "Action" &&
	    tabNameShort != "Chat" &&
	    tabNameShort != "PM") {
		LogERR("Tab button in Console window unknown: '%s' ('%s')",
			tabName.c_str(), tabNameShort.c_str());
		return;
	}

	// put the multiline box accordingly with the tab (there's a different
	// MLBox for each message type)
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	string screenTabName = StrFmt("Console/Screen%s", tabNameShort.c_str());
	CEGUI::Window* screenTabW = root->getChild(screenTabName);
	string screenName("Console/Screen");
	CEGUI::MultiLineEditbox* screenW = dynamic_cast<CEGUI::MultiLineEditbox*>(root->getChild(screenName));
	screenW->setText(screenTabW->getText());
#if 0 //not sure if this even exists anymore - with 0.8.4 it crashes CEGUI
	string screenScroll("Console/Screen__auto_vscrollbar__");
	dynamic_cast<CEGUI::Scrollbar*>(root->getChild(screenScroll))->setScrollPosition(100000.0f);
#endif
	// un-highlight the selected tab button
	CEGUI::TabButton* btn = dynamic_cast<CEGUI::TabButton*>(root->getChild(baseName + tabName));
	btn->setProperty("NormalTextColour", "ffc0c0c0");
}

void CltCEGUIConsole::printMessage(const MsgChat& msg)
{
	// select tabs/screens depending on type, and formatting line
	string line;
	string targetScreen = "Console/Screen";
	string targetButton = "Console/Tabs/";
	switch (msg.type)
	{
	case MsgChat::SYSTEM:
		line = StrFmt("%s :: %s", msg.origin.c_str(), msg.text.c_str());
		targetScreen += "System";
		targetButton += "System";
		break;
	case MsgChat::ACTION:
		line = StrFmt("%s (action) :: %s", msg.origin.c_str(), msg.text.c_str());
		targetScreen += "Action";
		targetButton += "Action";
		break;
	case MsgChat::CHAT:
		line = StrFmt("<%s> %s", msg.origin.c_str(), msg.text.c_str());
		targetScreen += "Chat";
		targetButton += "Chat";
		break;
	case MsgChat::PM:
		line = StrFmt("<%s (priv. msg)> %s", msg.origin.c_str(), msg.text.c_str());
		targetScreen += "PM";
		targetButton += "PM";
		break;
	default:
		LogERR("Message type unknown to be printed in Console window");
		return;
	}

	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	// add new line to correct tab
	CEGUI::MultiLineEditbox* screen = dynamic_cast<CEGUI::MultiLineEditbox*>
		(root->getChild(targetScreen));
	string text = screen->getText().c_str();
	text += line;
	screen->setText(text);

	// add new line to All tab
	CEGUI::MultiLineEditbox* screenAll = dynamic_cast<CEGUI::MultiLineEditbox*>
		(root->getChild("Console/ScreenAll"));
	text = screenAll->getText().c_str();
	text += line;
	screenAll->setText(text);

	// update the text, simulating that tab changed
	CEGUI::TabControl* tab = dynamic_cast<CEGUI::TabControl*>(root->getChild("Console/Tabs"));
	string tabName = tab->getTabContentsAtIndex(tab->getSelectedTabIndex())->getName().c_str();
	changeTab(tabName);

	/// \todo mafm: this is not working at the moment, and CEGUI guys say
	/// that we should do things in a fundamentally different way.  I rather
	/// let it be with this minor problem, than wasting several hours/days
	/// rewritting the class, just to find out another CEGUI shortcoming
	/// probably...

	// highlight the tab button
	CEGUI::TabButton* btn = dynamic_cast<CEGUI::TabButton*>(root->getChild(targetButton));
	btn->setProperty("NormalTextColour", "ffff0000");
	CEGUI::String selectedCol = btn->getProperty("SelectedTextColour");
}

bool CltCEGUIConsole::Event_TabSelectionChanged(const CEGUI::EventArgs& e)
{
	// find the tab which has been selected
	CEGUI::TabControl* tab = dynamic_cast<CEGUI::TabControl*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Console/Tabs"));
	string tabName = tab->getTabContentsAtIndex(tab->getSelectedTabIndex())->getName().c_str();
	// call the function to do the real job
	changeTab(tabName);

	return true;
}

bool CltCEGUIConsole::Event_KeyPressed(const CEGUI::EventArgs& e)
{
	// get the widget
	CEGUI::Editbox* entry = dynamic_cast<CEGUI::Editbox*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Console/Entrybox"));
	const char* cmd = 0;

	const CEGUI::KeyEventArgs& keyArgs = dynamic_cast<const CEGUI::KeyEventArgs&>(e);

	// handle in this special way the Up/Down arrow keys, to implement the
	// chat history
	switch (keyArgs.scancode)
	{
	case CEGUI::Key::ArrowUp:
		cmd = mConsoleHistory.getPrev();
		if (cmd)
			entry->setText(cmd);
		break;
	case CEGUI::Key::ArrowDown:
		cmd = mConsoleHistory.getNext();
		if (cmd)
			entry->setText(cmd);
		break;
	default:
		return false;
	}

	return true;
}

bool CltCEGUIConsole::Event_EditTextAccepted(const CEGUI::EventArgs& e)
{
	CEGUI::Editbox* entry = dynamic_cast<CEGUI::Editbox*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Console/Entrybox"));

	// send the text as chat message
	if (entry->getText().length() == 0) {
		// ignoring empty string
	} else {
		// process the input
		string cmd = entry->getText().c_str();
		if (cmd[0] != '/') {
			cmd = string("/say ") + cmd;
		}

		// execute the command and print output
		CommandOutput out;
		CltCommandMgr::instance().execute(&cmd[1], PermLevel::PLAYER, out);
		if (out.getOutput().length() > 0) {
			// print this only if we have something to print (error
			// or so)
			MsgChat consoleMsg;
			consoleMsg.origin = "Client";
			consoleMsg.type = MsgChat::SYSTEM;
			consoleMsg.text = out.getOutput();
			printMessage(consoleMsg);
		}

		// insert line into the history and clean the entrybox
		mConsoleHistory.insert(entry->getText().c_str());
		entry->setText("");
	}

	return true;
}

//------------------- CltCEGUIConsole::ChatHistory ----------------
CltCEGUIConsole::ChatHistory::ChatHistory()
{
	// adding empty string as default entry
	mLineList.push_back(string(""));
	mIndex = 1;
}

void CltCEGUIConsole::ChatHistory::insert(const char* str)
{
	mLineList.push_back(string(str));
	mIndex = mLineList.size();
}

const char* CltCEGUIConsole::ChatHistory::getByIndex(size_t i)
{
	if (i >= mLineList.size()) {
		LogDBG("ChatHistory: getByIndex: n>size, returning history[back]: %s",
		       mLineList.back().c_str());
		return mLineList.back().c_str();
	} else {
		return mLineList[i].c_str();
	}
}

const char* CltCEGUIConsole::ChatHistory::getNext()
{
	if (mIndex < (mLineList.size() - 1))
		++mIndex;

	LogDBG("ChatHistory: getNext: [%zu] %s", mIndex, getByIndex(mIndex));
	return getByIndex(mIndex);
}

const char* CltCEGUIConsole::ChatHistory::getPrev()
{
	if (mIndex > 0)
		--mIndex;

	LogDBG("ChatHistory: getPrev: [%zu] %s", mIndex, getByIndex(mIndex));
	return getByIndex(mIndex);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
