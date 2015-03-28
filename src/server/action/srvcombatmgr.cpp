/*
 * srvcombatmgr.cpp
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

#include "srvcombatmgr.h"

#include "common/net/msgs.h"
#include "common/d20/rolldie.h"
#include "common/stats.h"
#include "common/tablemgr.h"

#include "server/db/srvdbmgr.h"
#include "server/entity/srventityplayer.h"
#include "server/login/srvloginmgr.h"
#include "server/net/srvnetworkmgr.h"

#include <sstream>



/** There are 2 types of battles: duels, and normal battles.  A duel is
 *  one-on-one - thus no one can attack 2 characters dueling.  But this presents
 *  a couple problems: confused AI, and the ability to protect yourself from
 *  something just by dueling your buddy.
 *  
 *  The normal battle mode has other issues - namely when 2 battles combine.
 *  For example - if A fights B, and C fights D, you have 2 separate battles.
 *  But if A turns and attacks D, then the battles are joined.  And this
 *  presents some issues with round timing.  But any properties from one battle
 *  will carryover to another.  So you'll have the same initiative, weapon,
 *  you'll won't drop spells, etc.
 */

SrvCombatBattle::SrvCombatBattle() :
	mTicks(0),mRound(0)
{
}


SrvCombatBattle::~SrvCombatBattle()
{
	entities.clear();
}


void SrvCombatBattle::sendTick(uint32_t ticks)
{
	mTicks += ticks;

	// every 6 seconds is a new round.
	// (minimum resolution)
	if (mTicks >= 6000 )
	{
		/// check to see if battle should be ended.
		if ( checkBattle() )
			return;

		mTicks -= 6000;
		++mRound;

		/// See some round status info...
		LogDBG("Starting round %d of a battle", mRound);
		listInfo();

		///Order players by initiative
		///Highest initiative starts first with seleced action
		orderInit();

		///Perform players' actions.
		std::vector<SrvEntityPlayer*>::iterator it;
		for ( it = entities.begin(); it != entities.end(); ++it )
		{
			// Perform player's selected action.
			performAction( (*it) );
		}

		///If someone dies (remove them from the battle)
		/** We also need to deal with people that run away
		 *  (say gone for 3 rounds) */
		checkBattle();
	}
}

bool SrvCombatBattle::checkBattle()
{
	/// Just in case it's already done.
	if ( mState == MsgCombat::END )
	{
		LogDBG("Battle ended");
		return true;
	}

	/// If returns true - battle has ended
	if ( entities.size() < 2 )
	{
		///\todo: duffolonious: fix when entity leaves...
		//if ( mType == MsgCombat::DUEL )
		//	transactDuel();

		mState = MsgCombat::END;
	}

	if ( mState == MsgCombat::END )
	{
		LogDBG("Battle ended");
		return true;
	}
	else
		return false;
}

void SrvCombatBattle::addEntity( SrvEntityPlayer* entity )
{
	LogDBG("Adding entity '%s' to battle",
	       entity->getLoginData()->getPlayerName() );

	///Check to see if entity is already in the battle.
	std::vector<SrvEntityPlayer*>::iterator it;
	for ( it = entities.begin(); it != entities.end(); ++it )
	{
		if ( (*it)->getID() == entity->getID() )
			return;
	}

	/// set initiative
	entity->getPlayerInfo()->setInitiative( entity->getPlayerInfo()->generateInitiative() );
	/// set default action - defend
	entity->getPlayerInfo()->setAction( MsgCombatAction::ATTACK );

	/// If not - add entity...
	entities.push_back( entity );
}

SrvEntityPlayer* SrvCombatBattle::findEntity( uint64_t id )
{
	/// is the entity in the battle?
	std::vector<SrvEntityPlayer*>::iterator it;
	for ( it = entities.begin(); it != entities.end(); ++it )
	{
		if ( (*it)->getID() == id)
			return (*it);
	}
	return 0;
}

bool SrvCombatBattle::removeEntity( uint64_t id )
{
	/// remove entity from the battle
	std::vector<SrvEntityPlayer*>::iterator it;
	for ( it = entities.begin(); it != entities.end(); ++it )
	{
		if ( (*it)->getID() == id)
		{
			entities.erase(it);
			return true;
		}
	}
	return false;
}

