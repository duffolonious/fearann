/*
 * cltcombatmgr.cpp
 * Copyright (C) 2005-2008 by Bryan Duff <duff0097@umn.edu>
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

#include <cstdlib>

#include "client/net/cltnetmgr.h"
#include "client/cltentitymgr.h"
#include "client/entity/cltentitybase.h"
#include "client/cegui/cltceguimgr.h"

#include "cltcombatmgr.h"

//----------------------- CltCombatMgr ----------------------------
template <> CltCombatMgr* Singleton<CltCombatMgr>::INSTANCE = 0;

bool CltCombatMgr::handleMsg(MsgCombat* msg)
{
	LogDBG("Handling combat message...");
	switch(msg->getState())
	{
        case MsgCombat::START:
	{
		/// other player has initiated combat.  need to accept or end.
		mTarget = msg->target;
		/// Needs to be converted to a popup.
		CltEntityBase* entity = CltEntityMgr::instance().getEntity( mTarget );
		PERM_ASSERT( entity );
		const char * targetName = entity->className();
		string msgText = StrFmt("Will you accept a duel from %s?", 
					     targetName );
		CltCEGUIMgr::instance().Combat_DisplayMessage( msgText.c_str() );
		LogNTC("Will you accept a duel from '%s'?", targetName);
		return true;
	}
        case MsgCombat::END:
	{
		mState = MsgCombat::END;
		///Clear all lists.
		CltEntityBase* entity = CltEntityMgr::instance().getEntity( mTarget );
		LogNTC("Combat ended by '%s'.", entity->className() );
		mTarget = 0;
		return true;
	}
        case MsgCombat::ACCEPTED:
	{
		//set state
		mState = MsgCombat::ACCEPTED;
		mTarget = msg->target;
		CltEntityBase* entity = CltEntityMgr::instance().getEntity( mTarget );
		LogNTC("Combat accepted from by '%s'.", entity->className() );
		return true;
	}
        default:
		return false;
	}
}


bool CltCombatMgr::handleResultMsg(MsgCombatResult* msg)
{
	/// You would receive info from other players in the battle.
	switch( msg->getResult() )
	{
        case MsgCombatResult::HIT:
		LogNTC("%llu hit for %d damage.", msg->target, msg->damage);
		return true;
        case MsgCombatResult::MISS:
		LogNTC("Attacker missed %llu.", msg->target);
		return true;
        case MsgCombatResult::OTHER:
		LogNTC("Some other thing happended.");
		return true;
        default:
		return false;
	}
}

void CltCombatMgr::combatAction( MsgCombat::BATTLE_STATE state )
{
	mState = state;
	/// start/end/accept
	MsgCombat msg;
	msg.target = mTarget;
	msg.state = mState;
	CltNetworkMgr::instance().sendToServer( msg );
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
