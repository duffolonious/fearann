/*
 * srvcombatmgr.h
 * Copyright (C) 2006 by Bryan Duff <duff0097@umn.edu>
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

/** Combat Manager - basic concept.
 * The combat manager is similar to the trade manager - both are stateful, 
 * both use 1 to 1 interaction as the basic (as in, playerA trades with 
 * playerB, playerC fights playerD.
 * Although there are many differences, fighting is a lot more complex, and 
 * has many special cases.
 * Combat, simple cases:
 * 1. One human player vs. one AI creature/NPC
 * 2. One human player vs. one human player (duel)
 *
 */

#ifndef __FEARANN_SERVER_ACTION_COMBAT_MGR_H__
#define __FEARANN_SERVER_ACTION_COMBAT_MGR_H__


#include "common/patterns/singleton.h"
#include "common/net/msgs.h"


class LoginData;
class SrvEntityPlayer;


/** Class governing in-game events, such as day/night.
 */
class SrvCombatBattle
{
public:
	SrvCombatBattle();
	~SrvCombatBattle();

	/** Start battle initiated.  For duels the battle must be accepted.
	 * For normal fights (vs. animals or NPC's) - battles are automatically
	 * accepted.
	 */
	/** Receives the tick from the main application to count the game
	 * time */
	void sendTick(uint32_t ticks);

	///add entity
	void addEntity( SrvEntityPlayer* entity );
	/// find entity via unique id
	SrvEntityPlayer* findEntity( uint64_t id );
	bool removeEntity( uint64_t id );
	/// Or find player via name
	SrvEntityPlayer* findPlayer( uint64_t entityID );
	bool removePlayer( uint64_t id );

	/// List battle info
	void listInfo();

	/// Order players by initiative - current doesn't work.
	void orderInit();

	// trade items wagered in duel
	void transactDuel();

	void performAction( SrvEntityPlayer * playerEntity );

	void setState( MsgCombat::BATTLE_STATE _state ) { mState = _state; };
	MsgCombat::BATTLE_STATE getState() { return mState; };
	void setType( MsgCombat::BATTLE_TYPE _type ) { type = _type; };
	MsgCombat::BATTLE_TYPE getType() { return type; };

private:
	/// Check to see if battle should end (returns true if battle ended)
	bool checkBattle();
	/// Auxiliar variable to increase gametime every 5 seconds
	int mTicks;
	/// The round of combat it is.
	int mRound;
	/// battle state
	MsgCombat::BATTLE_STATE mState;
	/// battle type
	MsgCombat::BATTLE_TYPE type;

	///Probably change to a player entity class pointer
	std::vector<SrvEntityPlayer*> entities;
};


/** Class governing in-game events, such as day/night.
 */
class SrvCombatMgr : public Singleton<SrvCombatMgr>
{
public:
	/** Finalize, do whatever cleanup needed when the server shuts
	 * down. */
	void finalize();

	/// Handle the various combat messages.
	bool handleMsg( MsgCombat* msg );
	bool handleActionMsg( MsgCombatAction* msg );

	/** Receives the tick from the main application to count the game
	 * time */
	void sendTick(uint32_t ticks );

	/// Find a battle
	SrvCombatBattle * findBattle( uint64_t entityID );

	/// Add a new battle
	void addBattle( MsgCombat * msg );
	/** Remove an entity from a battle - if only 1 entity left in battle, 
	 * end battle */
	void removePlayerFromBattle( LoginData* player );

	void listBattles();

private:
	/** Singleton friend access */
	friend class Singleton<SrvCombatMgr>;

	/// Auxiliar variable to increase gametime every 5 seconds
	int mTicks;

	std::vector<SrvCombatBattle*> battles;

	/** Default constructor */
	SrvCombatMgr();
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