SrvEntityPlayer* SrvCombatBattle::findPlayer( uint64_t entityID )
{
	/// is the entity in the battle?
	std::vector<SrvEntityPlayer*>::iterator it;
	for ( it = entities.begin(); it != entities.end(); ++it )
	{
		//LogDBG("findPlayer: entity id: '%llu'", (*it)->getID() );
		if ( (*it)->getID() == entityID )
		{
			//LogDBG("findplayer: '%llu', entity player: ''", entityID );
			return (*it);
		}
	}
	return 0;
}

bool SrvCombatBattle::removePlayer( uint64_t id )
{
	/// remove entity from the battle
	std::vector<SrvEntityPlayer*>::iterator it;
	for ( it = entities.begin(); it != entities.end(); ++it )
	{
		if ( id == (*it)->getID() )
		{
			//LogDBG("removeplayer: '%s', entity player: '%s", player, 
			//						(*it)->getLoginData()->getPlayerName() );
			entities.erase( it );
			return true;
		}
	}
	return false;
}


void SrvCombatBattle::listInfo()
{
	// list battle info
	std::vector<SrvEntityPlayer*>::iterator it;
	for ( it = entities.begin(); it != entities.end(); ++it )
	{
		LogNTC("player: %s - health: %d",
		       (*it)->getLoginData()->getPlayerName(), 
		       (*it)->getPlayerInfo()->getHealth() );
	}
}


void SrvCombatBattle::orderInit()
{
	if ( entities.size() == 1)
	{
		return;
	}

	// declare variables used for sorting loop.
	std::vector<SrvEntityPlayer*> entities2;
	std::vector<SrvEntityPlayer*>::iterator i;
    std::vector<SrvEntityPlayer*>::iterator j;

    while ( entities.size() > 0 )
    {
		// i and j both start at the same element.
        i = entities.begin();
        j = i;
        for ( ; i != entities.end(); ++i )
        {
            if ( (*i)->getPlayerInfo()->getInitiative() 
				> (*j)->getPlayerInfo()->getInitiative() )
            {
                //LogDBG( "i '%d' is greater than j '%d'...\n",
                //        (*i)->getInitiative(), (*j)->getInitiative() );
                j = i;
            }
        }
        entities2.push_back( (*j) );
        entities.erase( j );
    }

	entities = entities2;
	entities2.clear();
}

void SrvCombatBattle::transactDuel()
{
	/** \todo: duffolonious: trade items wagered in duel.  the loser gives
	 * his selected items to the winner this is kind of a special case of
	 * trading - giving.  How I imagine this to work: two players will start
	 * up a trade.  One will select items to trade - but instead of a normal
	 * trade commit, they click the "Fight" button are start a duel.  The
	 * loser then gives his seleted items to the winner.
	 */
	return;
}

