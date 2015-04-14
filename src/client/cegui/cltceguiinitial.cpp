/*
 * cltceguiinitial.cpp
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

#include "common/configmgr.h"
#include "common/xmlmgr.h"
#include "common/sha1.h"
#include "common/net/msgs.h"

#include "client/cegui/cltceguimgr.h"
#include "client/content/cltcontentloader.h"
#include "client/content/cltcontentmgr.h"
#include "client/net/cltnetmgr.h"
#include "client/cltmain.h"

#include <CEGUI/Window.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/widgets/Combobox.h>
#include <CEGUI/widgets/Editbox.h>
#include <CEGUI/widgets/ListboxItem.h>
#include <CEGUI/widgets/ListboxTextItem.h>
#include <CEGUI/widgets/MultiColumnList.h>
#include <CEGUI/widgets/MultiLineEditbox.h>
#include <CEGUI/widgets/Spinner.h>
#include <CEGUI/widgets/ProgressBar.h>
#include <CEGUI/widgets/PushButton.h>

#include "cltceguiinitial.h"


/** Helper class to keep the character list with more data than the provided in
 * the list of the join menu (can't hold all data that we may need there)
 */
class CharListHelper
{
public:
	/** Add the character to the list */
	void addCharacter(const string& name,
			  const string& race,
			  const string& gender,
			  const string& playerClass,
			  const string& area) {
		list.push_back(CharEntryHelper(name, race, gender, playerClass, area));
	}

	/** Delete the character */
	void delCharacter(const string& name) {
		for (vector<CharEntryHelper>::iterator it = list.begin(); it != list.end(); ++it) {
			if (name == (*it).charName) {
				list.erase(it);
				return;
			}
		}
	}

	/** Get the race of a character */
	const char* getRaceOf(const string& name) const {
		return getElementWithName(name)->charRace.c_str();
	}
	/** Get the race of a character (initial letter only) */
	const char* getRaceInitialOf(const string& name) const {
		return getElementWithName(name)->charRace.substr(0, 1).c_str();
	}

	/** Get the gender of a character */
	const char* getGenderOf(const string& name) const {
		return getElementWithName(name)->charGender.c_str();
	}
	/** Get the gender of a character (initial letter only) */
	const char* getGenderInitialOf(const string& name) const {
		return getElementWithName(name)->charGender.substr(0, 1).c_str();
	}

	/** Get the area of a character */
	const char* getAreaOf(const string& name) const {
		return getElementWithName(name)->charArea.c_str();
	}

	/** Get the area of a character */
	const char* getClassOf(const string& name) const {
		return getElementWithName(name)->charClass.c_str();
	}

private:
	/** Structure of info that we want to maintain about each character
	 */
	class CharEntryHelper
	{
	public:
		string charName, charRace, charGender, charClass, charArea;

		CharEntryHelper(const string& n, const string& r, const string& g, const string& c, const string& a) :
			charName(n), charRace(r), charGender(g), charClass(c), charArea(a) { }
	};

	/// List of characters of the player
	std::vector<CharEntryHelper> list;

	/** Get the node of the character based on the name */
	const CharEntryHelper* getElementWithName(const string& name) const {
		for (size_t i = 0; i < list.size(); ++i) {
			if (list[i].charName == name)
				return &list[i];
		}
		return 0;
	}
};


//--------------------------- CltCEGUIInitial ------------------
template <> CltCEGUIInitial* Singleton<CltCEGUIInitial>::INSTANCE = 0;

CltCEGUIInitial::CltCEGUIInitial() :
	mDoContentUpdate(true), mContentUpdateReady(false), mContentListener(0)
{
}

void CltCEGUIInitial::setDoContentUpdate(bool value)
{
	mDoContentUpdate = value;
}

void CltCEGUIInitial::setup()
{
	mPingClient = new PingClient();
	mCharListHelper = new CharListHelper();

	mWinMgr = &CEGUI::WindowManager::getSingleton();

	// fill the windows with the initial data
	Connect_LoadConnectData();
	Login_LoadLoginData();
	NewChar_LoadNewCharData();

	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	// initial visibility
	root->getChild("Background")->setVisible(false);
	root->getChild("Connect")->setVisible(false); //set to false for now
	root->getChild("Login")->setVisible(false);
	root->getChild("NewUser")->setVisible(false);
	root->getChild("Join")->setVisible(false);
	root->getChild("NewChar")->setVisible(false);
	root->getChild("Download")->setVisible(false);
	root->getChild("LoadingBanner")->setVisible(false);

	// subscribe events
	CEGUI_EVENT("Connect/Quit",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::Connect_Quit);
	CEGUI_EVENT("Connect/Options",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::Connect_Options);
	CEGUI_EVENT("Connect/Connect",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::Connect_Connect);

	CEGUI_EVENT("Login/Quit",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::Login_Quit);
	CEGUI_EVENT("Login/NewUser",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::Login_NewUser);
	CEGUI_EVENT("Login/Login",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::Login_Login);

	CEGUI_EVENT("NewUser/Cancel",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::NewUser_Cancel);
	CEGUI_EVENT("NewUser/Create",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::NewUser_Create);

	CEGUI_EVENT("Join/CharList",
		    CEGUI::MultiColumnList::EventSelectionChanged,
		    CltCEGUIInitial::Join_OnSelectionChanged);
	CEGUI_EVENT("Join/Quit",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::Join_Quit);
	CEGUI_EVENT("Join/Join",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::Join_Join);
	CEGUI_EVENT("Join/NewChar",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::Join_NewChar);
	CEGUI_EVENT("Join/DelChar",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::Join_DelChar);

	CEGUI_EVENT("NewChar/Cancel",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::NewChar_Cancel);
	CEGUI_EVENT("NewChar/Create",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIInitial::NewChar_Create);
	CEGUI_EVENT("NewChar/Ab_Str",
		    CEGUI::Spinner::EventValueChanged,
		    CltCEGUIInitial::NewChar_SpinnerValueChanged);
	CEGUI_EVENT("NewChar/Ab_Con",
		    CEGUI::Spinner::EventValueChanged,
		    CltCEGUIInitial::NewChar_SpinnerValueChanged);
	CEGUI_EVENT("NewChar/Ab_Dex",
		    CEGUI::Spinner::EventValueChanged,
		    CltCEGUIInitial::NewChar_SpinnerValueChanged);
	CEGUI_EVENT("NewChar/Ab_Int",
		    CEGUI::Spinner::EventValueChanged,
		    CltCEGUIInitial::NewChar_SpinnerValueChanged);
	CEGUI_EVENT("NewChar/Ab_Wis",
		    CEGUI::Spinner::EventValueChanged,
		    CltCEGUIInitial::NewChar_SpinnerValueChanged);
	CEGUI_EVENT("NewChar/Ab_Cha",
		    CEGUI::Spinner::EventValueChanged,
		    CltCEGUIInitial::NewChar_SpinnerValueChanged);
	CEGUI_EVENT("NewChar/CharRace",
		    CEGUI::Combobox::EventListSelectionAccepted,
		    CltCEGUIInitial::NewChar_ComboboxSelectionAccepted);
	CEGUI_EVENT("NewChar/CharGender",
		    CEGUI::Combobox::EventListSelectionAccepted,
		    CltCEGUIInitial::NewChar_ComboboxSelectionAccepted);

	if (mDoContentUpdate) {
		// disabling Login button until content downloaded, we need to
		// have the xml config files parsed for many functions in the
		// initial menus
		root->getChild("Login/Login")->disable();
	}
}

