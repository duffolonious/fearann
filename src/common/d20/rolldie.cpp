/*
 * rolldie.cpp
 * Copyright (C) 2006-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *						Bryan Duff <duff0097@umn.edu>
 *						Arnaud Fleurentdidier Messaoudi <fken42@gmail.com>
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

#include <string>
#include <cstdlib>
#include <sys/time.h>

#include "rolldie.h"


/** Structure to hold data to roll dices, including modifiers, such as '3d12+5'
 * (3 times, size 12, modifier +5)
 */
class RollData
{
	public:
	RollData() :
	times(-1), dieSize(-1), modifier(-1) { }
	
	/** Number of times to roll */
	int times;
	
	/** Die size */
	int dieSize;
	
	/** Modifier to the random result */
	int modifier;
};


//----------------------- RollDie ----------------------------
template <> RollDie* Singleton<RollDie>::INSTANCE = 0;

RollDie::RollDie()
{
	// seed the rng -- mafm: with seconds is not very trustable, since
	// players can guess it by the uptime reported when connecting
	struct timeval now;
	gettimeofday(&now, 0);
	srand(now.tv_usec);
}

RollDie::~RollDie()
{
}

int RollDie::roll(const char* dieString)
{
	return roll(string(dieString));
}

int RollDie::roll(const string& dieString)
{
	// parse the string and get the result
	RollData rollData;
	parseString(dieString, rollData);

	// calculate and return data
	int count = 0;
	for (int i = 0; i < rollData.times; ++i)
		count += rand() % (rollData.dieSize+1);
	return (count + rollData.modifier);
}

bool RollDie::parseString(const string& dieString, RollData& rollData)
{
        /* Trying to find the number of times the dice is beeing rolled number
	 * of times to roll the dice :
	 *
	 * 3d20+4
	 * ^
	 */

        /** position of the first 'd' token in the original string */
	string::size_type dPos = dieString.find_first_of('d');

	/* if there is not one and only one d parameter you can assume the
	 * string to parse is wrong */
	if ( dPos == string::npos )
	{
		LogERR("Rolling dice: Die size not present");
		return false;
        }
	else if ( dPos != dieString.find_last_of('d') )
	{
		LogERR("Rolling dice: too many die sizes have been given");
		return false;
        }
	else
	{
		string parsed = dieString.substr(0, dPos);
		rollData.times = atoi(parsed.c_str());
        }



        /* Trying to find the modifier
	 *
	 * result modifier :
	 * 3d20+4
	 *     ^^
	 */

        /** position of the modifier token in the original string */
	string::size_type modifierPos = dieString.find_first_of("+-");

	/* if there is more than one modifier token (+ or -) you can assume the
	 * string to parse is wrong */
        if (modifierPos == string::npos)
	{
		// not found, just go on : there will be no result's
		// modification
	}
	else if (modifierPos != dieString.find_last_of("+-"))
	{
		LogERR("Rolling dice: Modifier present several times");
		return false;
	}
	else
	{
		string parsed = dieString.substr(modifierPos);
		rollData.modifier = atoi(parsed.c_str());
	}



        /* Trying to find die size
	 * dice size :
	 *
	 * 3d20+4
	 *   ^^
	 */

        /** parsed string which contains dice size */
	string parsed;

	/* if there is a modifier, then you have to be careful of not putting it
	 * in the dieSize variable */
        if ( modifierPos == string::npos )
		parsed = dieString.substr ( dPos + 1 );
	else
		parsed = dieString.substr ( dPos + 1 , modifierPos - dPos - 1 );

        /** temporary variable that shows if the parsed string contains a
	 * correct dice size */
	string::size_type parseResult;

	/* if parsed contains a caracter different from [0-9] then the original
	 * string was wrong */
	parseResult = parsed.find_first_not_of ( "0123456789" );
	if ( parseResult != string::npos )
	{
		LogERR ( "Rolling dice: Wrong die size" );
		return false;
	}

	/* the die size is ok, put it in the result object */
	rollData.dieSize = atoi ( parsed.c_str() );

        /* display the result in the DBG terminal */

        LogDBG("Rolling dice: parsed '%id%d%+d' from '%s'", rollData.times, rollData.dieSize, rollData.modifier, dieString.c_str());
        return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