void SrvCombatBattle::performAction( SrvEntityPlayer * playerEntity )
{
	/** Perform the entity's set action; a couple things about them:
	 *  1. Combat actions are always directed to a target, which could be the
	 *  player itself, a friend, an enemy, or a location (for certain spells).
	 *  2. Actions like defend are considered non-combat actions - because you
	 *  don't actually attack anyone.  If you have such an action for 3 rounds
	 *  and nobody attacks you - then you are removed from the battle, the 
	 *  combat manager assumes you've run away or something similar.
	 *  3. This will probably be re-done at some point, where all actions 
	 *  (including non-combat actions) are unified, and dealt with together.
	 *  We'll see...
	 */

	MsgCombatAction::BATTLE_ACTION action;
	PlayerInfo * playerInfo = playerEntity->getPlayerInfo();
	action = (MsgCombatAction::BATTLE_ACTION)playerInfo->getAction();

	SrvEntityPlayer* targetEntity = 0;
	int attack = 0, defense = 0;
	bool hit = false, criticalHit = false;

	LogDBG("Perform action: %u", action );

	switch ( action )
	{
	case MsgCombatAction::ATTACK:
	{
		/// Check target (make sure it's in the battle)
		targetEntity = findPlayer( playerInfo->getTarget() );
		if ( !targetEntity )
		{
			LogDBG("Can not find target to attack");
			return;
		}

		PlayerInfo * targetInfo = targetEntity->getPlayerInfo();

		LogDBG("player '%s' is attacking '%s'", 
		       playerEntity->getName(),
		       targetEntity->getName());

		LogDBG("- target: '%llu'", playerInfo->getTarget() );

		/// Setup result message from action...
		MsgCombatResult msgresult;
		msgresult.target = playerInfo->getTarget();
		msgresult.result = MsgCombatResult::MISS;
		msgresult.damage = 0;  /// for magic this may not always be the case.

		/// Check weapon type - make sure target is in range
		if (playerInfo->getSpecialActionType() == MsgCombatAction::SPELL )
		{
			bool resist = false, save = false; //true means target defends.
			///\todo: duffolonious: the entire CAST block.
			/// find spell in lookup table.
			std::string sp_action = playerEntity->getPlayerInfo()->getSpecialAction();
			if ( sp_action.empty() )
				return;

			/// take spell from castors spell count.
			std::string spLevel = TableMgr::instance().getTable( "spells")
				->getValue(sp_action, "level" );

			/// make sure caster not interupted.
			//if ( interupted )
			//	return;

			/// check if saved vs...
			/// target can be a coord (spot on the ground), enemy or friend.
			
			std::string saveStr = TableMgr::instance().getTable( "spells")
				->getValue(sp_action, "savingthrow" );
			std::string resistanceStr = TableMgr::instance().getTable( "spells")
				->getValue(sp_action, "resistance");
			if ( !saveStr.empty() && saveStr.compare( "none" ) != 0 )
			{
				///calculate save.

				int savingThrow = RollDie::instance().roll("1d20");
				if ( savingThrow == 20 )
					save = true;
				else if ( savingThrow == 1 )
					save = false;
				else 
				{
					int saveDifficulty = 10 + atoi( spLevel.c_str() ) \
						+ Stats::getAbilityModifier( playerInfo->getProperty("charisma") );
					std::string targetClass = targetInfo->getClass();
					string targetLevel = StrFmt("%d", targetInfo->getLevel());
					savingThrow += atoi( TableMgr::instance().getTable( targetClass.c_str() )->getValue( targetLevel, "will") );
					if (  savingThrow > saveDifficulty )
						save = true;
				}
			}	

			if ( !resistanceStr.empty() 
			     && resistanceStr.compare( "resistance" ) != 0 )
			{
				///calculate resistance
				resist = false;
			}

			if ( !save || !resist )
				hit = true;
		}
		else
		{
			/// perform unarmed/melee/range attack function.
			/// if hit calc damage - subtract from target.
			attack = RollDie::instance().roll("1d20");

			///\todo: duffolonious: add weapon bonuses
			int classBonus = 0;
			string level = StrFmt("%d", playerInfo->getLevel());
			LogDBG( "- level: %s, class: %s", level.c_str(), playerInfo->getClass() );
			const Table* classTable = TableMgr::instance().getTable( playerInfo->getClass() );
			PERM_ASSERT(classTable);
			classBonus = atoi( classTable->getValue( level.c_str(), "attack" ) );
			LogDBG( "- class attack bonus +%d", classBonus );
			attack += classBonus;

			LogDBG("- attack roll: %d", attack);

			if ( attack == 20 )
			{
				//automatic hit and critical hit
				criticalHit = true;
				hit = true;
			}
			else if ( attack == 1)
			{
				//automatic miss
				hit = false;
			}
			else
			{
				//add attack bonus (fex: magical weapons)

				//calculate target's defense.
				defense = targetEntity->getPlayerInfo()->getAC();
				LogDBG( "- target's defense: '%d'", defense );
				//defense += targetEntity->getPlayerInfo()->getDodge();
				//LogDBG( "- target's defense (w/ dodge): '%d'", defense );
				if ( attack >= defense )
				{
					//hit
					hit = true;
				}
			}
		}

		if ( hit )
		{
			//do damage - always at least one.
			int damage = 1;
			if (playerInfo->getSpecialActionType() == MsgCombatAction::SPELL )
			{
				std::string spellName = playerInfo->getSpecialAction();
				std::string damageStr = "1d3";
				//damageStr = TableMgr::instance().getTable( "spells" )
				//						->getCellValue( spellName, "damage" );
				damage = RollDie::instance().roll( damageStr );
			}
			else
			{
				damage = RollDie::instance().roll( playerInfo->getWeapon().getDamage() );
				if ( criticalHit )
				{
					///\todo: duffolonious: get correct hit from weapon table.
					damage *= 2;
				}
				damage += Stats::getAbilityModifier( playerInfo->getProperty( "strength" ) );
			}

			if ( damage < 1 )
					damage = 1;

			///\todo: duffolonious: Check if spell interupted.

			LogDBG("- hit for damage %d", damage );

			int hp = targetEntity->getPlayerInfo()->getHealth();
			hp -= damage;
			if ( ( hp ) < 1 )
			{
				///\todo: duffolonious: nobody dies yet... but soon.
				targetEntity->getPlayerInfo()->setHealth( 1 );

				//target is dying or dead - and thus removed from battle
				if ( type == MsgCombat::DUEL )
				{
					transactDuel();
				}

				removeEntity( targetEntity->getID() );

				if ( entities.size() < 2 )
					mState = MsgCombat::END;
			}
			else
				targetEntity->getPlayerInfo()->setHealth( hp );

			LogDBG("- remaining hp: %d", hp);

			msgresult.damage = damage;
			msgresult.result = MsgCombatResult::HIT;
		}
		else
			LogDBG("- player missed");

		/// send message to target and attacker (send to everyone?)
		std::vector<LoginData*> combatants;
		combatants.push_back( playerEntity->getLoginData() );
		combatants.push_back( targetEntity->getLoginData() );
		SrvNetworkMgr::instance().sendToPlayerList(msgresult, combatants );

		break;
	}
	case MsgCombatAction::DEFEND:
	{
		// +2 to dodge (adds to dexterity, I think), -4 to attack
		//PlayerInfo * playerInfo = playerEntity->getPlayerInfo();
		//playerInfo->setDodge( playerInfo->getDodge() + 2 );
		LogDBG("Action set to defend.");
		break;
	}
	case MsgCombatAction::NON_COMBAT:
	{
		///\todo: duffolonious: this will be for noncombat actions, such as healing.
		break;
	}
	default:
		LogDBG("No action is betting taken.");
		break;
	}
}