CltCEGUIInitial::~CltCEGUIInitial()
{
}

void CltCEGUIInitial::Connect_LoadConnectData()
{
	// get the server list window in CEGUI
	CEGUI::MultiColumnList* serverList = static_cast<CEGUI::MultiColumnList*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Connect/ServerList"));
	PERM_ASSERT(serverList);
	serverList->setSelectionMode(CEGUI::MultiColumnList::RowSingle);

	// get the root node of the XML file
	const XMLNode* rootXMLNode = XMLMgr::instance().loadXMLFile(SERVERLIST_FILE);
        if (!rootXMLNode) {
                LogERR("File %s not found (or empty)", SERVERLIST_FILE);
                return;
        }

	// iterate with the entries in the file to fill the window
	const XMLNode* childXMLNode = rootXMLNode->getFirstChild();
        while (childXMLNode) {
		string serverName = childXMLNode->getAttrValueAsStr("name");
		string serverPort = childXMLNode->getAttrValueAsStr("port");
		int serverPingPort = childXMLNode->getAttrValueAsInt("pingport");
		/* LogDBG("Got server from list: '%s:%s/%d'",
		   serverName.c_str(), serverPort.c_str(), serverPingPort); */

		unsigned int row = serverList->addRow();
		CEGUI::ListboxTextItem* server_name = new CEGUI::ListboxTextItem(serverName.c_str(), 0);
		CEGUI::ListboxTextItem* server_port = new CEGUI::ListboxTextItem(serverPort.c_str(), 0);
		CEGUI::ListboxTextItem* server_ping = new CEGUI::ListboxTextItem("off-line", 0);
		// we have to set up this in order to get them highlighted
		server_name->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
		server_port->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
		server_ping->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
		serverList->setItem(server_name, 0, row);
		serverList->setItem(server_port, 1, row);
		serverList->setItem(server_ping, 2, row);

		mPingClient->addServer(serverName.c_str(), serverPingPort, row);

		const XMLNode* aux = childXMLNode;
		childXMLNode = childXMLNode->getNextSibling();
		delete aux;
        }
	delete rootXMLNode;

	// setting by default the first row as active
	serverList->clearAllSelections();
	CEGUI::MCLGridRef position(0, 0);
	serverList->setItemSelectState(position, true);

	// disabling Options button until we do something with them
	static_cast<CEGUI::PushButton*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Connect/Options"))->disable();
}

bool CltCEGUIInitial::Connect_Quit(const CEGUI::EventArgs& e)
{
	CltMain::quit();
	return true;
}

bool CltCEGUIInitial::Connect_Options(const CEGUI::EventArgs& e)
{
	/// \todo mafm: implement
	return true;
}

bool CltCEGUIInitial::Connect_Connect(const CEGUI::EventArgs& e)
{
	CEGUI::MultiColumnList* serverList = static_cast<CEGUI::MultiColumnList*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Connect/ServerList"));
	PERM_ASSERT(serverList);

	if (serverList->getSelectedCount() == 0) {
		LogERR("Cannot connect: No server selected");
		CltCEGUIMgr::instance().Notification_DisplayMessage("No server selected");
		return true;
	}

	// disable while processing
	static_cast<CEGUI::PushButton*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Connect/Connect"))->disable();

	// get selected server
	CEGUI::ListboxItem* address = serverList->getFirstSelectedItem();
	CEGUI::ListboxItem* port = serverList->getNextSelected(address);
	bool connected = CltNetworkMgr::instance().connectToServer(address->getText().c_str(),
								   atoi(port->getText().c_str()));
	if (!connected) {
		LogERR("Cannot connect to server");
		CltCEGUIMgr::instance().Notification_DisplayMessage("Cannot connect to server");
		// enable again
		static_cast<CEGUI::PushButton*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Connect/Connect"))->enable();
	} else {
		// send initial message
		MsgConnect msg;
		CltNetworkMgr::instance().sendToServer(msg);
	}

	return true;
}

