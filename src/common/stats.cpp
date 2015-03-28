/*
 * stats.cpp
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

#include "config.h"

#include "stats.h"

int Stats::getAbilityModifier(int ability)
{
	return ((ability-10) >> 1);
}

PlayerInfo::PlayerInfo()
{
	playerClass = "fighter";
	mAlignment = PlayerInfo::NEUTRAL;
	race = PlayerInfo::HUMAN;
	// default to first level
	mLevel = 1;
	action = 1;
	mTarget = 0;
}

PlayerInfo::PlayerInfo(std::string _gender,
		       PLAYER_RACE _race,
		       std::string _class,
		       PLAYER_ALIGNMENT _alignment,
		       int _level )
{
	gender = _gender;
	race = _race;
	playerClass = _class;
	mAlignment = _alignment;
	mLevel = _level;
	action = 1; //ATTACK
	mTarget = 0;
}

void PlayerInfo::equipItem( PLAYER_EQUIP type, int itemID )
{
	switch( type )
	{
		case HEAD:
			head = itemID;
			break;
        case NECK:
			neck = itemID;
			break;
        case RING1:
			ring1 = itemID;
			break;
        case RING2:
			ring2 = itemID;
			break;
        case ROBE:
			robe = itemID;
			break;
        case SHOULDER:
			shoulder = itemID;
			break;
        case ARM:
			arm = itemID;
			break;
        case LHAND:
			lhand = itemID;
			break;
        case RHAND:
			rhand = itemID;
			break;
        case TORSO:
			torso = itemID;
			break;
        case BELT:
			belt = itemID;
			break;
        case THIGH:
			thigh = itemID;
			break;
        case FEET:
			feet = itemID;
			break;
	}
}

int PlayerInfo::getEquippedItem( PLAYER_EQUIP type )
{
	switch( type )
	{
		case HEAD:
			return head;
        case NECK:
			return neck;
        case RING1:
			return ring1;
        case RING2:
			return ring2;
        case ROBE:
			return robe;
        case SHOULDER:
			return shoulder;
        case ARM:
			return arm;
        case LHAND:
			return lhand;
        case RHAND:
			return rhand;
        case TORSO:
			return torso;
        case BELT:
			return belt;
        case THIGH:
			return thigh;
        case FEET:
			return feet;
	}
	return -1;
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
