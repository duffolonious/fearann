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

#ifndef __FEARANN_BOT_NPC_TRADE_H__
#define __FEARANN_BOT_NPC_TRADE_H__


#include "common/patterns/singleton.h"
#include "common/net/msgs.h"

#include <string>


/** Class contains and manages objects
 */
class NPCTradeMgr : public Singleton<NPCTradeMgr>
{
public:
	/// Handle incoming combat messages
	bool handleMsg(MsgCombat* msg);

	/// Trading partner stuff...
	std::string getTarget(void) { return target; };
	void setTarget(std::string _target) { target = _target; };

	///Get/Set state
	MsgTrade::MESSAGE_TYPE getState( void ) { return mState; };
	void setState( MsgTrade::MESSAGE_TYPE state ) { mState = state; };

private:
	/** Singleton friend access */
	friend class Singleton<NPCTradeMgr>;


	/// The structure containing the entities in the inventory
	std::string target;

	MsgTrade::MESSAGE_TYPE mState;

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