void CltCEGUIInitial::Connect_to_Login(const char* uptime,
				       const char* players,
				       const char* users,
				       const char* chars)
{
	// delete unneeded classes
	delete mPingClient; mPingClient = 0;

	// content update
	if (mDoContentUpdate) {
		LogNTC("Sending query for content update");
		Download_RaiseWindow();
		CltContentMgr::instance().sendUpdateQuery();
	}

	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	// change menu
	root->getChild("Initial/Login")->setVisible(true);
	root->getChild("Initial/Connect")->setVisible(false);

	// fill values of new menus
	string text;
	text = string("Uptime: ") + uptime;
	root->getChild("Initial/Login/Uptime_Lbl")->setText(text.c_str());

	text = string("Players on-line: ") + players;
	root->getChild("Initial/Login/Players_Lbl")->setText(text.c_str());

	text = string("Total Users: ") + users;
	root->getChild("Initial/Login/TotalUsers_Lbl")->setText(text.c_str());

	text = string("Total Chars: ") + chars;
	root->getChild("Initial/Login/TotalChars_Lbl")->setText(text.c_str());
}

void CltCEGUIInitial::Connect_ProcessPingReplies(float timestamp)
{
	if (mPingClient) {
		// collect the data available
		mPingClient->processPingReplies();
		list<PingClient::PingServerEntry> pingReplies;
		mPingClient->collectPingReplyData(pingReplies);
		for (list<PingClient::PingServerEntry>::iterator it = pingReplies.begin();
		     it != pingReplies.end(); ++it) {
			if (it->timestamp != 0) {
				Connect_UpdatePingReply(it->id,
							StrFmt("%.0fms",
							       ((timestamp*1000.0f) - it->timestamp)));
				// manipulate timestamp to mark it as "already
				// updated"
				it->timestamp = 0;
			}
			if (it->socket == -1) {
				// socket not valid anymore
				Connect_UpdatePingReply(it->id,
							"off-line");
			}
		}
	}
}

void CltCEGUIInitial::Connect_SendPings(float timestamp)
{
	if (mPingClient) {
		// send a new set of ping requests
		uint32_t ms = static_cast<uint32_t>(timestamp)*1000;
		mPingClient->sendPings(ms);
	}
}

void CltCEGUIInitial::Connect_UpdatePingReply(uint32_t entry, const char* text)
{
	CEGUI::MultiColumnList* serverList =
		static_cast<CEGUI::MultiColumnList*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Connect/ServerList"));
	PERM_ASSERT(serverList);

	CEGUI::MCLGridRef position(entry, 2);
	CEGUI::ListboxTextItem* item = static_cast<CEGUI::ListboxTextItem*>
		(serverList->getItemAtGridReference(position));
	if (item) {
		item->setText(text);
		// mark the list to be updated, won't do it otherwise
		serverList->handleUpdatedItemData();
	} else {
		LogWRN("Ping reply from unknown server (entry %u, text %s)",
		       entry, text);
	}
}

void CltCEGUIInitial::Login_LoadLoginData()
{
	// mafm: if we use the stored password, stored as sha1, we mark it using
	// a fake value to be able to do the right thing later
	string storedUsername = ConfigMgr::instance().getConfigVar("Client.Settings.User", "");
	string storedPassword = ConfigMgr::instance().getConfigVar("Client.Settings.Password", "");
	if (storedUsername != "") {
		CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Login/Username")->setText(storedUsername);
	}
	if (storedPassword != "") {
		storedPassword = "<stored>";
		CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Login/Password")->setText(storedPassword);
	}
}

bool CltCEGUIInitial::Login_Quit(const CEGUI::EventArgs& e)
{
	CltMain::quit();
	return true;
}

bool CltCEGUIInitial::Login_NewUser(const CEGUI::EventArgs& e)
{
	Login_to_NewUser();
	return true;
}

bool CltCEGUIInitial::Login_Login(const CEGUI::EventArgs& e)
{
        CEGUI::Editbox* username = static_cast<CEGUI::Editbox*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Login/Username"));
        CEGUI::Editbox* password = static_cast<CEGUI::Editbox*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Login/Password"));

        if (username->getText().size() == 0
	    || password->getText().size() == 0) {
                LogERR("Username or password values not filled in");
		CltCEGUIMgr::instance().Notification_DisplayMessage("Username or password values not filled in");
                return true;
        }

	// mafm: We save the password encoded. When filling the window, if we
	// use the stored password we use the key "<stored>", and we set the
	// encoded password field of the message as the value stored in the
	// file. If the password was changed, we send the new password encoded
	// and save this encoded form. We need to do this because the password
	// is encoded, so there's no way to decode it to put it in the window.

        // saving login data
	bool result = ConfigMgr::instance().storeConfigVar("Client.Settings.User",
							   username->getText().c_str());
	if (!result)
		LogERR("Couldn't save login information in cfg file");

	MsgLogin msg;
	msg.username = username->getText().c_str();

	if (password->getText() == "<stored>") {
		LogDBG("Using stored pwd for login message");
		msg.pw_md5sum = ConfigMgr::instance().getConfigVar("Client.Settings.Password", "");
		PERM_ASSERT(msg.pw_md5sum != "");
	} else {
		LogDBG("Using new provided pwd for login message");
		SHA1::encode(password->getText().c_str(), msg.pw_md5sum);

		// new password, save it to the config file as hash
		bool resultSave = ConfigMgr::instance().storeConfigVar("Client.Settings.Password",
								       msg.pw_md5sum.c_str());
		if (!resultSave)
			LogERR("Couldn't save login information in cfg file");
	}

	// disable while processing
	static_cast<CEGUI::PushButton*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Login/Login"))->disable();
	static_cast<CEGUI::PushButton*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Login/NewUser"))->disable();

	CltNetworkMgr::instance().sendToServer(msg);

        return true;
}

