/*
 * cltceguiactionmenu.cpp
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
#include <CEGUI/widgets/PushButton.h>

#include "common/net/msgs.h"

#include "client/action/cltcombatmgr.h"
#include "client/entity/cltentitybase.h"
#include "client/entity/cltentitymainplayer.h"
#include "client/cltentitymgr.h"
#include "client/cltviewer.h"

#include "cltceguimgr.h"

#include "cltceguidialog.h"
#include "cltceguiactionmenu.h"


//----------------------- CltCEGUIActionMenu ----------------------------
template <> CltCEGUIActionMenu* Singleton<CltCEGUIActionMenu>::INSTANCE = 0;

CltCEGUIActionMenu::CltCEGUIActionMenu()
{
	///\todo: replace with popup menu.
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* menu = root->getChild("InGame/ActionMenu");
    PERM_ASSERT(menu);
	menu->setVisible(false);

	// installing events
	CEGUI_EVENT("InGame/ActionMenu",
		    CEGUI::Window::EventShown,
		    CltCEGUIActionMenu::Event_Shown);
	CEGUI_EVENT("InGame/ActionMenu/InfoAction",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIActionMenu::Info_Action);
	CEGUI_EVENT("InGame/ActionMenu/PickupAction",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIActionMenu::Pickup_Action);
	CEGUI_EVENT("InGame/ActionMenu/FightAction",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIActionMenu::Fight_Action);
	CEGUI_EVENT("InGame/ActionMenu/DialogAction",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIActionMenu::Dialog_Action);
	CEGUI_EVENT("InGame/ActionMenu/TradeAction",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIActionMenu::Trade_Action);
}

CltCEGUIActionMenu::~CltCEGUIActionMenu()
{
}

bool CltCEGUIActionMenu::Event_Shown(const CEGUI::EventArgs& e)
{
	return true;
}

bool CltCEGUIActionMenu::ShowAt( float norm_x, float norm_y)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* menu = root->getChild("InGame/ActionMenu");
    PERM_ASSERT(menu);
	mSelectedEntityID = CltViewer::instance().pick( norm_x, norm_y );
	if ( mSelectedEntityID == 0 )
		menu->setVisible(false);
	else
	{
		CltEntityBase* entity = CltEntityMgr::instance().getEntity( mSelectedEntityID );
		float distance = CltEntityMainPlayer::instance().getDistanceToEntity( mSelectedEntityID );
		string text = StrFmt("%s (%.1f)", entity->getName(),  distance);
		menu->getChild("Target_Lbl")->setText(text.c_str());
		///Change window position to where mouse was clicked
		string x = StrFmt("{%.2f,0}", CltCEGUIMgr::instance().convertXToCEGUI( norm_x ) );
		string y = StrFmt("{%.2f,0}", CltCEGUIMgr::instance().convertYToCEGUI( norm_y )-0.05 );
		menu->setProperty( "UnifiedXPosition", x.c_str() );
		menu->setProperty( "UnifiedYPosition", y.c_str() );
		buildMenu( entity );
		menu->setVisible(true);
	}
	return true;
}

void CltCEGUIActionMenu::buildMenu( CltEntityBase* entity)
{
	/// clear buttons
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* menu = root->getChild("InGame/ActionMenu");
	menu->getChild("PickupAction")->setVisible(false);
	menu->getChild("FightAction")->setVisible(false);
	menu->getChild("DialogAction")->setVisible(false);
	menu->getChild("TradeAction")->setVisible(false);

	/// build info button.
	setupButton( "InGame/ActionMenu/InfoAction", 0);
	if ( entity->className() == std::string("Object") )
	{
		setupButton( "InGame/ActionMenu/PickupAction", 1);
	}
	else if ( entity->className() == std::string("Creature") )
	{
		setupButton( "InGame/ActionMenu/FightAction", 1);
	}
	else if ( entity->className() == std::string("Player") )
	{
		setupButton( "InGame/ActionMenu/FightAction", 1); //fight
		setupButton( "InGame/ActionMenu/TradeAction", 2); //trade
		setupButton( "InGame/ActionMenu/DialogAction", 3); //dialog
	}
}

void CltCEGUIActionMenu::setupButton( const char * buttonStr, uint32_t pos)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* button = root->getChild(buttonStr);
    PERM_ASSERT(button);

	float x_pos = pos * 0.22 + 0.01 ;
	//float y_pos = 0.44 - pos * 0.11 ;
	float y_pos = 0.22 ;
	//LogDBG("x,y: %.2f, %.2f", x_pos, y_pos);
	string x = StrFmt("{%.2f,0}", x_pos );
	string y = StrFmt("{%.2f,0}", y_pos );
	button->setProperty( "UnifiedXPosition", x.c_str() );
	button->setProperty( "UnifiedYPosition", y.c_str() );
	button->setVisible(true);
}

bool CltCEGUIActionMenu::Info_Action(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* menu = root->getChild("InGame/ActionMenu");
	CEGUI::Window* button = root->getChild("InGame/ActionMenu/InfoAction");
    PERM_ASSERT(menu && button);
	if (mSelectedEntityID != 0)
	{
		CltEntityBase* entity = CltEntityMgr::instance().getEntity( mSelectedEntityID );
		LogNTC("Info: %s", entity->className() );
	}
	menu->setVisible(false);
	return true;
}

bool CltCEGUIActionMenu::Pickup_Action(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* menu = root->getChild("InGame/ActionMenu");
	CEGUI::Window* button = root->getChild("InGame/ActionMenu/PickupAction");
    PERM_ASSERT(menu && button);
	if (mSelectedEntityID != 0)
	{
		CltEntityBase* entity = CltEntityMgr::instance().getEntity( mSelectedEntityID );
		if ( std::string("Object") == entity->className() )
		{
			CltEntityMainPlayer::instance().pickup( mSelectedEntityID );
		}
	}
	menu->setVisible(false);
	return true;
}

bool CltCEGUIActionMenu::Fight_Action(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* menu = root->getChild("InGame/ActionMenu");
	LogDBG("Perform fight action");
	menu->setVisible(false);
	CltCombatMgr::instance().setTarget( mSelectedEntityID );
	CltCombatMgr::instance().combatAction(MsgCombat::START);
	return true;
}

bool CltCEGUIActionMenu::Dialog_Action(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* menu = root->getChild("InGame/ActionMenu");
	LogDBG("Perform dialog action");

	if ( mSelectedEntityID != 0 )
		CltCEGUIDialog::instance().initiateDialog( mSelectedEntityID );

	menu->setVisible(false);
	return true;
}

bool CltCEGUIActionMenu::Trade_Action(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* menu = root->getChild("InGame/ActionMenu");
	LogDBG("Perform trade action");
	menu->setVisible(false);
	return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
