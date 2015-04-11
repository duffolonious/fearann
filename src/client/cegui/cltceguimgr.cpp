/*
 * cltceguimgr.cpp
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

#include <unistd.h>
#include <errno.h>

#include <CEGUI/Colour.h>
#include <CEGUI/Font.h>
#include <CEGUI/FontManager.h>
#include <CEGUI/SchemeManager.h>
#include <CEGUI/Size.h>
#include <CEGUI/Window.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>
#include <CEGUI/widgets/MultiColumnList.h>
#include <CEGUI/widgets/Listbox.h>
#include <CEGUI/widgets/ListboxTextItem.h>
#include <CEGUI/widgets/Editbox.h>
#include <CEGUI/widgets/ListHeaderSegment.h>
#include <CEGUI/widgets/ProgressBar.h>
#include <CEGUI/widgets/PushButton.h>

#include "common/net/msgs.h"

#include "client/cltcamera.h"
#include "client/cltmain.h"
#include "client/net/cltnetmgr.h"
#include "client/action/cltcombatmgr.h"
#include "client/cegui/cltceguiinitial.h"

#include "cltceguimgr.h"

//----------------------- CltCEGUIMgr ----------------------------
template <> CltCEGUIMgr* Singleton<CltCEGUIMgr>::INSTANCE = 0;

CltCEGUIMgr::CltCEGUIMgr() :
	mWinMgr(0), mGUIContext(0), mCameraListener(0)
{
}

CltCEGUIMgr::~CltCEGUIMgr()
{
	CltCameraMgr::instance().removeListener(mCameraListener);
	delete mCameraListener;
}

void CltCEGUIMgr::setup(int screenWidth, int screenHeight)
{
	// initialize CEGUI system
	/* CEGUI::OpenGLRenderer* ceguiRenderer = & */CEGUI::OpenGLRenderer::bootstrapSystem(CEGUI::Sizef(640, 480));
	CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Informative);

	// set CWD to the directory containing CEGUI files, so we don't have the
	// data/client/cegui/ part in the data files
	char cwd[1024];
	if(getcwd(cwd, sizeof(cwd)) == NULL) {
		LogERR("getcwd failed: %s", strerror(errno));
	}
	if(chdir(CEGUI_DATA_DIR)) {
		LogERR("CEGUI chdir failed: %s", strerror(errno));
	}

	// font
	CEGUI::FontManager::getSingleton().createFreeTypeFont("Vera",
								10,
								false,
								"../fonts/vera/Vera.ttf");

	// load and set CEGUI resources
	CEGUI::SchemeManager::getSingleton().createFromFile("fearann.scheme");
	mWinMgr = &CEGUI::WindowManager::getSingleton();
	CEGUI::Window* root = mWinMgr->createWindow("DefaultWindow", "root");
	mGUIContext = &CEGUI::System::getSingleton().getDefaultGUIContext();
	mGUIContext->setDefaultFont("Vera");
	mGUIContext->getMouseCursor().setDefaultImage("FearannLook/MouseArrow");
	mGUIContext->setRootWindow(root);

	// restore directory settings
	if(chdir(cwd)) {
		LogERR("chdir failed: %s", strerror(errno));
	}

	// register events
	CEGUI_EVENT("Dialog/Notification/Close",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::Notification_Close);

	CEGUI_EVENT("Dialog/Quit/Cancel",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::Quit_Cancel);
	CEGUI_EVENT("Dialog/Quit/Quit",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::Quit_Quit);

	CEGUI_EVENT("Dialog/Combat/No",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::Combat_Cancel);
	CEGUI_EVENT("Dialog/Combat/Yes",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::Combat_Accept);

	CEGUI_EVENT("InGame/Panel/Inventory_Btn",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::Panel_InventoryButton);
	CEGUI_EVENT("InGame/Panel/Contacts_Btn",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::Panel_ContactsButton);
	CEGUI_EVENT("InGame/Panel/Quit_Btn",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::Panel_QuitButton);

	CEGUI_EVENT("InGame/ContactList/Add",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::Contacts_Add);

	CEGUI_EVENT("InGame/ContactList/Remove",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::Contacts_Remove);

	CEGUI_EVENT("InGame/PlayerStats/Camera",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIMgr::PlayerStats_Camera);

	// dialogs
	root->getChild("Dialog/Notification")->setVisible(false);
	root->getChild("Dialog/Quit")->setVisible(false);

	// in game windows
	root->getChild("InGame/Console")->setVisible(false);
	root->getChild("InGame/Panel")->setVisible(false);
	root->getChild("InGame/PlayerStats")->setVisible(false);
	root->getChild("InGame/Calendar")->setVisible(false);
	root->getChild("InGame/Minimap")->setVisible(false);
	root->getChild("InGame/ContactList")->setVisible(false);
	root->getChild("InGame/Inventory")->setVisible(false);

	// camera mode name and listener
	PlayerStats_SetCameraModeName(CltCameraMgr::instance().getActiveCameraMode().getName());
	class LocalCameraListener : public CltCameraListener
	{
		virtual void cameraModeChanged(const CltCameraMode* mode) {
			PlayerStats_SetCameraModeName(mode->getName());
		}
	};
	mCameraListener = new LocalCameraListener();
	CltCameraMgr::instance().addListener(mCameraListener);

	// initialize the initial menus
	CltCEGUIInitial::instance().setup();
}