void CltCEGUIInitial::Login_to_NewUser()
{
	// switch window shown
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/NewUser")->setVisible(true);
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Login")->setVisible(false);
}

void CltCEGUIInitial::Login_Failed()
{
	// enable buttons again
	static_cast<CEGUI::PushButton*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Login/Login"))->enable();
	static_cast<CEGUI::PushButton*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Login/NewUser"))->enable();
}


bool CltCEGUIInitial::NewUser_Cancel(const CEGUI::EventArgs& e)
{
	NewUser_to_Login();
	return true;
}

void CltCEGUIInitial::NewUser_to_Login()
{
	// switch window shown
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Login")->setVisible(true);
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/NewUser")->setVisible(false);
}

bool CltCEGUIInitial::NewUser_Create(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
        CEGUI::Editbox* username = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewUser/Username"));
        CEGUI::Editbox* password = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewUser/Password"));
        CEGUI::Editbox* password_rep = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewUser/PasswordRep"));
        CEGUI::Editbox* email = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewUser/Email"));
        CEGUI::Editbox* realname = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewUser/RealName"));
        PERM_ASSERT(username && password && email && realname);

        if (username->getText().size() == 0
	    || password->getText().size() == 0
	    || password->getText() != password_rep->getText()) {
		// we don't process it if there are discrepancies
                LogERR("Username or password values not filled in or password mismatch");
		CltCEGUIMgr::instance().Notification_DisplayMessage("Username or password values not filled in");
                return true;
        }

	// send the message to the server
	MsgNewUser msg;
	msg.username = username->getText().c_str();
	SHA1::encode(password->getText().c_str(), msg.pw_md5sum);
	msg.email = email->getText().c_str();
	msg.realname = realname->getText().c_str();
	CltNetworkMgr::instance().sendToServer(msg);

        /* LogDBG("New user data: u(%s) p(%s) e(%s) r(%r)",
	msg.username.c_str(), msg.pw_md5sum.c_str(), msg.email.c_str(), msg.realname.c_str()); */

	return true;
}


void CltCEGUIInitial::Login_to_Join(vector<MsgLoginReply::CharacterListEntry>& charListEntries)
{
	CltContentLoader::instance().loadCfgFiles();
	PERM_ASSERT(CltContentLoader::instance().isConfigLoaded());

	CEGUI::MultiColumnList* charList = static_cast<CEGUI::MultiColumnList*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Join/CharList"));
	PERM_ASSERT(charList);
	charList->setSelectionMode(CEGUI::MultiColumnList::RowSingle);

	while (!charListEntries.empty()) {
		MsgLoginReply::CharacterListEntry entry = charListEntries.back();
		charListEntries.pop_back();
		string racePretty = CltContentLoader::instance().getInfoCharacter(entry.race)->getPrettyName();
		mCharListHelper->addCharacter(entry.name, racePretty, entry.gender, 
										entry.playerClass, entry.area);

		string raceLetter = mCharListHelper->getRaceInitialOf(entry.name);
		string genderLetter = mCharListHelper->getGenderInitialOf(entry.name);
		//string playerClass = mCharListHelper->getClassOf(entry.name);

		unsigned int row = charList->addRow();
		CEGUI::ListboxTextItem* charName = new CEGUI::ListboxTextItem(entry.name.c_str(), 0);
		CEGUI::ListboxTextItem* charRace = new CEGUI::ListboxTextItem(raceLetter.c_str(), 0);
		CEGUI::ListboxTextItem* charGender = new CEGUI::ListboxTextItem(genderLetter.c_str(), 0);
		// we have to set up this in order to get them
		// highlighted
		charName->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
		charRace->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
		charGender->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
		charList->setItem(charName, 0, row);
		charList->setItem(charRace, 1, row);
		charList->setItem(charGender, 2, row);
	}

	// sorting by character name
	charList->setSortColumnByID(0);
	charList->setSortDirection(CEGUI::ListHeaderSegment::Descending);

	// setting by default the last char used as active
	CEGUI::String charName = ConfigMgr::instance().getConfigVar("Client.Settings.LastCharacter", "");
	charList->clearAllSelections();
	if (charName.size() != 0) {
		CEGUI::ListboxItem* lastChar = charList->findListItemWithText(charName, 0);
		if (lastChar) {
			// select the row in the charlist
			charList->setItemSelectState(lastChar, true);
		} else {
			LogNTC("Last character used not found in saved configuration");
		}
	}

	// now switch the window shown
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Join")->setVisible(true);
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Login")->setVisible(false);
}

bool CltCEGUIInitial::Join_FillNewChar(string charname,
				       string race,
				       string gender,
					   string playerClass,
				       string area)
{
	CEGUI::MultiColumnList* charList = static_cast<CEGUI::MultiColumnList*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Join/CharList"));
	PERM_ASSERT(charList);

	mCharListHelper->addCharacter(charname, race, gender, playerClass, area);
	if (charname.empty() || race.empty() || gender.empty() 
		|| playerClass.empty() || area.empty() )
		LogWRN("Some empty data when adding newly created character: '%s' '%s' '%s' '%s'",
		       charname.c_str(), race.c_str(), gender.c_str(), area.c_str());

	string raceLetter = mCharListHelper->getRaceInitialOf(charname);
	string genderLetter = mCharListHelper->getGenderInitialOf(charname);

	unsigned int row = charList->addRow();
	CEGUI::ListboxTextItem* charName = new CEGUI::ListboxTextItem(charname.c_str(), 0);
	CEGUI::ListboxTextItem* charRace = new CEGUI::ListboxTextItem(raceLetter.c_str(), 0);
	CEGUI::ListboxTextItem* charGender = new CEGUI::ListboxTextItem(genderLetter.c_str(), 0);
	// we have to set up this in order to get them highlighted
	charName->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
	charRace->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
	charGender->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
	charList->setItem(charName, 0, row);
	charList->setItem(charRace, 1, row);
	charList->setItem(charGender, 2, row);

	// sorting by character name
	charList->setSortColumnByID(0);
	charList->setSortDirection(CEGUI::ListHeaderSegment::Descending);

	// setting by default the last char used as active
	charList->clearAllSelections();
	CEGUI::ListboxItem* newChar = charList->findListItemWithText(charName->getText(), 0);
	if (newChar)
		charList->setItemSelectState(newChar, true);
	else
		LogWRN("New character created not found in list");

	return true;
}

