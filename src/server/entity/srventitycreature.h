/*
 * srventitycreature.h
 * Copyright (C) 2006-2008 by Bryan Duff <duff0097@umn.edu>
 *			      Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#ifndef __FEARANN_SERVER_ENTITY_CREATURE_H__
#define __FEARANN_SERVER_ENTITY_CREATURE_H__


#include "common/net/msgs.h"
#include "common/stats.h"

#include "srventitybase.h"


class PlayerInfo;


/** Representation of a Creature in the server
 *
 * @author mafm
 */
class SrvEntityCreature : public SrvEntityBase
{
public:
	SrvEntityCreature(MsgEntityCreate& basic,
			  MsgEntityMove& mov,
			  MsgPlayerData& data );
	//SrvEntityCreature( const SrvEntityCreature& oldCreature );
	virtual ~SrvEntityCreature();

	/** Get the player info */
	PlayerInfo * getPlayerInfo();
	/** Update all the movement related stuff with the data provided by the
	 * client */
	void updateMovementFromClient(MsgEntityMove* msg);
	/** Send the info about movement to the player (the client normally
	 * doesn't receive the own position from the server, so it's because the
	 * position has been reseted or something similar) */
	void sendMovementToClient();
	/** have creature move to position
	 * void moveTo( Vector3 pos ); */
	/** halt movement of creature
	 * void halt(); */
	/** set personality to:
	 * -aggressive: attacks anything
	 * -flee: runs away from everything
	 * -passive: does nothing
	 * void setPersonality( PERSONA ); */

protected:
	/// Player data
	MsgPlayerData mPlayerData;
	/// Extra player info (class, alignment)
	PlayerInfo * mPlayerInfo;

	/** Virtual function overriden from base class, to treat the
	 * specifics of the player. */
	//void insertIntoDB();
	void saveToDB();
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
