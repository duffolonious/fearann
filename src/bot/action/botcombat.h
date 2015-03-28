/*
 * botcombat.h
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

#ifndef __FEARANN_BOT_ACTION_COMBAT_H__
#define __FEARANN_BOT_ACTION_COMBAT_H__

#include "common/patterns/singleton.h"
#include "common/net/msgs.h"

#include <string>


/** Class contains and manages objects
 */
class BotCombatMgr : public Singleton<BotCombatMgr>
{
public:
	/// Handle incoming combat messages
	bool handleMsg(MsgCombat* msg);
	/// ... combat action messages
	bool handleActionMsg(MsgCombatAction* msg);
	bool handleResultMsg(MsgCombatResult* msg);

	/// List items in inventory
	void listCombatInfo();

	/// Trading partner stuff...
	uint64_t getTarget(void) { return target; };
	void setTarget(uint64_t _target) { target = _target; };

	///Get/Set state
	MsgCombat::BATTLE_STATE getState( void ) { return mState; };
	void setState( MsgCombat::BATTLE_STATE state ) { mState = state; };

private:
	/** Singleton friend access */
	friend class Singleton<BotCombatMgr>;


	/// The structure containing the entities in the inventory
	uint64_t target;

	MsgCombat::BATTLE_STATE mState;


	~BotCombatMgr();
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
