/*
 * config.h
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

#ifndef __FEARANN_CONFIG_H__
#define __FEARANN_CONFIG_H__


/// Version string
#define VERSION "0.3.1"

/// Using std namespace shouldn't be harmful
using namespace std;

/// Including <string> so we can use it everywhere as basic type
#include <string>

/// Logging functions, defined in common/logger
#include "common/logger.h"

/// Basic types used everywhere
#include "common/datatypes.h"

/// Utility functions
#include "common/util.h"

/// Assert that will stay there in the final code
#define PERM_ASSERT(x)							\
	if (!(x)) {							\
		LogFATAL("FATAL: Assertion failed"			\
			 " (file: '%s' line: '%d'): %s\n",		\
			 __FILE__, __LINE__, #x);			\
		exit(-1);						\
	}

/// If we're not using GNU C, elide __attribute__
#ifndef __GNUC__
#  define  __attribute__(x)  /*NOTHING*/
#endif

/// Pi number
const float PI_NUMBER = 3.141592f;

#endif



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
