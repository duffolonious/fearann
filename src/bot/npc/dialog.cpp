/*
 * dialog.cpp
 * Copyright (C) 2005-2006 by Bryan Duff <duff0097@umn.edu>
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

#include "common/tablemgr.h"

#include "bot/bot.h"
#include "bot/net/botnetmgr.h"

#include "dialog.h"

Dialog::Dialog(std::string target):
	mTarget(target),
	mCurrentStory("story1")
{
}


MsgNPCDialog Dialog::getStory( std::string id )
{
	MsgNPCDialog msgdialog;
	std::string prevStory, lines, story;
	const Table *tblDialog;

	LogDBG("getStory: Story info:\nid ='%s'\n current story = '%s'", 
										id.c_str(), mCurrentStory.c_str() );
	if ( id != "0" )
	{
		/// get old current story
		tblDialog = TableMgr::instance().getTable( mCurrentStory.c_str() );
		/// find link that "id" points to
		story = tblDialog->getValue( id, "link" );
		if ( story.empty() )
			story = mCurrentStory; /// keep current
		else
			mCurrentStory = story; /// update current
	}
	else
		story = mCurrentStory;

	LogDBG( "getStory: load next story: '%s'", story.c_str() );
	/// follow link.
	if ( story == "end" )
	{
		/// story ended.
		LogDBG("story ended");
		msgdialog.text = "";
		msgdialog.done = true;
	}
	else
	{
		//LogDBG("compiling story...");
		tblDialog = TableMgr::instance().getTable( story.c_str() );
		/// iterate through and dump all lines...
		std::string tmp, tmpStr;
		for (uint32_t i = 0; i < (static_cast<const Table*>(tblDialog))->getNumRows(); i++ )
    	{
      char buf[32];
      sscanf(buf, "%d", &i);
			tmp = tblDialog->getValue( string( buf ), "line" );
			if ( tmp.empty() )
				continue;

			if ( i > 0 ) /// prepend numeric choice
			{
				tmpStr = StrFmt("%u. %s", i, tmp.c_str() );
				NPCDialogOption option( i, tmpStr );
				msgdialog.addOption( &option );
			}
			else
				msgdialog.text = tmp;
		}
		msgdialog.done = false;
	}

	/// return next story or nothing if it's ended.
	return msgdialog;
}

//----------------------- NPCDialogMgr ----------------------------
template <> NPCDialogMgr* Singleton<NPCDialogMgr>::INSTANCE = 0;

NPCDialogMgr::~NPCDialogMgr()
{
}


bool NPCDialogMgr::handleMsg(MsgNPCDialogReply* msg)
{
	MsgNPCDialog msgdialog;
	Dialog * dl;
	std::string id;

	/// Assume story mode.
	LogDBG("Handling Dialog Reply - origin: '%s', done = '%d", msg->origin.c_str(), msg->done );
	if ( msg->done ) //if client says dialog is done.
	{
		removeDialog( msg->origin );
		return true;
	}	

	if ( !mDialogs[msg->origin.c_str()] )
	{
		LogDBG("Handling starting new story...");
		dl = new Dialog(msg->origin);
		mDialogs[ msg->origin.c_str() ] = dl;
		id = "0"; //Start with initial story
	}
	else
	{
		LogDBG("Handling continuing story: msg text: '%s'", id.c_str() );
		id = StrFmt("%d", msg->option );
		dl = mDialogs[ msg->origin.c_str() ];
	}

	/// Get message.
	msgdialog = dl->getStory( id );
	if ( msgdialog.done )
	{
		removeDialog( msg->origin );
	}

	msgdialog.target = msg->origin;
	LogDBG("sending: '%s' to '%s'", msgdialog.text.c_str(), 
									msgdialog.target.c_str() );
	Bot->getNetworkMgr()->sendToServer(msgdialog);

	return true;
}

bool NPCDialogMgr::handleMsg(MsgChat* msg)
{
	MsgChat msgchat;

	/// We don't want to continually bounce messages...
	if ( msg->origin == Bot->getName() )
		return false;

	//LogDBG("Handling dialog message...");
	switch(msg->type)
    {
        case MsgChat::PM:
		{
			///Do some default response.
			const Table * tblDialog = TableMgr::instance().getTable("greeting");
			///\todo: duffolonious: at some point - make random.
			msgchat.text = tblDialog->getValue("0", "line");
			msgchat.type = MsgChat::CHAT;
			Bot->getNetworkMgr()->sendToServer(msgchat);
			return true;
		}
        default:
            return false;

    }
}

void NPCDialogMgr::removeDialog( std::string player )
{
	std::map<std::string, Dialog*>::iterator it;
	for( it = mDialogs.begin(); it != mDialogs.end(); it++ )
	{
		if ( player == (*it).first )
		{
			delete mDialogs[ player.c_str() ];
			mDialogs.erase( it );
		}
	}
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