bool CltCEGUIInitial::Join_RemoveDelChar(std::string charname)
{
	CEGUI::MultiColumnList* charList = static_cast<CEGUI::MultiColumnList*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Join/CharList"));
	PERM_ASSERT(charList);

	mCharListHelper->delCharacter(charname);

	CEGUI::ListboxItem* delChar = charList->findListItemWithText(charname, 0);
	if (delChar) {
		for (size_t i = 0; i < charList->getRowCount(); ++i) {
			bool found = charList->isListboxItemInRow(delChar, i);
			if (found) {
				charList->removeRow(i);
				return true;
			}
		}
	}

	// if we are here there's something wrong
	LogERR("Deleted character not found in list");

	return true;
}

bool CltCEGUIInitial::Join_OnSelectionChanged(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::MultiColumnList* charList = static_cast<CEGUI::MultiColumnList*>
		(root->getChild("Initial/Join/CharList"));
	PERM_ASSERT(charList);

	// selected item (if nothing selected, we leave here silently, we have
	// to wait until somethingis selected)
	CEGUI::ListboxTextItem* selCharName = static_cast<CEGUI::ListboxTextItem*>
		(charList->getFirstSelectedItem());
	if (!selCharName)
		return true;

	try {
		// name
		string charName = selCharName->getText().c_str();
		if (charName.empty())
			throw "Empty name";
		string name = string(".: ") + charName + string(" :.");;
		root->getChild("Initial/Join/CharName_Lbl")->setText(name.c_str());

		// race
		string race = mCharListHelper->getRaceOf(charName);
		if (race.empty())
			throw "Empty race";
		string textRace = string("Race: ") + race;
		root->getChild("Initial/Join/CharRace_Lbl")->setText(textRace.c_str());

		// gender
		string gender = mCharListHelper->getGenderOf(charName);
		if (gender.empty())
			throw "Empty gender";
		string textGender = string("Gender: ") + gender;
		root->getChild("Initial/Join/CharGender_Lbl")->setText(textGender.c_str());

		// area
		string area = mCharListHelper->getAreaOf(charName);
		if (area.empty())
			throw "Empty area";
		string textArea = string("Area: ") + area;
		root->getChild("Initial/Join/CharArea_Lbl")->setText(textArea.c_str());

		// image
		string imageProperty = StrFmt("set:RacePictures image:%s_%s",
					      race.c_str(), gender.c_str());
		root->getChild("Initial/Join/CharImage")->setProperty("Image", imageProperty);

	} catch (const char* data) {
		LogWRN("Couldn't get data from the selected character: %s", data);
		return true;
	}

	return true;
}


bool CltCEGUIInitial::Join_Quit(const CEGUI::EventArgs& e)
{
	CltMain::quit();
	return true;
}

bool CltCEGUIInitial::Join_Join(const CEGUI::EventArgs& e)
{
	CEGUI::MultiColumnList* charList = static_cast<CEGUI::MultiColumnList*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Join/CharList"));
	PERM_ASSERT(charList);

	if (charList->getSelectedCount() == 0) {
		LogERR("No character selected");
		CltCEGUIMgr::instance().Notification_DisplayMessage("No character selected");
		return true;
	}

	// sending join request
	CEGUI::ListboxItem* item = charList->getFirstSelectedItem();
	PERM_ASSERT(item);
	MsgJoin msg;
	msg.charname = item->getText().c_str();
	CltNetworkMgr::instance().sendToServer(msg);

        // saving character data
	bool result = ConfigMgr::instance().storeConfigVar("Client.Settings.LastCharacter",
							   item->getText().c_str());
	if (!result)
		LogERR("Couldn't save login information in cfg file");

	// set face image in PlayerStats window
	string imageProperty = StrFmt("set:RacePictures image:%s_%s-face",
				      mCharListHelper->getRaceOf(msg.charname),
				      mCharListHelper->getGenderOf(msg.charname));
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("InGame/PlayerStats/Picture")->setProperty("Image", imageProperty);

	Join_to_LoadingGame();
	return true;
}

bool CltCEGUIInitial::Join_NewChar(const CEGUI::EventArgs& e)
{
	Join_to_NewChar();
	return true;
}

void CltCEGUIInitial::Join_to_NewChar()
{
	// switch window shown
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/NewChar")->setVisible(true);
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Join")->setVisible(false);
}

void CltCEGUIInitial::Join_to_LoadingGame()
{
	// switch window shown
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/LoadingBanner")->setVisible(true);
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Join")->setVisible(false);
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Download")->setVisible(false);
}

