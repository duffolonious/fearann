/*
 * srvworldtimemgr.h
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

#ifndef __FEARANN_SERVER_WORLD_TIME_MGR_H__
#define __FEARANN_SERVER_WORLD_TIME_MGR_H__


#include "common/patterns/singleton.h"


class LoginData;


/** Class governing in-game events, such as day/night.
 *
 * @author mafm
 */
class SrvWorldTimeMgr : public Singleton<SrvWorldTimeMgr>
{
public:
	/** Finalize, do whatever cleanup needed when the server shuts down. */
	void finalize();

	/** Receives the tick from the main application to count the game
	 * time */
	void sendTick(uint32_t ticks);
	/** Get game time */
	uint32_t getGameTime() const;
	/** Change the time by the given number of minutes */
	void changeTime(int minutes);
	/** Send the time to the given player (useful when joining the game, so
	 * the player doesn't have to wait until next tick for the environment
	 * to be set up correctly). */
	void sendTimeToPlayer(const LoginData* player) const;

private:
	/** Singleton friend access */
	friend class Singleton<SrvWorldTimeMgr>;


	/** Game time variable, stored in the DB between executions. The unit of
	 * time is 1 minute in game, with a day being 2h of realtime a minute is
	 * 60/12 = 5 seconds -- this is the minimal resolution of the game.
	 */
	uint32_t mGameTime;
	/// Auxiliar variable to increase gametime every 5 seconds
	int mTicks;


	/** Default constructor */
	SrvWorldTimeMgr();

	/** Loads the in-game time from the DB */
	void loadFromDB();
	/** Saves the time to the DB */
	void saveToDB();
	/** Send the time update to all players. */
	void sendTimeToAllPlayers() const;
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
