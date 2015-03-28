/*
 * stats.h
 * Copyright (C) 2006 by Bryan Duff <duff0097@umn.edu>
 *                       Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#ifndef __FEARANN_COMMON_STATS_H__
#define __FEARANN_COMMON_STATS_H__


#include "d20/rolldie.h"

#include <string>
#include <cstdlib> 
#include <ctime> 


namespace Stats {
	/** returns (ability-10)/2 */
	int getAbilityModifier(int ability);
}

class Weapon
{
public:
	enum WEAPON_ATTACK { LIGHT = 1, ONE_HANDED, TWO_HANDED, RANGED };
	enum WEAPON_TYPE { BLUDGEONING = 1, PIERCING, SLASHING };

	Weapon() :
		mAttack(LIGHT),
		mType(BLUDGEONING),
		mBonus(0),
		mDrawn(true)
	{
		mDamage = "1d6+2";
	};

	const char* getDamage() const { return mDamage.c_str(); };	

private:
	/// light(unarmed)/one-handed/two-handed/ranged;
	WEAPON_ATTACK mAttack;
	WEAPON_TYPE mType;
	/// bonus to attack
	int mBonus;
	/// damage (default 1d3)
	std::string mDamage;
	/// Is the weapon drawn or sheathed?
	bool mDrawn;
};


/** Player info
 */
class PlayerInfo
{
public:
	enum PLAYER_RACE { HUMAN = 1, ELF, DWARF };
	enum PLAYER_ALIGNMENT
		{ 
			NONE = 0, 
			LAWFUL_GOOD,
			NEUTRAL_GOOD,
			CHAOTIC_GOOD,
			LAWFUL_NEUTRAL,
			NEUTRAL,
			CHAOTIC_NEUTRAL,
			LAWFUL_EVIL,
			NEUTRAL_EVIL,
			CHAOTIC_EVIL
		};

	enum PLAYER_EQUIP
		{
			HEAD=1,
			NECK,
			RING1,
			RING2,
			ROBE,
			SHOULDER,
			ARM,
			LHAND,
			RHAND,
			TORSO,
			BELT,
			THIGH,
			FEET
		};

	PlayerInfo();
	PlayerInfo(std::string _gender,
		   PlayerInfo::PLAYER_RACE _race,
		   std::string _class,
		   PlayerInfo::PLAYER_ALIGNMENT _alignment,
		   int _level );
	virtual ~PlayerInfo() {};

	/// generate ac: base + armor + shield + dex + size
	int getAC(void) { return (10 + 0 + 0 + Stats::getAbilityModifier(getProperty("dexterity")) + 0); };
	/// generate hp: (class base + str + con ) * mLevel
	int getHP(void) { return (10 + Stats::getAbilityModifier(getProperty("strength")) + \
				  Stats::getAbilityModifier(getProperty("constitution") ) ) \
			* mLevel; };
	int generateInitiative(void) { return ( RollDie::instance().roll("1d20") 
						+ Stats::getAbilityModifier(getProperty("dexterity") ) ); };
	/*
	  virtual void setFortitude( int value ) { mFortitude = value; };
	  int getFortitude() { return mFortitude; };
	  virtual void setReflex( int value ) { mReflex = value; };
	  int getReflex() { return mReflex; };
	  virtual void setWill( int value ) { mWill = value; };
	  int getWill() { return mWill; };
	*/
	/// Misc bonuses
	virtual void setDodge( int value ) { mDodgeBonus = value; };
	int getDodge() { return mDodgeBonus; };
	virtual void setDeflection( int value ) { mDeflectionBonus = value; };
	int getDeflection() { return mDeflectionBonus; };


	void equipItem( PLAYER_EQUIP, int value );
	int getEquippedItem( PLAYER_EQUIP );

	/// Reset every battle
	void setInitiative( int _init ) { initiative = _init; };
	int getInitiative() { return initiative; };

	void setLevel( int _level) { mLevel = _level; };
	int getLevel() { return mLevel; };

	void setHealth( int _health) { health = _health; };
	int getHealth() { return health; };

	/// Combat data
	void setTarget( uint64_t _target ) { mTarget = _target; };
	uint64_t getTarget() { return mTarget; };

	void setAction( uint32_t _action ) { action = _action; };
	uint32_t getAction() { return action; };
	void setSpecialActionType( int _sp_action_type )
	{ sp_action_type = _sp_action_type; };
	uint32_t getSpecialActionType() { return sp_action_type; };
	void setSpecialAction( std::string _sp_action ) { sp_action = _sp_action; };
	std::string getSpecialAction() { return sp_action; };

	Weapon getWeapon() { return weapon; };
	//Weapon hasWeapon2() { return weapon2; };
	//Shield getShield() { return 0; };

	PLAYER_RACE getRace() { return race; };
	void setClass(std::string _class) { playerClass = _class; };
	const char * getClass() { return playerClass.c_str(); };
	PLAYER_ALIGNMENT getAlignment() { return mAlignment; };
	virtual void setAlignment( PlayerInfo::PLAYER_ALIGNMENT value ) { mAlignment = value; };

	void setReputation( int _reputation) { reputation = _reputation; };
	int getReputation() { return reputation; };

	int getProperty( const char * _prop ) { return mProperties[_prop]; };
	void setProperty( const char * _prop, int value )
	{ mProperties[_prop] = value; };

private:
	/// this will store attributes, saves, and other misc. data
	/// map has:
	/// attributes: strength, dexterity, constitution, intelligence, wisdom, charisma
	std::map<const char *, int> mProperties;

	/// Gender (m or f)
	std::string gender;
	/// Level
	int mLevel;
	/// Race
	PlayerInfo::PLAYER_RACE race;
	/// Class
	std::string playerClass;
	/// Alignment
	PlayerInfo::PLAYER_ALIGNMENT mAlignment;
	/// hp
	int health;
	/// reputation - how AI treats you.
	int reputation;

	int mFortitude;
	int mReflex;
	int mWill;

	///bonuses
	int mDodgeBonus;
	int mDeflectionBonus;

	/// action to be taken - currently combat only.
	uint32_t action;
	uint32_t sp_action_type;
	std::string sp_action;

	///Targeted player for combat
	uint64_t mTarget;

	Weapon weapon;
	Weapon weapon2;
	//Shield shield;

	// Higher the better - highest number in a battle attack first every round.
	int initiative;
	/** Now I'll go over different player parts.
	 *  Armor sections, hands, neck, fingers (for rings), etc...
	 */
	int head;
	int neck;
	int ring1;
	int ring2;
	int robe;
	int shoulder;
	int arm;
	int lhand;
	int rhand;
	int torso;
	int belt;
	int thigh;
	int feet;

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