void CltCEGUIInitial::LoadingGame_to_Game()
{
	// delete unneeded
	delete mCharListHelper; mCharListHelper = 0;

	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	// switch window shown
	root->getChild("Initial/LoadingBanner")->setVisible(false);
	root->getChild("Initial/Background")->setVisible(false);

	// destroy windows
	mWinMgr->destroyWindow(root->getChild("Initial/Background"));
	mWinMgr->destroyWindow(root->getChild("Initial/Connect"));
	mWinMgr->destroyWindow(root->getChild("Initial/Login"));
	mWinMgr->destroyWindow(root->getChild("Initial/NewUser"));
	mWinMgr->destroyWindow(root->getChild("Initial/Join"));
	mWinMgr->destroyWindow(root->getChild("Initial/NewChar"));
	mWinMgr->destroyWindow(root->getChild("Initial/Download"));
	mWinMgr->destroyWindow(root->getChild("Initial/LoadingBanner"));
	mWinMgr->cleanDeadPool();
}

bool CltCEGUIInitial::Join_DelChar(const CEGUI::EventArgs& e)
{
	CEGUI::MultiColumnList* charList = static_cast<CEGUI::MultiColumnList*>
		(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Join/CharList"));
	PERM_ASSERT(charList);

	if (charList->getSelectedCount() == 0) {
		LogERR("No character selected");
		CltCEGUIMgr::instance().Notification_DisplayMessage("No character selected");
		return true;
	}

	CEGUI::ListboxItem* item = charList->getFirstSelectedItem();
	PERM_ASSERT(item);

	/// \todo mafm: add missing confirmation dialog

	MsgDelChar msg;
	msg.charname = item->getText().c_str();
	CltNetworkMgr::instance().sendToServer(msg);

	return true;
}


void CltCEGUIInitial::NewChar_LoadNewCharData()
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
        CEGUI::Combobox* raceBox = static_cast<CEGUI::Combobox*>
		(root->getChild("NewChar/CharRace"));
        CEGUI::Combobox* genderBox = static_cast<CEGUI::Combobox*>
		(root->getChild("NewChar/CharGender"));
        CEGUI::Combobox* classBox = static_cast<CEGUI::Combobox*>
		(root->getChild("NewChar/CharClass"));
        PERM_ASSERT(raceBox && genderBox && classBox);

	CEGUI::ListboxTextItem* raceD = new CEGUI::ListboxTextItem("Dwarf", 0);
	CEGUI::ListboxTextItem* raceE = new CEGUI::ListboxTextItem("Elf", 0);
	CEGUI::ListboxTextItem* raceH = new CEGUI::ListboxTextItem("Human", 0);
	// we have to set up this in order to get them highlighted
	raceD->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
	raceE->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
	raceH->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
	raceBox->addItem(raceD);
	raceBox->addItem(raceE);
	raceBox->addItem(raceH);
	raceBox->clearAllSelections();
	raceBox->setText("Race");
	raceBox->setSingleClickEnabled(true);

	CEGUI::ListboxTextItem* genderF = new CEGUI::ListboxTextItem("f", 0);
	CEGUI::ListboxTextItem* genderM = new CEGUI::ListboxTextItem("m", 0);
	// we have to set up this in order to get them highlighted
	genderF->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
	genderM->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
	genderBox->addItem(genderF);
	genderBox->addItem(genderM);
	genderBox->clearAllSelections();
	genderBox->setText("G");
	genderBox->setSingleClickEnabled(true);

	CEGUI::ListboxTextItem* classFighter = new CEGUI::ListboxTextItem("fighter", 0);
	CEGUI::ListboxTextItem* classSorcerer = new CEGUI::ListboxTextItem("sorcerer", 0);
	// we have to set up this in order to get them highlighted
	classFighter->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
	classSorcerer->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
	classBox->addItem(classFighter);
	classBox->addItem(classSorcerer);
	classBox->clearAllSelections();
	classBox->setText("Class");
	classBox->setSingleClickEnabled(true);

	// points to use when creating a new char
	pointsNewChar = 0;

	root->getChild("NewChar/Points")->setText(StrFmt("%d", pointsNewChar));
}

bool CltCEGUIInitial::NewChar_Cancel(const CEGUI::EventArgs& e)
{
	NewChar_to_Join();
	return true;
}

void CltCEGUIInitial::NewChar_to_Join()
{
	// switch window shown
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Join")->setVisible(true);
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/NewChar")->setVisible(false);
}

void CltCEGUIInitial::NewChar_RecalculateBonusPoints(const char* race)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Editbox* bonus_str = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Str"));
	CEGUI::Editbox* bonus_con = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Con"));
	CEGUI::Editbox* bonus_dex = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Dex"));
	CEGUI::Editbox* bonus_int = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Int"));
	CEGUI::Editbox* bonus_wis = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Wis"));
	CEGUI::Editbox* bonus_cha = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Cha"));
	PERM_ASSERT(bonus_str && bonus_con && bonus_dex
		       && bonus_int && bonus_wis && bonus_cha);

	const ContentInfoCharacter* charInfo = CltContentLoader::instance().getInfoCharacter(race);
	bonus_str->setText(StrFmt("%d", charInfo->getBonusStr()));
	bonus_con->setText(StrFmt("%d", charInfo->getBonusCon()));
	bonus_dex->setText(StrFmt("%d", charInfo->getBonusDex()));
	bonus_int->setText(StrFmt("%d", charInfo->getBonusInt()));
	bonus_wis->setText(StrFmt("%d", charInfo->getBonusWis()));
	bonus_cha->setText(StrFmt("%d", charInfo->getBonusCha()));

	// recalculating total
	NewChar_RecalculateTotalPoints();
}

