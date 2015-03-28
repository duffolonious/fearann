/*
 * cltentitymainplayer.cpp
 * Copyright (C) 2006 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#include "common/net/msgs.h"

#include "client/cltcamera.h"
#include "client/cegui/cltceguiinventory.h"
#include "client/net/cltnetmgr.h"
#include "client/cltentitymgr.h"

#include "cltentitymainplayer.h"


//----------------------- CltEntityMainPlayer ----------------------------
CltEntityMainPlayer* CltEntityMainPlayer::INSTANCE = 0;

CltEntityMainPlayer& CltEntityMainPlayer::instance()
{
	if (!INSTANCE)
		LogERR("This special class has to be initialized with createEntity()");
	return *INSTANCE;
}

bool CltEntityMainPlayer::isInitialized()
{
	return INSTANCE != 0;
}

CltEntityMainPlayer* CltEntityMainPlayer::createEntity(const MsgEntityCreate* entityBasicData)
{
	INSTANCE = new CltEntityMainPlayer(entityBasicData);
	return INSTANCE;
}

CltEntityMainPlayer::CltEntityMainPlayer(const MsgEntityCreate* entityBasicData) :
	CltEntityPlayer(entityBasicData)
{
	// override var initialized by base class
	mClassName = "MainPlayer";
}

CltEntityMainPlayer::~CltEntityMainPlayer()
{
}

void CltEntityMainPlayer::setPlayerData(MsgPlayerData* playerData)
{
       mPlayerData = *playerData;
}

void CltEntityMainPlayer::pickup(uint32_t itemID)
{
        // send message to the server
        MsgInventoryGet msg;
        msg.itemID = itemID;
        CltNetworkMgr::instance().sendToServer(msg);
}

void CltEntityMainPlayer::drop(uint32_t itemID)
{
        MsgInventoryDrop msg;
        msg.itemID = itemID;
        CltNetworkMgr::instance().sendToServer(msg);
}

void CltEntityMainPlayer::addToInventory(InventoryItem* item)
{
	CltCEGUIInventory::instance().addItem(item);
}

void CltEntityMainPlayer::removeFromInventory(uint32_t itemID)
{
	CltCEGUIInventory::instance().removeItem(itemID);
}

float CltEntityMainPlayer::getDistanceToEntity( uint32_t targetID ) const
{
	CltEntityBase* target = CltEntityMgr::instance().getEntity(targetID);
	PERM_ASSERT(target);

	const MsgEntityMove& mMov = *target->getMovementProperties();

	// formula for distance, look in some place for more info if you don't
	// understand it
	float x = pow(mEntityMovData.position.x - mMov.position.x, 2);
	float y = pow(mEntityMovData.position.y - mMov.position.y, 2);
	float z = pow(mEntityMovData.position.z - mMov.position.z, 2);
	float distance = sqrt(x + y + z);
	LogDBG("Distance (%.1f, %.1f, %.1f) to (%.1f, %.1f, %.1f) = %0.1f",
	       mEntityMovData.position.x, mEntityMovData.position.y, mEntityMovData.position.z,
	       mMov.position.x, mMov.position.y, mMov.position.z,
	       distance);

	return distance;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
