/*
 * cltmain.cpp
 * Copyright (C) 2006-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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
#include "client/cltconfig.h"

#include "common/configmgr.h"

#include "client/cegui/cltceguiinitial.h"
#include "client/net/cltnetmgr.h"
#include "client/cltviewer.h"

#include "cltmain.h"

#include <osg/ArgumentParser>

#include <cstdlib>

/// Path to the config file
#define CLIENT_CONFIG_FILE	"data/client/client.cfg"


/*******************************************************************************
 * CltMain
 ******************************************************************************/
void CltMain::parseArguments(int argc, char* argv[])
{
	osg::ArgumentParser arguments(&argc, argv);
	int errorIndex = 0;

	// --help
	if ((errorIndex = arguments.find("-h")) > 0
	    || (errorIndex = arguments.find("--help")) > 0) {
		arguments.remove(errorIndex);
		printf("Options for Fearann Muin Client:\n");
		printf("  --no-content-update: Disables automatic content update\n");
		exit(EXIT_SUCCESS);
	}

	// --version
	if ((errorIndex = arguments.find("-v")) > 0
	    || (errorIndex = arguments.find("--version")) > 0) {
		arguments.remove(errorIndex);
		printf("Fearann Muin On-line RPG (client), version %s\n", VERSION);
		exit(EXIT_SUCCESS);
	}

	// --no-content-update
	if ((errorIndex = arguments.find("--no-content-update")) > 0) {
		arguments.remove(errorIndex);
		LogNTC("Disabling content update, requested from command line");
		CltCEGUIInitial::instance().setDoContentUpdate(false);
	}

	// options left unread are converted into errors
	arguments.reportRemainingOptionsAsUnrecognized();
	if (arguments.errors()) {
		osg::ArgumentParser::ErrorMessageMap errors = arguments.getErrorMessageMap();
		for (osg::ArgumentParser::ErrorMessageMap::iterator it = errors.begin();
		     it != errors.end(); ++it) {
			LogERR("%s", it->first.c_str());
		}
		exit(EXIT_FAILURE);
	}
}

void CltMain::start()
{
	LogNTC("Fearann Muin client starting...");

	// loading config
	bool configLoaded = ConfigMgr::instance().loadConfigFile(CLIENT_CONFIG_FILE);
	if (!configLoaded) {
		LogERR("Couldn't load config file: '%s'", CLIENT_CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	// starting viewer and GUI within it (GUI needs the window to be created
	// before)
	CltViewer::instance().setup();

	// starting the render loop
	CltViewer::instance().start();
}

void CltMain::quit()
{
	LogNTC("Fearann Muin client stopping...");
	CltNetworkMgr::instance().disconnect();
	CltViewer::instance().stop();
	LogNTC("Fearann Muin client shut down.");
	exit(EXIT_SUCCESS);
}


/*******************************************************************************
 * Main function call
 ******************************************************************************/
int main(int argc, char* argv[])
{
	CltMain::parseArguments(argc, argv);
	CltMain::start();

	return 0;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