void CltCEGUIInitial::NewChar_RecalculateTotalPoints()
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
        CEGUI::Spinner* ab_str = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Str"));
        CEGUI::Spinner* ab_con = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Con"));
        CEGUI::Spinner* ab_dex = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Dex"));
        CEGUI::Spinner* ab_int = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Int"));
        CEGUI::Spinner* ab_wis = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Wis"));
        CEGUI::Spinner* ab_cha = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Cha"));
        PERM_ASSERT(ab_str && ab_con && ab_dex && ab_int && ab_wis && ab_cha);

	CEGUI::Editbox* bonus_str = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Str"));
	CEGUI::Editbox* bonus_con = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Con"));
	CEGUI::Editbox* bonus_dex = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Dex"));
	CEGUI::Editbox* bonus_int = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Int"));
	CEGUI::Editbox* bonus_wis = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Wis"));
	CEGUI::Editbox* bonus_cha = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Bonus_Cha"));
	PERM_ASSERT(bonus_str && bonus_con && bonus_dex
		       && bonus_int && bonus_wis && bonus_cha);

	CEGUI::Editbox* total_str = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Total_Str"));
	CEGUI::Editbox* total_con = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Total_Con"));
	CEGUI::Editbox* total_dex = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Total_Dex"));
	CEGUI::Editbox* total_int = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Total_Int"));
	CEGUI::Editbox* total_wis = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Total_Wis"));
	CEGUI::Editbox* total_cha = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/Total_Cha"));
	PERM_ASSERT(total_str && total_con && total_dex
		       && total_int && total_wis && total_cha);

	total_str->setText(StrFmt("%d", static_cast<int>(ab_str->getCurrentValue()) + atoi(bonus_str->getText().c_str())));
	total_con->setText(StrFmt("%d", static_cast<int>(ab_con->getCurrentValue()) + atoi(bonus_con->getText().c_str())));
	total_dex->setText(StrFmt("%d", static_cast<int>(ab_dex->getCurrentValue()) + atoi(bonus_dex->getText().c_str())));
	total_int->setText(StrFmt("%d", static_cast<int>(ab_int->getCurrentValue()) + atoi(bonus_int->getText().c_str())));
	total_wis->setText(StrFmt("%d", static_cast<int>(ab_wis->getCurrentValue()) + atoi(bonus_wis->getText().c_str())));
	total_cha->setText(StrFmt("%d", static_cast<int>(ab_cha->getCurrentValue()) + atoi(bonus_cha->getText().c_str())));
}

bool CltCEGUIInitial::NewChar_SpinnerValueChanged(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
        CEGUI::Spinner* ab_str = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Str"));
        CEGUI::Spinner* ab_con = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Con"));
        CEGUI::Spinner* ab_dex = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Dex"));
        CEGUI::Spinner* ab_int = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Int"));
        CEGUI::Spinner* ab_wis = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Wis"));
        CEGUI::Spinner* ab_cha = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Cha"));
        PERM_ASSERT(ab_str && ab_con && ab_dex && ab_int && ab_wis && ab_cha);

	int totalValue = 0;
	totalValue += static_cast<uint32_t>(ab_str->getCurrentValue());
	totalValue += static_cast<uint32_t>(ab_con->getCurrentValue());
	totalValue += static_cast<uint32_t>(ab_dex->getCurrentValue());
	totalValue += static_cast<uint32_t>(ab_int->getCurrentValue());
	totalValue += static_cast<uint32_t>(ab_wis->getCurrentValue());
	totalValue += static_cast<uint32_t>(ab_cha->getCurrentValue());

	// mafm: This reverts the action if total value is bigger than total
	// points permitted, it's a bit tricky but yet cleaner than manually
	// controlling all clicks for all spinners
	if ((totalValue - 78) > pointsNewChar) {
		CEGUI::Spinner* spinner = static_cast<CEGUI::Spinner*>
			(static_cast<const CEGUI::WindowEventArgs&>(e).window);
		float val = spinner->getCurrentValue();
		spinner->setCurrentValue(val - 1.0f);
		return true;
	}

	root->getChild("NewChar/Points")->setText(StrFmt("%d",
								    pointsNewChar - (totalValue - 78)));

	// recalculating total
	NewChar_RecalculateTotalPoints();

	return true;
}

bool CltCEGUIInitial::NewChar_ComboboxSelectionAccepted(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
        CEGUI::Combobox* raceBox = static_cast<CEGUI::Combobox*>
		(root->getChild("NewChar/CharRace"));
        CEGUI::Combobox* genderBox = static_cast<CEGUI::Combobox*>
		(root->getChild("Initial/NewChar/CharGender"));
        PERM_ASSERT(raceBox && genderBox );

	string race = raceBox->getText().c_str();
	string gender = genderBox->getText().c_str();

	if (race != "Race") {
		// If the race is set, recalculate the bonus points
		NewChar_RecalculateBonusPoints(race.c_str());

		// if we have race and gender we can put the image
		if (gender != "G") {
			string imageProperty = StrFmt("set:RacePictures image:%s_%s",
						      race.c_str(), gender.c_str());
			root->getChild("Initial/NewChar/CharImage")->setProperty("Image",
										     imageProperty);
		}
	}

	return true;
}