//---------------------------- SrvCombatMgr ---------------------------
template <> SrvCombatMgr* Singleton<SrvCombatMgr>::INSTANCE = 0;

SrvCombatMgr::SrvCombatMgr() :
	mTicks(0)
{
}

void SrvCombatMgr::finalize()
{
	for (size_t i = 0; i < battles.size(); ++i) {
		delete battles[i]; battles[i] = 0;
	}
	battles.clear();
}

bool SrvCombatMgr::handleMsg( MsgCombat* msg )
{
	SrvCombatBattle * playerBattle = 0, * targetBattle = 0;

	if ( !SrvLoginMgr::instance().findPlayer( msg->target ) 
		|| msg->player ==  msg->target )
	{
		LogERR("handleMsg: Invalid target...aborting.");
		return false;
	}

	playerBattle = findBattle( msg->player );
	targetBattle = findBattle( msg->target );

	switch( msg->getState() )
	{
	case MsgCombat::START:
	{
		LogDBG("Starting a combat session...");
		if ( playerBattle == 0 && targetBattle == 0 )
		{
			LogDBG("Starting a combat session...");
			addBattle( msg );
		}
		/** \todo: duffolonious: add normal combat.
		    else
		    {
		    /// find where to add the target if a battle is already in progress
		    if ( targetBattle )
		    {
		    targetBattle->addEntity( Server->getLoginMgr()
		    ->findPlayer( msg->player.c_str() )
		    ->getPlayerEntity() );
		    }
		    else if ( battle )
		    {
		    targetBattle->addEntity( Server->getLoginMgr()
		    ->findPlayer( msg->target )
		    ->getPlayerEntity() );
		    }
		    }
		*/

		///Send trade message to other player
		MsgCombat msgcombat;
		msgcombat.state = MsgCombat::START;
		///The target for the player receiving the message
		msgcombat.target = msg->player;

		LogDBG("Sending message to '%llu'", msg->target);

		LoginData * target = SrvLoginMgr::instance().findPlayer( msg->target );
		if ( !target )
			return false;
		SrvNetworkMgr::instance().sendToPlayer(msgcombat, target);

		return true;
	}
	case MsgCombat::END:
	{
		/// Applicable when in duels only - not accepted duels.
		if ( playerBattle->getState() == MsgCombat::ACCEPTED )
			return false;

		playerBattle->removePlayer( msg->player );
			
		if ( playerBattle->getType() == MsgCombat::DUEL )
		{
			// end and cleanup battle
			// cancel wager
		}

		/// Relay message to other members of battle.
		MsgCombat msgcombat;
		msgcombat.type = MsgCombat::END;
		///The target for the player receiving the message
		msgcombat.target = msg->player;

		LoginData * target = SrvLoginMgr::instance().findPlayer( msg->target );
		if ( !target )
			return false;
		SrvNetworkMgr::instance().sendToPlayer(msgcombat, target);

		return false;
	}
	case MsgCombat::ACCEPTED:
	{
		LogDBG("player '%llu' accepted combat from '%llu'", 
		       msg->player,
		       msg->target);

		///if this is not a duel you'll automatically accept.
		playerBattle->setState( MsgCombat::ACCEPTED );
		/// Then send message to other members of battle.

		/// Relay message to other members of battle.
		MsgCombat msgcombat;
		msgcombat.state = MsgCombat::ACCEPTED;
		///The target for the player receiving the message
		msgcombat.target = msg->player;

		LoginData * target = SrvLoginMgr::instance().findPlayer( msg->target );
		if ( !target )
			return false;
		SrvNetworkMgr::instance().sendToPlayer(msgcombat, target);

		return true;
	}
	default:
		break;
	}
	return false;
}

