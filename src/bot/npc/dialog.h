/*
 * dialog.h
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

#ifndef __FEARANN_BOT_NPC_DIALOG_H__
#define __FEARANN_BOT_NPC_DIALOG_H__


#include "common/patterns/singleton.h"
#include "common/net/msgs.h"

#include <string>
#include <map>


/** this will hold the place of dialog for each player talking to the npc.
 */
class Dialog
{
private:
	std::string mTarget;
	std::string mCurrentStory;

	//bool handleMsg( MsgChat* );

public:
	Dialog(std::string target);

	MsgNPCDialog getStory( std::string id );

	///who the npc is talking too.
	void setTarget( const char * target ) { mTarget = target; };
	const char * getTarget() { return mTarget.c_str(); };
};

/** Class manages dialog with players.
 */
class NPCDialogMgr : public Singleton<NPCDialogMgr>
{
public:
	/// Handle incoming combat messages
	bool handleMsg(MsgNPCDialogReply* msg);
	bool handleMsg(MsgChat* msg);

	/// remove old dialog
	void removeDialog( std::string player );

	/// Trading partner stuff...
	std::string getTarget(void) { return target; };
	void setTarget(std::string _target) { target = _target; };

private:
	/** Singleton friend access */
	friend class Singleton<NPCDialogMgr>;


	/// The structure containing the entities in the inventory
	std::string target;

	std::map<std::string, Dialog*> mDialogs;


	~NPCDialogMgr();
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