bool CltCEGUIInitial::NewChar_Create(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
        CEGUI::Editbox* charName = static_cast<CEGUI::Editbox*>
		(root->getChild("Initial/NewChar/CharName"));
        CEGUI::Combobox* charRace = static_cast<CEGUI::Combobox*>
		(root->getChild("Initial/NewChar/CharRace"));
        CEGUI::Combobox* charGender = static_cast<CEGUI::Combobox*>
		(root->getChild("Initial/NewChar/CharGender"));
        CEGUI::Combobox* charClass = static_cast<CEGUI::Combobox*>
		(root->getChild("Initial/NewChar/CharClass"));
        CEGUI::Spinner* ab_str = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Str"));
        CEGUI::Spinner* ab_con = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Con"));
        CEGUI::Spinner* ab_dex = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Dex"));
        CEGUI::Spinner* ab_int = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Int"));
        CEGUI::Spinner* ab_wis = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Wis"));
        CEGUI::Spinner* ab_cha = static_cast<CEGUI::Spinner*>
		(root->getChild("Initial/NewChar/Ab_Cha"));
        PERM_ASSERT(charName && charRace && charGender
		       && ab_str && ab_con && ab_dex && ab_int && ab_wis && ab_cha);

        if (charName->getText().size() == 0) {
                CltCEGUIMgr::instance().Notification_DisplayMessage("ERROR: Character Name not set");
                return true;
        }

	string race = charRace->getText().c_str();
	string gender = charGender->getText().c_str();
	string playerClass = charClass->getText().c_str();

        if (race == "Race" || gender == "G" || playerClass == "Class" ) {
                CltCEGUIMgr::instance().Notification_DisplayMessage("ERROR: Race and/or gender value not set");
                return true;
        }

	int totalValue = 0;
	totalValue += static_cast<int>(ab_str->getCurrentValue());
	totalValue += static_cast<int>(ab_con->getCurrentValue());
	totalValue += static_cast<int>(ab_dex->getCurrentValue());
	totalValue += static_cast<int>(ab_int->getCurrentValue());
	totalValue += static_cast<int>(ab_wis->getCurrentValue());
	totalValue += static_cast<int>(ab_cha->getCurrentValue());
	if ((totalValue - 78) != 0) {
		CltCEGUIMgr::instance().Notification_DisplayMessage("You still have points to use "
								    "for your character, please expend all them.");
		return true;
	}

	MsgNewChar msg;
	msg.charname = charName->getText().c_str();
	msg.race = CltContentLoader::instance().getInfoCharacter(charRace->getSelectedItem()->getText().c_str())->getName();
	msg.gender = charGender->getSelectedItem()->getText().c_str();
	msg.playerClass = charClass->getSelectedItem()->getText().c_str();

	msg.ab_choice_str = static_cast<uint8_t>(ab_str->getCurrentValue());
	msg.ab_choice_con = static_cast<uint8_t>(ab_con->getCurrentValue());
	msg.ab_choice_dex = static_cast<uint8_t>(ab_dex->getCurrentValue());
	msg.ab_choice_int = static_cast<uint8_t>(ab_int->getCurrentValue());
	msg.ab_choice_wis = static_cast<uint8_t>(ab_wis->getCurrentValue());
	msg.ab_choice_cha = static_cast<uint8_t>(ab_cha->getCurrentValue());
	CltNetworkMgr::instance().sendToServer(msg);

	LogDBG("name:%s race:%s gender:%s str:%u con:%u dex:%u int:%u wis:%u cha:%u",
	       msg.charname.c_str(),
	       msg.race.c_str(),
	       msg.gender.c_str(),
	       msg.ab_choice_str,
	       msg.ab_choice_con,
	       msg.ab_choice_dex,
	       msg.ab_choice_int,
	       msg.ab_choice_wis,
	       msg.ab_choice_cha);

	return true;
}


void CltCEGUIInitial::Download_RaiseWindow()
{
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Download/Filename")->setText("Asking the server for content updates...");

	// switch window shown
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Download")->setVisible(true);

	// content listener
	class LocalContentListener : public CltContentListener
	{
	private:
		CltCEGUIInitial* mCEGUIInitial;
	public:
		LocalContentListener(CltCEGUIInitial* ci) : mCEGUIInitial(ci) { }

		virtual void filePartAdded(const char* filename, float filePct, float totalPct, float avgSpeed) {
			mCEGUIInitial->Download_UpdateFileStats(filename, filePct);
			mCEGUIInitial->Download_UpdateTotalStats(filename, totalPct);
		}

		virtual void updateCompleted(float sizeKB, float seconds, float avgSpeed) {
			mCEGUIInitial->Download_Completed(sizeKB, seconds, avgSpeed);
		}
	};
	mContentListener = new LocalContentListener(this);
	CltContentMgr::instance().addListener(mContentListener);
}

void CltCEGUIInitial::Download_UpdateFileStats(const char* fileName,
					      float pct)
{
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Download/FileNumber_Lbl")->setText(StrFmt("%.1f%%", pct*100.0f));
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Download/Filename")->setText(fileName);

	static_cast<CEGUI::ProgressBar*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Download/FileProgress"))->setProgress(pct);
}

void CltCEGUIInitial::Download_UpdateTotalStats(const char* fileName,
						float pct)
{
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Download/TotalNumber_Lbl")->setText(StrFmt("%.1f%%", pct*100.0f));
	CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Download/Filename")->setText(fileName);

	static_cast<CEGUI::ProgressBar*>(CltCEGUIMgr::instance().getGUIContext()->getRootWindow()->getChild("Initial/Download/TotalProgress"))->setProgress(pct);
}

void CltCEGUIInitial::Download_Completed(float sizeKB, float seconds, float avgSpeed)
{
	string completed = StrFmt("Content update completed, %.02fKB in %0.1fs (%.02f kB/s)",
				  sizeKB, seconds, avgSpeed);

	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	root->getChild("Initial/Download/Filename")->setText(completed.c_str());
	root->getChild("Initial/Download/FileNumber_Lbl")->setText("100%");
	root->getChild("Initial/Download/TotalNumber_Lbl")->setText("100%");

	static_cast<CEGUI::ProgressBar*>(root->getChild("Initial/Download/FileProgress"))->setProgress(1.0f);
	static_cast<CEGUI::ProgressBar*>(root->getChild("Initial/Download/TotalProgress"))->setProgress(1.0f);

	// enabling login button now
	if (mDoContentUpdate) {
		static_cast<CEGUI::PushButton*>(root->getChild("Initial/Login/Login"))->enable();
	}

	mContentUpdateReady = true;
	delete mContentListener; mContentListener = 0;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
