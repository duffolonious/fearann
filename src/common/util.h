/*
 * util.h
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

/** \file util.h
 *
 * @author mafm
 *
 * This file contains several utility functions, data structures or
 * algorithms, used everywhere in the code and generally
 * unclassifiable under a common class.
 */

#ifndef __FEARANN_COMMON_UTIL_H__
#define __FEARANN_COMMON_UTIL_H__

#include <stdint.h>
#include <string>


/** String formatter, a wrapper around vsnprintf so we get rid of all the mess
 * of the variadic functions in regular code.  And no, C++ features are not as
 * clean as this solution, although they have other advantages; the main purpose
 * of this function is printf-like clarity.
 *
 * @author mafm
 */
const char* StrFmt(const char* fmt, ...) __attribute__((format(printf, 1, 2)));


/** Trim function (both sides)
 *
 * @author mafm
 */
void StrTrim(std::string& source);


/** Convert string to uint64_t
 */
uint64_t StrToUInt64(const std::string& str);


/** Ensure that the first parameter is less or equal than the second, otherwise
 * set it as equal
 *
 * @param target The parameter that we want to check if it's within the limits
 *
 * @param limit The limit
 *
 * @author mafm
 */
template <class T>
inline
void ensureLessOrEqual(T& target, const T& limit)
{
	if (target > limit) {
		target = limit;
	}
}

/** Ensure that the first parameter is greater or equal than the second,
 * otherwise set it as equal
 *
 * @param target The parameter that we want to check if it's within the limits
 *
 * @param limit The limit
 *
 * @author mafm
 */
template <class T>
inline
void ensureGreaterOrEqual(T& target, const T& limit)
{
	if (target < limit) {
		target = limit;
	}
}

/** Power 2, simple shortcut
 *
 * @param number number to raise to the power of 2
 *
 * @author mafm
 */
inline float power2(const float& number)
{
	return (number * number);
}


#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