void CltCEGUIMgr::render()
{
	CEGUI::System::getSingleton().renderAllGUIContexts();
}

void CltCEGUIMgr::disableFocus()
{
	CEGUI::Window* window = mGUIContext->getRootWindow()->getActiveChild();
	if (window) {
		window->deactivate();
	}
}

bool CltCEGUIMgr::Notification_DisplayMessage(const char* msg)
{
	mGUIContext->getRootWindow()->getChild("Dialog/Notification/Message")->setText(msg);
	mGUIContext->getRootWindow()->getChild("Dialog/Notification")->setVisible(true);
	return true;
}

bool CltCEGUIMgr::Notification_Close(const CEGUI::EventArgs& e)
{
	mGUIContext->getRootWindow()->getChild("Dialog/Notification")->setVisible(false);
	return true;
}


bool CltCEGUIMgr::Quit_Cancel(const CEGUI::EventArgs& e)
{
	mGUIContext->getRootWindow()->getChild("Dialog/Quit")->setVisible(false);
	return true;
}

bool CltCEGUIMgr::Quit_Quit(const CEGUI::EventArgs& e)
{
	CltMain::quit();
	return true;
}

void CltCEGUIMgr::Combat_DisplayMessage( const char * msg )
{
	mGUIContext->getRootWindow()->getChild("Dialog/Combat/Message")->setText( msg );
	mGUIContext->getRootWindow()->getChild("Dialog/Combat")->setVisible(true);
}

bool CltCEGUIMgr::Combat_Cancel(const CEGUI::EventArgs& e)
{
	mGUIContext->getRootWindow()->getChild("Dialog/Combat")->setVisible(false);
	//Send combat end message.
	CltCombatMgr::instance().combatAction(MsgCombat::END);
	return true;
}

bool CltCEGUIMgr::Combat_Accept(const CEGUI::EventArgs& e)
{
	mGUIContext->getRootWindow()->getChild("Dialog/Combat")->setVisible(false);
	//Send combat accept message.
	CltCombatMgr::instance().combatAction(MsgCombat::ACCEPTED);
	return true;
}

bool CltCEGUIMgr::Panel_InventoryButton(const CEGUI::EventArgs& e)
{
	ToggleWindow_Inventory();
	return true;
}

bool CltCEGUIMgr::Panel_ContactsButton(const CEGUI::EventArgs& e)
{
	CEGUI::Window* contacts = mGUIContext->getRootWindow()->getChild("InGame/ContactList");
	PERM_ASSERT(contacts);

	if (contacts->isVisible())
		contacts->setVisible(false);
	else
		contacts->setVisible(true);

	return true;
}

