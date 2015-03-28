/*
 * srvworldtimemgr.cpp
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

#include "srvworldtimemgr.h"

#include "common/net/msgs.h"

#include "server/db/srvdbmgr.h"
#include "server/net/srvnetworkmgr.h"


/*******************************************************************************
 * SrvWorldTimeMgr
 ******************************************************************************/
template <> SrvWorldTimeMgr* Singleton<SrvWorldTimeMgr>::INSTANCE = 0;

SrvWorldTimeMgr::SrvWorldTimeMgr() :
	mGameTime(0), mTicks(0)
{
	loadFromDB();
}

void SrvWorldTimeMgr::finalize()
{
	saveToDB();
}

void SrvWorldTimeMgr::sendTick(uint32_t ticks)
{
	mTicks += ticks;

	// every 5 seconds add 1 minute to the game time counter (minimum
	// resolution)
	if (mTicks >= 5000) {
		mTicks -= 5000;
		++mGameTime;
		saveToDB();
		sendTimeToAllPlayers();
	}
}

void SrvWorldTimeMgr::sendTimeToPlayer(const LoginData* player) const
{
	MsgTimeMinute msg;
	msg.gametime = mGameTime;
	SrvNetworkMgr::instance().sendToPlayer(msg, player);
}

void SrvWorldTimeMgr::sendTimeToAllPlayers() const
{
	MsgTimeMinute msg;
	msg.gametime = mGameTime;
	SrvNetworkMgr::instance().sendToAllPlayers(msg);
}

uint32_t SrvWorldTimeMgr::getGameTime() const
{
	return mGameTime;
}

void SrvWorldTimeMgr::changeTime(int minutes)
{
	mGameTime += minutes;
	saveToDB();
	sendTimeToAllPlayers();
}

void SrvWorldTimeMgr::loadFromDB()
{
	SrvDBQuery query;
	query.setTables("world");
	query.addColumnWithoutValue("time");
	int numresults = SrvDBMgr::instance().querySelect(&query);
	if (numresults != 1) {
		LogERR("Couldn't get the game time from the DB (nresults: %d)",
		       numresults);
	} else {
		int time = 0;
		query.getResult()->getValue(0, "time", time);
		mGameTime = static_cast<uint32_t>(time);
		LogNTC("Time loaded: %u", mGameTime);
	}
}

void SrvWorldTimeMgr::saveToDB()
{
	string time = StrFmt("%u", mGameTime);

	SrvDBQuery query;
	query.setTables("world");
	query.addColumnWithValue("time", time);
	bool success = SrvDBMgr::instance().queryUpdate(&query);
	if (!success) {
		LogERR("Couldn't save the game time to the DB");
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
