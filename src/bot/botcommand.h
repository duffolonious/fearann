/*
 * srvcommand.h
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

/// @ingroup srvconsole
/// @{

#ifndef __FEARANN_BOT_COMMAND_H__
#define __FEARANN_BOT_COMMAND_H__

#include "common/command.h"


/** Command manager class for the server, the registerCommands method
 * is defined so it registers the commands specific to the server.
 */
class BotCommandMgr : public CommandMgr
{
public:
	/** Defined to complete the class */
	virtual void registerCommands();
};


#endif

/// @}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