bool CltCEGUIMgr::Panel_QuitButton(const CEGUI::EventArgs& e)
{
	Quit_DisplayDialog();
	return true;
}


void CltCEGUIMgr::Contacts_AddToList(const char* name, char type, char _status,
				    const char* last_login, const char* comment)
{
	CEGUI::MultiColumnList* contactList = static_cast<CEGUI::MultiColumnList*>
		(mGUIContext->getRootWindow()->getChild("InGame/ContactList/List"));
	PERM_ASSERT(contactList);

	string typeStr = StrFmt("%c", type);
	const char* status = 0;
	CEGUI::Colour col;
	if (_status == 'C') {
		status = "On-line";
		col.setRGB(0.0f, 1.0f, 0.0f);
	} else {
		status = "Off-line";
                col.setRGB(1.0f, 0.0f, 0.0f);
	}

	/** \todo mafm: do something to show this additional info, tooltips
	 * don't seem to work in a cell-based or row fashion...

	string tooltipStr = StrFmt("%s\nType: %s\nStatus: %s\nLast Login: %s\nComment: %s\n",
			  name, typeStr.c_str(), status, last_login, comment);
	CEGUI::String tooltip = tooltipStr.c_str();

	*/
	LogERR("Contact status '%s'", status);

        CEGUI::ListboxTextItem* charType = 0;
        CEGUI::ListboxTextItem* charName = static_cast<CEGUI::ListboxTextItem*>
		(contactList->findListItemWithText(name, 0));
        if (!charName) {
		// if the player is already there (so the status changed) use
		// those elements, otherwise create new ones
		unsigned int row = contactList->addRow();
		charType = new CEGUI::ListboxTextItem(typeStr.c_str(), 0);
		charName = new CEGUI::ListboxTextItem(name, 0);

		// we have to set up this in order to get them highlighted
		charType->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
		charName->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");

		// putting the elements in the list
		contactList->setItem(charType, 0, row);
		contactList->setItem(charName, 1, row);
	} else {
		// we found and got the name item from the list, now get the
		// type...
		unsigned int row = contactList->getItemRowIndex(charName);
		CEGUI::MCLGridRef pos(row, 0);
		charType = static_cast<CEGUI::ListboxTextItem*>
			(contactList->getItemAtGridReference(pos));
	}
	charName->setTextColours(col);

	// sorting by type and then name
	contactList->setSortColumnByID(1);
	contactList->setSortDirection(CEGUI::ListHeaderSegment::Descending);
	contactList->setSortColumnByID(0);
	contactList->setSortDirection(CEGUI::ListHeaderSegment::Descending);

	contactList->clearAllSelections();
}

bool CltCEGUIMgr::Contacts_Remove(const CEGUI::EventArgs& e)
{
	CEGUI::MultiColumnList* contactList = static_cast<CEGUI::MultiColumnList*>
		(mGUIContext->getRootWindow()->getChild("InGame/ContactList"));
	PERM_ASSERT(contactList);

	if (contactList->getSelectedCount() == 0) {
		LogERR("No contact selected");
		Notification_DisplayMessage("No contact selected");
		return true;
	}

	CEGUI::ListboxItem* item = contactList->getFirstSelectedItem();
	PERM_ASSERT(item);

	/// \todo mafm: add missing confirmation dialog

	MsgContactDel msg;
	msg.charname = item->getText().c_str();
	CltNetworkMgr::instance().sendToServer(msg);

	return true;
}

bool CltCEGUIMgr::Contacts_Add(const CEGUI::EventArgs& e)
{
	/// \todo mafm: missing implementation
	return true;

/*
	CEGUI::MultiColumnList* contactList = static_cast<CEGUI::MultiColumnList*>
		(mWinMgr->getWindow("InGame/ContactList"));
	PERM_ASSERT(contactList);

	if (contactList->getSelectedCount() == 0)
	{
		LogERR("No contact selected");
		Notification_DisplayMessage("No contact selected");
		return true;
	}

	CEGUI::ListboxItem* item = contactList->getFirstSelectedItem();
	PERM_ASSERT(item);

	MsgContactAdd msg;
	msg.charname = charname;
	msg.type = type;
	msg.comment = comment;
	CltNetworkMgr::instance().sendToServer(msg);
*/
	return true;
}