bool SrvCombatMgr::handleActionMsg( MsgCombatAction* msg )
{
	/// Set player's action
	SrvEntityPlayer* playerEntity = SrvLoginMgr::instance().findPlayer( msg->player.c_str() )->getPlayerEntity();
	if (!playerEntity)
		return false;

	PlayerInfo * playerInfo = playerEntity->getPlayerInfo();
	playerInfo->setAction( msg->getAction() );
	playerInfo->setSpecialActionType( msg->getSpecialAction() );
	playerInfo->setSpecialAction( msg->sp_action );

	return true;
}

void SrvCombatMgr::sendTick(uint32_t ticks)
{
	// send to each battle
	SrvCombatBattle * battle;
	std::vector<SrvCombatBattle*>::iterator it;	
	for ( it = battles.begin(); it!=battles.end(); ++it )
	{
		/// If the battle is accepted send ticks.
		if ( (*it)->getState() == MsgCombat::ACCEPTED )
			(*it)->sendTick( ticks );
		else if ( (*it)->getState() == MsgCombat::END )
		{
			LogDBG("Deleting ended battle.");
			battle = (*it);
			battles.erase(it);
			delete battle; battle = 0;
			if ( battles.empty() )
				break;
		}	
	}
}

SrvCombatBattle * SrvCombatMgr::findBattle( uint64_t entityID )
{
	vector<SrvCombatBattle*>::iterator it;
	for ( it = battles.begin(); it != battles.end(); ++it )
	{
		if ( (*it)->findPlayer( entityID ) )
			return (*it);
	}
	return 0;
}

void SrvCombatMgr::addBattle( MsgCombat* msg )
{
	/// get the entities in the combat message (entity attacker and target)
	LogDBG("Finding entities for battle");
	SrvEntityPlayer* player = SrvLoginMgr::instance().findPlayer( msg->player )->getPlayerEntity();
	SrvEntityPlayer* target = SrvLoginMgr::instance().findPlayer( msg->target )->getPlayerEntity();

	if ( !target || !player )
	{
		LogERR("addBattle: Couldn't find player or target.");
		return;
	}

	LogDBG("Set battletype and target");

	///create battle and add entities, as well as state.
	SrvCombatBattle * battle = new SrvCombatBattle();

	/// Currently we are only allowing duels.
	//battle->setType( msg->getBattleType() );
	battle->setType( MsgCombat::DUEL );

	player->getPlayerInfo()->setTarget( 
		target->getID() );
	target->getPlayerInfo()->setTarget( 
		player->getID() );

	LogDBG("new battle: player: '%llu', target: '%llu'",
	       player->getPlayerInfo()->getTarget(),
	       target->getPlayerInfo()->getTarget() );

	/// Only duels aren't automatically accepted.
	if ( msg->getBattleType() == MsgCombat::DUEL )
		battle->setState( MsgCombat::START );
	else
	{
		/// Right now you can attack anybody - this will probably change
		battle->setState( MsgCombat::ACCEPTED );
	}

	LogDBG("Adding entities to battle");
	battle->addEntity( player );
	battle->addEntity( target );

	battles.push_back( battle );
}

void SrvCombatMgr::removePlayerFromBattle( LoginData* player )
{
	/// Remove an entity from the battle (after death)
	/// Or after disconnected.
	uint64_t playerID = StrToUInt64( player->getPlayerID() );
	if ( playerID == 0 ) //not sure if this will protect anything.
		return; 

	SrvCombatBattle * battle = findBattle( playerID );
	if ( battle == 0 )
		return;
	battle->removePlayer( playerID );	
}

void SrvCombatMgr::listBattles()
{
	LogNTC("List battles:");
	vector<SrvCombatBattle*>::iterator it;
	for ( it = battles.begin(); it != battles.end(); ++it )
	{
		if ( (*it)->getState() == MsgCombat::ACCEPTED )
			(*it)->listInfo();
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
