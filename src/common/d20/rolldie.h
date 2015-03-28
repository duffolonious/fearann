/*
 * rolldie.h
 * Copyright (C) 2006-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *			      Bryan Duff <duff0097@umn.edu>
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

#ifndef __FEARANN_COMMON_RND_H__
#define __FEARANN_COMMON_RND_H__

#include <map>
#include <vector>
#include <string>


#include "common/patterns/singleton.h"


class RollData;


/**
 * This class reads and writes configuration files, considering only the lines
 * containing "=" symbols, and tries to parse the key and
 */
class RollDie : public Singleton<RollDie>
{
public:

	/**
	 * Roll with a given string such as '3d12+5'
	 *
	 * @param dieString a string which must contains dice rolling
	 * informations
	 *
	 * @return the sum of the dice values once they've been modified
	 */
	int roll(const std::string& dieString);

	/**
	 * Roll with a given string such as '3d12+5'
	 *
	 * @param dieString a string which must contains dice rolling
	 * informations
	 *
	 * @return the sum of the dice values once they've been modified
	 */
	int roll(const char* dieString);

private:
	/** Singleton friend access */
	friend class Singleton<RollDie>;

	/**
	 * Parse a string of the form '3d12+5'
	 *
	 * @param dieString the string you want to parse
	 *
	 * @param rollData the object which will contains the 3 parsed elements
	 * found in the dieString
	 *
	 * @return true if the string has been parsed
	 */
	bool parseString(const std::string& dieString, RollData& rollData);

	/** Default constructor */
	RollDie();
	/** Destructor */
	~RollDie();

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
