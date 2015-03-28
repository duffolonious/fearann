/*
 * srvmain.h
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

#ifndef __FEARANN_SERVER_H__
#define __FEARANN_SERVER_H__


#include "common/patterns/singleton.h"

#include <string>
#include <ctime>


/** The main server system application
 *
 * @author mafm
 */
class SrvMain : public Singleton<SrvMain>
{
public:
	/** Main initialization routine, loading and initializing all modules
	 * needed.  */
	void start();
	/** Shut down the game (game-related parts of the application) */
	static void shutdownGame();

	/** Method to read the input from the terminal (defined in the bottom */
	void interactiveModeReadInput() const;
	/** Get the uptime of the server */
	std::string getUptime() const;

private:
	/** Singleton friend access */
	friend class Singleton<SrvMain>;


	/// Helper var to calculate the uptime
	time_t mStartTimestamp;
	/// True if running in interactive mode (reading input from term)
	bool mInteractiveMode;


	/** Default constructor */
	SrvMain();

	/** Loop of tasks to perform periodically */
	void mainLoop();

	/** Execute a command */
	void executeCommand(const std::string& cmd) const;

	/** Run a simple startup script, with commands to load the map and
	 * similar functions */
	bool loadStartupScript(const std::string& file);
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