bool CltCEGUIMgr::PlayerStats_Camera(const CEGUI::EventArgs& e)
{
	CltCameraMgr::instance().cycleCameraMode();
	return true;
}

void CltCEGUIMgr::PlayerStats_Update(MsgPlayerData* msg)
{
	float health = float(msg->health_cur)/float(msg->health_max);
	dynamic_cast<CEGUI::ProgressBar*>(mGUIContext->getRootWindow()->getChild("InGame/PlayerStats/Health"))->setProgress(health);

	float magic = float(msg->magic_cur)/float(msg->magic_max);
	dynamic_cast<CEGUI::ProgressBar*>(mGUIContext->getRootWindow()->getChild("InGame/PlayerStats/Magic"))->setProgress(magic);

	float stamina = float(msg->stamina)/100.0f;
	dynamic_cast<CEGUI::ProgressBar*>(mGUIContext->getRootWindow()->getChild("InGame/PlayerStats/Stamina"))->setProgress(stamina);

	float load = float(msg->load_cur)/float(msg->load_max);
	dynamic_cast<CEGUI::ProgressBar*>(mGUIContext->getRootWindow()->getChild("InGame/PlayerStats/Load"))->setProgress(load);

	mGUIContext->getRootWindow()->getChild("InGame/PlayerStats/Gold")->setText(StrFmt("G: %d", msg->gold));
}

void CltCEGUIMgr::PlayerStats_SetCameraModeName(const char* name)
{
	CltCEGUIMgr::instance().getGUIContext()
	  ->getRootWindow()->getChild("InGame/PlayerStats/Camera")->setText(name);
}

void CltCEGUIMgr::Calendar_SetTime(const string& time, const string& date, const string& year)
{
	mGUIContext->getRootWindow()->getChild("InGame/Calendar/Time_Lbl")->setText(time.c_str());
	mGUIContext->getRootWindow()->getChild("InGame/Calendar/Date_Lbl")->setText(date.c_str());
	mGUIContext->getRootWindow()->getChild("InGame/Calendar/Year_Lbl")->setText(year.c_str());
}

void CltCEGUIMgr::Calendar_SetMoonPicture(const string& moonPictureName)
{
	string imageProperty = StrFmt("set:InterfaceDecorations image:%s", moonPictureName.c_str());
	mGUIContext->getRootWindow()->getChild("InGame/Calendar/Moon")->setProperty("Image", imageProperty);
}

void CltCEGUIMgr::Quit_DisplayDialog()
{
	// just raising the window
	mGUIContext->getRootWindow()->getChild("Dialog/Quit")->setVisible(true);
}

void CltCEGUIMgr::StartPlaying()
{
	// raising the default windows when entering the game
	mGUIContext->getRootWindow()->getChild("InGame/Console")->setVisible(true);
	mGUIContext->getRootWindow()->getChild("InGame/Panel")->setVisible(true);
	mGUIContext->getRootWindow()->getChild("InGame/PlayerStats")->setVisible(true);
	mGUIContext->getRootWindow()->getChild("InGame/Calendar")->setVisible(true);
	mGUIContext->getRootWindow()->getChild("InGame/Minimap")->setVisible(true);
}

void CltCEGUIMgr::ToggleWindow_Inventory()
{
	CEGUI::Window* inv = mGUIContext->getRootWindow()->getChild("InGame/Inventory");
	PERM_ASSERT(inv);

	if (inv->isVisible())
		inv->setVisible(false);
	else
		inv->setVisible(true);
}

float CltCEGUIMgr::convertXToCEGUI( float norm_x )
{
	return (norm_x+1)/2;
}

float CltCEGUIMgr::convertYToCEGUI( float norm_y )
{
	float y_unified = fabsf( (norm_y+1)/2-1 );
	if ( y_unified < 0 )
		y_unified = 0;
	return y_unified;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
