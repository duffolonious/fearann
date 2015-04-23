/*
 * cltceguidialog.cpp
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
#include <CEGUI/widgets/FrameWindow.h>
#include <CEGUI/widgets/PushButton.h>
#include <CEGUI/widgets/ListboxItem.h>
#include <CEGUI/widgets/ListboxTextItem.h>
#include <CEGUI/widgets/MultiColumnList.h>

#include "common/net/msgs.h"

#include "client/net/cltnetmgr.h"
#include "client/entity/cltentitybase.h"

#include "cltceguimgr.h"

#include "cltceguidialog.h"


//----------------------- CltCEGUIDialog ----------------------------
template <> CltCEGUIDialog* Singleton<CltCEGUIDialog>::INSTANCE = 0;

CltCEGUIDialog::CltCEGUIDialog()
{
	///\todo: replace with popup menu.
	// installing events
	CEGUI_EVENT("InGame/Dialog",
		    CEGUI::Window::EventShown,
		    CltCEGUIDialog::Event_Shown);
	CEGUI_EVENT("InGame/Dialog/Reply",
		    CEGUI::PushButton::EventClicked,
		    CltCEGUIDialog::Event_Reply);
//	CEGUI_EVENT("InGame/Dialog",
//		    CEGUI::FrameWindow::EventCloseClicked,
//		    CltCEGUIDialog::Event_Close);

	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* menu = root->getChild("InGame/ActionMenu");
    PERM_ASSERT(menu);
	menu->setVisible(false);
}

CltCEGUIDialog::~CltCEGUIDialog()
{
}

void CltCEGUIDialog::updateDialog( MsgNPCDialog* msg )
{
	///Update message
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* dialog = root->getChild("InGame/Dialog");
    PERM_ASSERT(dialog);
	if ( msg->done )
	{
		dialog->setVisible(false);
		return;
	}

	CEGUI::Window* dialogText = root->getChild("InGame/Dialog/Dialog_Text");
    PERM_ASSERT(dialogText);

	CEGUI::MultiColumnList* replyList = static_cast<CEGUI::MultiColumnList*> (root->getChild("InGame/Dialog/List"));
	PERM_ASSERT(replyList);
	replyList->setSelectionMode(CEGUI::MultiColumnList::RowSingle);

	//dialogText
	dialogText->setText( msg->text.c_str() );

	uint32_t rowCount = replyList->getRowCount();
	for ( uint32_t i = 0; i < rowCount; ++i )
		replyList->removeRow( 0 );

	///Update options
	for (uint32_t i = 0; i < msg->options.size(); ++i )
	{
		CEGUI::ListboxTextItem* reply = new CEGUI::ListboxTextItem(msg->options[i].getText().c_str(), 0);
		reply->setSelectionBrushImage("FearannLook/ListboxSelectionBrush");
		unsigned int row = replyList->addRow();
		replyList->setItem( reply, 0, row );
	}

	/// Make first option the default
	replyList->clearAllSelections();
	CEGUI::MCLGridRef position(0, 0);
	replyList->setItemSelectState(position, true);

	dialog->setVisible(true);
}

bool CltCEGUIDialog::initiateDialog( uint64_t entityID )
{
	if ( entityID == 0 )
		return false;
	MsgNPCDialogReply msg;
	mSelectedEntityID = entityID;
	msg.target = mSelectedEntityID;
	msg.done = false;
	msg.option = 0;
	CltNetworkMgr::instance().sendToServer( msg );
	return true;
}

bool CltCEGUIDialog::Event_Shown(const CEGUI::EventArgs& e)
{
	return true;
}

bool CltCEGUIDialog::Event_Reply(const CEGUI::EventArgs& e)
{
	CEGUI::WindowManager* mWinMgr = &CEGUI::WindowManager::getSingleton();
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* dialog = root->getChild("InGame/Dialog");
	PERM_ASSERT(dialog);

	CEGUI::MultiColumnList* replyList = static_cast<CEGUI::MultiColumnList*> (root->getChild("InGame/Dialog/List"));
	PERM_ASSERT(replyList);

	if (replyList->getSelectedCount() == 0) {
		LogERR("Dialog: No Reply Selected");
		CltCEGUIMgr::instance().Notification_DisplayMessage("No reply selected");
		return true;
	}

	CEGUI::ListboxItem* reply = replyList->getFirstSelectedItem();

	MsgNPCDialogReply msg;
	msg.target = mSelectedEntityID;
	msg.option = atoi( reply->getText().substr(0,1).c_str() );
	msg.done = false;
	LogDBG("Option %u selected", msg.option );
	CltNetworkMgr::instance().sendToServer( msg );
	return true;
}

bool CltCEGUIDialog::Event_Close(const CEGUI::EventArgs& e)
{
	CEGUI::Window* root = CltCEGUIMgr::instance().getGUIContext()->getRootWindow();
	CEGUI::Window* dialog = root->getChild("InGame/Dialog");
	PERM_ASSERT(dialog);

	MsgNPCDialogReply msg;
	msg.target = mSelectedEntityID;
	msg.done = true;
	LogDBG("Dialog close: ending dialog");
	CltNetworkMgr::instance().sendToServer( msg );
	dialog->setVisible(false);
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
