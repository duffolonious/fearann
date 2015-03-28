/*
 * srventitycreature.cpp
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

#include "config.h"

#include "srventitycreature.h"

#include "server/db/srvdbmgr.h"
#include "server/net/srvnetworkmgr.h"


/*******************************************************************************
 * SrvEntityCreature
 ******************************************************************************/
SrvEntityCreature::SrvEntityCreature(MsgEntityCreate& basic,
				     MsgEntityMove& mov,
				     MsgPlayerData& data ) :
    SrvEntityBase(basic, mov),
    mPlayerData(data)
{
	mPlayerInfo = new PlayerInfo( basic.meshSubtype,
				(PlayerInfo::PLAYER_RACE)0,
                "fighter",
                (PlayerInfo::PLAYER_ALIGNMENT)4,
                1 );

	// Set attributes
    mPlayerInfo->setProperty( "strength", mPlayerData.ab_str );
    mPlayerInfo->setProperty( "constitution", mPlayerData.ab_con );
    mPlayerInfo->setProperty( "dexterity", mPlayerData.ab_dex );
    mPlayerInfo->setProperty( "wisdom", mPlayerData.ab_wis );
    mPlayerInfo->setProperty( "intelligence", mPlayerData.ab_int );
    mPlayerInfo->setProperty( "charisma", mPlayerData.ab_cha );

    // Set health
    mPlayerInfo->setHealth( mPlayerData.health_cur );
}
/*
SrvEntityCreature::SrvEntityCreature( const SrvEntityCreature& oldCreature )
{
	SrvEntityBase(const SrvEntityCreature&);
	mPlayerInfo = new PlayerInfo( basic.meshSubtype,
				(PlayerInfo::PLAYER_RACE)0,
                "fighter",
                (PlayerInfo::PLAYER_ALIGNMENT)4,
                1 );

	///Use default copy constructor from these basic classes.
	mPlayerData = PlayerData( oldCreature.mPlayerData );
	mBasic = MsgEntityCreate( oldCreature.mBasic );
	mMov = MsgEntityMove( oldCreature.mMov );

	// Set attributes
    mPlayerInfo->setProperty( "strength", mPlayerData.ab_str );
    mPlayerInfo->setProperty( "constitution", mPlayerData.ab_con );
    mPlayerInfo->setProperty( "dexterity", mPlayerData.ab_dex );
    mPlayerInfo->setProperty( "wisdom", mPlayerData.ab_wis );
    mPlayerInfo->setProperty( "intelligence", mPlayerData.ab_int );
    mPlayerInfo->setProperty( "charisma", mPlayerData.ab_cha );

	insertIntoDB();
}
*/
SrvEntityCreature::~SrvEntityCreature()
{
    saveToDB();
    delete mPlayerInfo; mPlayerInfo = 0;
}
/*
void SrvEntityCreature::insertIntoDB()
{
	///\todo: duffolonious: This is ugly - but how else do you create/use a new id.
    string charname = mBasic.entityName;
    string area = mMov.area;
    string pos1 = StrFmt("%.1f", mMov.position.x);
    string pos2 = StrFmt("%.1f", mMov.position.y);
    string pos3 = StrFmt("%.1f", mMov.position.z);
    string rot = StrFmt("%.3f", mMov.rot);
	string type = mBasic.meshType;
	string subtype = mBasic.meshSubType;

	SrvDBQuery query;
    query.setTables("creatures");
    query.addColumnWithValue("area", area);
    query.addColumnWithValue("pos1", pos1);
    query.addColumnWithValue("pos2", pos2);
    query.addColumnWithValue("pos3", pos3);
    query.addColumnWithValue("rot", rot);
    query.addColumnWithValue("type", type);
    query.addColumnWithValue("subtype", subtype);
    bool success = SrvDBMgr::instance().queryInsert(&query);
    if (!success) {
        LogERR("Error while inserting position for creature '%s'",
               charname.c_str());
        return;
    }

	/// All this to set the new id.
	SrvDBQuery query;
    query.setTables("entities");
    query.setCondition("owner ISNULL");
    query.setCondition("area='" + area.str() + "'");
    query.setCondition("pos1='" + pos1.str() + "'");
    query.setCondition("pos2='" + pos1.str() + "'");
    query.setCondition("pos3='" + pos1.str() + "'");
    query.setCondition("rot='" + rot.str() + "'");
    query.setCondition("type='" + type.str() + "'");
    query.setCondition("subtype='" + subtype.str() + "'");
    query.setOrder("id");
    query.addColumnWithoutValue("id");
    int numresults = SrvDBMgr::instance().querySelect(&query);
    if (numresults == 0) {
        LogWRN("There are no creatures in the DB to get id");
        return true;
    } else if (numresults < 0) {
        LogERR("Error loading creature from the DB");
        return false;
    } else if (numresults > 0) {
		LogERR("Error: too many creatures to get id");
	}

	int row = 0, id = 0;
	query.getValue(row, "id", id);
	mBasic.entityID = id;
}
*/

void SrvEntityCreature::saveToDB()
{
	// mafm: sometimes works bad with the "exact" data, +=0.5 for pos3=z
	// (height)
	string charname = mBasic.entityName;
	string area = mMov.area;
	string pos1 = StrFmt("%.1f", mMov.position.x);
	string pos2 = StrFmt("%.1f", mMov.position.y);
	string pos3 = StrFmt("%.1f", mMov.position.z + 0.5f);
	string rot = StrFmt("%.3f", mMov.rot);
	string id = StrFmt("%llu", getID());

	SrvDBQuery query;
	query.setTables("creatures");
	query.setCondition("id='" + id + "'");
	query.addColumnWithValue("area", area);
	query.addColumnWithValue("pos1", pos1);
	query.addColumnWithValue("pos2", pos2);
	query.addColumnWithValue("pos3", pos3);
	query.addColumnWithValue("rot", rot);
	bool success = SrvDBMgr::instance().queryUpdate(&query);
	if (!success) {
		LogERR("Error while saving position for creature '%s'",
		       charname.c_str());
		return;
	}
}

void SrvEntityCreature::updateMovementFromClient(MsgEntityMove* msg)
{
    // the client (in the current implementation) is not aware of its ID, and we
    // can't trust it anyway
    msg->entityID = mBasic.entityID;

    // we don't trust in the area either, and at the moment we don't have more
    // than one anyway...
    msg->area = mBasic.area;

    // "adopt" the message, and notify the subscribers
    mMov = *msg;
    SrvEntityBaseObserverEvent event(SrvEntityBaseObserverEvent::ENTITY_CREATE, *msg);
    notifyObservers(event);

	LogDBG("Updating position: '%s' id=%llu, pos (%.1f, %.1f, %.1f) rot=%.1f"
	       " RUN=%d FW=%d BW=%d RL=%d RR=%d",
	       getName(), mMov.entityID,
	       mMov.position.x, mMov.position.y, mMov.position.z, mMov.rot,
	       mMov.run, mMov.mov_fwd, mMov.mov_bwd, mMov.rot_left, mMov.rot_right);
}

PlayerInfo * SrvEntityCreature::getPlayerInfo()
{
    return mPlayerInfo;
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
