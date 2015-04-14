/*
 * botcombat.cpp
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

#include "botcombat.h"

//----------------------- ConfigMgr ----------------------------
template <> BotCombatMgr* Singleton<BotCombatMgr>::INSTANCE = 0;

BotCombatMgr::~BotCombatMgr()
{
}


bool BotCombatMgr::handleMsg(MsgCombat* msg)
{
	LogDBG("Handling combat message...");
	switch(msg->getState())
    {
        case MsgCombat::START:
			/// other player has initiated trade.
			/// need to accept or end.
			target = msg->target;
			LogNTC("Will you accept a duel from '%lu'?",
                                                        msg->target );
			return true;
        case MsgCombat::END:
			mState = msg->getState();
			///Clear all lists.
			LogNTC("Combat ended by '%lu'.", target );
			target = 0;
			return true;
        case MsgCombat::ACCEPTED:
			//set state
			mState = msg->getState();
			target = msg->target;
			LogNTC("Combat accepted from '%lu'.", target );
			return true;
        default:
            return false;
    }
}


bool BotCombatMgr::handleActionMsg(MsgCombatAction* msg)
{
	/// You would receive info from other players in the battle.
	/// Not sure if this is necessary.
	switch( msg->getAction() )
    {
        case MsgCombatAction::ATTACK:
			return true;
        case MsgCombatAction::DEFEND:
			return true;
        case MsgCombatAction::NON_COMBAT:
			return true;
        default:
            return false;
    }
}


bool BotCombatMgr::handleResultMsg(MsgCombatResult* msg)
{
	/// You would receive info from other players in the battle.
	switch( msg->getResult() )
    {
        case MsgCombatResult::HIT:
			LogNTC("%lu hit for %d damage.", msg->target, msg->damage);
			return true;
        case MsgCombatResult::MISS:
			LogNTC("Attacker missed %lu.", msg->target );
			return true;
        case MsgCombatResult::OTHER:
			LogNTC("Some other thing happended.");
			return true;
        default:
            return false;
    }
}


void BotCombatMgr::listCombatInfo()
{
	LogNTC("Target (%lu).", target);
	return;
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
