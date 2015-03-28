/*
 * srvmain.cpp
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

#include "config.h"

#include "srvmain.h"

#include "common/configmgr.h"

#include "server/content/srvcontentmgr.h"
#include "server/console/srvconsolemgr.h"
#include "server/console/srvcommand.h"
#include "server/db/srvdbmgr.h"
#include "server/login/srvloginmgr.h"
#include "server/net/srvnetworkmgr.h"
#include "server/world/srvworldmgr.h"
#include "server/world/srvworldtimemgr.h"
#include "server/action/srvtrademgr.h"
#include "server/action/srvcombatmgr.h"

#include <iostream>
#include <istream>
#include <fstream>
#include <ctime>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>


/// Path to the config file
#define SERVER_CONFIG_FILE	"data/server/server.cfg"


/*******************************************************************************
 * SrvMain
 ******************************************************************************/
template <> SrvMain* Singleton<SrvMain>::INSTANCE = 0;

SrvMain::SrvMain()
{
	mInteractiveMode = true;
	mStartTimestamp = time(0);

	// seed the rng -- mafm: with seconds is not very trustable, since
	// players can guess it by the uptime reported when connecting
	struct timeval now;
	gettimeofday(&now, 0);
	srand(now.tv_usec);
}

void SrvMain::shutdownGame()
{
	// mafm: we perform here the proper destruction related with the game
	// (not the application), so managers can save the needed data and so on

	LogNTC("Game server saving its data...");

	SrvTradeMgr::instance().finalize();
	SrvCombatMgr::instance().finalize();
	SrvContentMgr::instance().finalize();
	SrvWorldMgr::instance().finalize();
	SrvLoginMgr::instance().finalize();
	SrvNetworkMgr::instance().finalize();
	SrvWorldTimeMgr::instance().finalize();
	SrvDBMgr::instance().finalize();

	LogNTC("Game server properly shut down.");
	exit(EXIT_SUCCESS);
}

void SrvMain::start()
{
	LogNTC("Game server starting...");

	// load the config file
	bool configLoaded = ConfigMgr::instance().loadConfigFile(SERVER_CONFIG_FILE);
	if (!configLoaded) {
		LogFATAL("Couldn't load config file: '%s'", SERVER_CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	// execution mode
	string execMode = ConfigMgr::instance().getConfigVar("Server.Runtime.ExecutionMode", "-");
	if (execMode == "interactive") {
		mInteractiveMode = true;
		LogNTC("Using Interactive mode");
	} else if (execMode == "daemon") {
		mInteractiveMode = false;
		LogNTC("Using Daemon mode");
	} else {
		LogFATAL("Couldn't get ExecutionMode from config file");
		exit(EXIT_FAILURE);
	}

	// Run startup commands
	string scriptFile = ConfigMgr::instance().getConfigVar("Server.Runtime.StartUpScript", "-");
	if (scriptFile == "-") {
		LogFATAL("Couldn't get StartUp Script file name from config file");
		exit(EXIT_FAILURE);
	} else {
		loadStartupScript(scriptFile.c_str());
	}

	// run loop until the end
	mainLoop();
}

void SrvMain::mainLoop()
{
	// infinite loop, the app will exit by another means
	while (true) {
		// be friendly to computer, sleeping for a while
		static struct timespec interval = { 0, 10*1000*1000 };
		struct timespec remaining = { 0, 0 };
		nanosleep(&interval, &remaining);
		struct timespec elapsed = { interval.tv_sec - remaining.tv_sec,
					    interval.tv_nsec - remaining.tv_nsec };

		// send a tick to the time manager (milliseconds)
		SrvWorldTimeMgr::instance().sendTick(elapsed.tv_nsec/(1000*1000));

		// send a tick to the time manager (milliseconds)
		SrvCombatMgr::instance().sendTick(elapsed.tv_nsec/(1000*1000));

		// if in interactive mode, try to read input
		if (mInteractiveMode) {
			interactiveModeReadInput();
		}

		// process incoming messages from the network
		SrvNetworkMgr::instance().processIncomingMsgs();

		/// Send some data to clients
		SrvContentMgr::instance().sendDataToClients();
	}
}

bool SrvMain::loadStartupScript(const std::string& file)
{
	LogNTC("Executing startup script: '%s'", file.c_str());
	string line;
	ifstream script(file.c_str());
	if (script.is_open()) {
		while (!script.eof()) {
			getline(script, line);
			StrTrim(line);
			if (line.empty()) {
				continue;
			}
			LogNTC("> %s", line.c_str());
			executeCommand(line.c_str());
		}
		LogNTC("Startup script completed: '%s'", file.c_str());
		script.close();
		return true;
	} else {
		LogERR("Unable to open script file: '%s'", strerror(errno));
		return false;
	}
}

void SrvMain::interactiveModeReadInput() const
{
	// vars to store a command-line-in-progress, and the character to be
	// read
	string cmd;
	char c;

	// prepare values for select call (fd=1 is stdin)
	int fd = 1;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(0, &fdset);
	struct timeval tspec = {0, 0};

	switch (select(fd, &fdset, 0, 0, &tspec))
	{
	case 1:
		c = getchar();
		while ((c != '\n') && (c != EOF)) {
			cmd.append(1, c);
			c = getchar();
		}

		if (c == '\n') {
			fprintf(stderr, "Command: %s\n", cmd.c_str());
			if (cmd.length() > 0) {
				executeCommand(cmd);
			}
			cmd.clear();
		}
		break;
	default:
		tspec.tv_sec = 0;
		tspec.tv_usec = 0;
		break;
	}
}

void SrvMain::executeCommand(const std::string& cmd) const
{
	if (cmd.empty() || cmd[0] == '\n') {
		LogWRN("Empty command, try 'help'");
	} else {
		CommandOutput out;
		SrvCommandMgr::instance().execute(cmd.c_str(), PermLevel::ADMIN, out);
		fprintf(stderr, "%s\n", out.getOutput().c_str());
	}
}

std::string SrvMain::getUptime() const
{
	time_t current_timestamp = time(0);
	double numeric_uptime_double = difftime(current_timestamp, mStartTimestamp);
	int numeric_uptime = static_cast<int>(numeric_uptime_double);
	int days = numeric_uptime / (24*60*60);
	int days_rest = numeric_uptime % (24*60*60);
	int hours = days_rest / (60*60);
	int hours_rest = days_rest % (60*60);
	int minutes = hours_rest / (60);
	int seconds = hours_rest % 60;

	return StrFmt("%dd %dh%dm%ds",
		      days, hours, minutes, seconds);
}


/*******************************************************************************
 * Main functions
 ******************************************************************************/
void SignalHandler(int sig)
{
	LogNTC("Handling signal %d.", sig);
	SrvMain::shutdownGame();
}

int main(int argc, char* argv[])
{
	// install the signal handler
	if ((signal(SIGHUP, SignalHandler) == SIG_ERR)
	    || (signal(SIGINT,  SignalHandler) == SIG_ERR)
	    || (signal(SIGTERM, SignalHandler) == SIG_ERR)) {
		LogERR("%s: Could not install signal handler.", argv[0]);
	}

	// now init the application
	SrvMain::instance().start();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
