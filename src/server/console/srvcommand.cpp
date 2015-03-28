/*
 * srvcommand.cpp
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

#include "srvcommand.h"

#include "common/net/msgs.h"
#include "common/logmgr.h"
#include "common/tablemgr.h"

#include "server/srvmain.h"
#include "server/content/srvcontentmgr.h"
#include "server/login/srvloginmgr.h"
#include "server/net/srvnetworkmgr.h"
#include "server/world/srvworldtimemgr.h"
#include "server/world/srvworldmgr.h"
#include "server/action/srvcombatmgr.h"

#include <cstdlib>


/** Quit, stop the server
 */
class SrvCommandQuit : public Command
{
public:
	SrvCommandQuit() :
		Command(PermLevel::ADMIN,
			  "quit",
			  "Stop the server") {
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 0) {
			out.appendLine("This command doesn't accept arguments, ignoring");
		}

		MsgChat msg;
		msg.origin = "Server";
		msg.text = "Server is shutting down by admin request";
		msg.type = MsgChat::SYSTEM;
		SrvNetworkMgr::instance().sendToAllConnections(msg);

		out.appendLine("Server is shutting down by admin request");
		SrvMain::shutdownGame();
	}
};


/** Show some statistics and info about the game
 */
class SrvCommandShowStats : public Command
{
public:
	SrvCommandShowStats() :
		Command(PermLevel::ADMIN,
			  "show_stats",
			  "Show some statistics and info about the game") {
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 0) {
			out.appendLine("This command doesn't accept arguments, ignoring");
		}

		string uptime = SrvMain::instance().getUptime();
		string gameTime = StrFmt("%u", SrvWorldTimeMgr::instance().getGameTime());
		int numAccts = SrvLoginMgr::instance().getNumberOfAccounts();
		int numChars = SrvLoginMgr::instance().getNumberOfCharacters();
		int numPlayers = SrvLoginMgr::instance().getNumberOfConnectionsPlaying();
		out.appendLine(StrFmt("Server uptime: %s", uptime.c_str()));
		out.appendLine(StrFmt("Game time: %s", gameTime.c_str()));
		out.appendLine(StrFmt("Number of accounts: %d", numAccts));
		out.appendLine(StrFmt("Number of characters: %d", numChars));
		out.appendLine(StrFmt("Number of players: %d", numPlayers));
	}
};


/** Change the time in the server
 */
class SrvCommandChangeTime : public Command
{
public:
	SrvCommandChangeTime() :
		Command(PermLevel::ADMIN,
			  "changetime",
			  "Change time by the given minutes") {
		mArgNames.push_back(string("+-minutes"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() != 1) {
			out.appendLine("This command needs exactly one argument, aborting");
			return;
		}

		int minutes = atoi(args[0].c_str());
		string text = StrFmt("Time changed by %d minutes", minutes);
		out.appendLine(text.c_str());
		SrvWorldTimeMgr::instance().changeTime(minutes);

		MsgChat msg;
		msg.origin = "Server";
		msg.type = MsgChat::SYSTEM;
		msg.text = text;
		SrvNetworkMgr::instance().sendToAllPlayers(msg);
	}
};


/** Load an area. */
class SrvCommandLoadArea : public Command
{
public:
	SrvCommandLoadArea() :
		Command(PermLevel::ADMIN,
			  "load_area",
			  "Load an area") {
		mArgNames.push_back(string("area"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() != 1) {
			out.appendLine("This command needs exactly one argument, aborting");
			return;
		}

		if (!SrvWorldMgr::instance().loadArea(args[0]))
			out.appendLine(StrFmt("ERROR: loading area '%s' (already exists?)", args[0].c_str()));
		else
			out.appendLine(StrFmt("Area loaded successfully: %s", args[0].c_str()));
	}
};


/** Load objects. */
class SrvCommandLoadObjects : public Command
{
public:
	SrvCommandLoadObjects() :
		Command(PermLevel::ADMIN,
			  "load_objects",
			  "Load objects for the given area") {
		mArgNames.push_back(string("area"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() != 1) {
			out.appendLine("This command needs exactly one argument, aborting");
			return;
		}

		if (!SrvWorldMgr::instance().loadObjectsFromDB(args[0]))
			out.appendLine("ERROR: loading objects from DB (already loaded?)");
		else
			out.appendLine("Objects loaded successfully from DB");
	}
};


/** Load creatures. */
class SrvCommandLoadCreatures : public Command
{
public:
	SrvCommandLoadCreatures() :
		Command(PermLevel::ADMIN,
			  "load_creatures",
			  "Load creatures for the given area") {
		mArgNames.push_back(string("area"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() != 1) {
			out.appendLine("This command needs exactly one argument, aborting");
			return;
		}

		if (!SrvWorldMgr::instance().loadCreaturesFromDB(args[0]))
			out.appendLine("ERROR: loading creatures from DB (already loaded?)");
		else
			out.appendLine("Creatures loaded successfully from DB");
	}
};


/** Load tables. */
class SrvCommandLoadTable : public Command
{
public:
	SrvCommandLoadTable() :
		Command(PermLevel::ADMIN,
			  "load_table",
			  "Load game data tables") {
		mArgNames.push_back(string("table"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() != 1) {
			out.appendLine("This command needs exactly one argument, aborting");
			return;
		}

		bool loaded = TableMgr::instance().loadFromFile(args[0].c_str());
		if (loaded) {
			out.appendLine("Table loaded successfully");
		} else {
			out.appendLine("ERROR: table coudln't be loaded");
		}
	}
};


/** Reload content to be sent to clients. */
class SrvCommandReloadContent : public Command
{
public:
	SrvCommandReloadContent() :
		Command(PermLevel::ADMIN,
			  "reload_content",
			  "Reload the content to be sent to clients") {
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 0) {
			out.appendLine("This command doesn't accept arguments, ignoring");
		}

		if (!SrvContentMgr::instance().reloadContentTree())
			out.appendLine("ERROR: there was a problem reloading content (active transfers?)");
		else
			out.appendLine("Content tree for client reloaded sucessfully.");
	}
};


/** Reload content to be sent to clients. */
class SrvCommandCombat : public Command
{
public:
	SrvCommandCombat() :
		Command(PermLevel::ADMIN,
			  "combat",
			  "combat commands") {
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 0) {
			out.appendLine("This command doesn't accept arguments, ignoring");
		}

		SrvCombatMgr::instance().listBattles();
	}
};


/** Who, to list the players in the server.  When the client sends a /who
 * command it's redirected to this one too. */
class SrvCommandWho : public Command
{
public:
	SrvCommandWho() :
		Command(PermLevel::PLAYER,
			  "who",
			  "Show the current players") {
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 0) {
			out.appendLine("This command doesn't accept arguments, ignoring");
		}

		// printing 6 names per line, truncated to 12 characters
		unsigned int namesPerLine = 6;
		unsigned int namesCurLine = 0;

		// get the raw list and sort it
		list<string> nameList;
		SrvLoginMgr::instance().getPlayerNameList(nameList);
		nameList.sort();

		// header
		out.appendLine(StrFmt("Total number of players: %zu", nameList.size()));

		string line = "  ";
		for (list<string>::iterator it = nameList.begin(); it != nameList.end(); ++it) {
			// max of 12 chars per name, fill if doesn't reach it
			line += (*it).substr(0, 12);
			for (int i = 12-(*it).size(); i >= 0; i--) {
				line += " ";
			}
			++namesCurLine;
			if (namesCurLine >= namesPerLine) {
				out.appendLine(StrFmt("  %s", line.c_str()));
				namesCurLine = 0;
				line.clear();
			}
		}
		out.appendLine(line.c_str());
	}
};


/** Modify server's log message level. */
class SrvCommandSetLogLevel : public Command
{
public:
	SrvCommandSetLogLevel() :
		Command(PermLevel::ADMIN, "loglevel", "Modify the Log Message level"){
		mArgNames.push_back(string("DEBUG|INFO|WARNING|ERROR|FATAL"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() != 1) {
			out.appendLine("This command needs exactly one argument ( which must be DBG, NTC, WN or ERR ), aborting");
			return;
		}

		if (!LogMgr::instance().setLogMsgLevel(args[0].c_str())) {
			out.appendLine(StrFmt("ERROR: modifying log level '%s' (wrong parameter?)", args[0].c_str()));
		} else {
			out.appendLine(StrFmt("Log level %s succesfully set", args[0].c_str()));
		}
	}
};


/*******************************************************************************
 * SrvCommandMgr
 ******************************************************************************/
template <> SrvCommandMgr* Singleton<SrvCommandMgr>::INSTANCE = 0;

SrvCommandMgr::SrvCommandMgr()
{
	registerCommands();
}

void SrvCommandMgr::registerCommands()
{
	// mafm: The procedure is quite simple, for each command that we want to
	// support we have to create an instance and add it to the manager, the
	// base class takes care of the rest (including the deletion of the
	// instances in the destructor).  The data for each command is
	// especified in the constructor, and the argument names are a
	// convenience way to show some understandable info in the help (such as
	// "pm target ...", indicating that you have to especify the target
	// player and then whatever you want to tell her/him).

	// admin commands
	addCommand(new SrvCommandQuit());
	addCommand(new SrvCommandShowStats());
	addCommand(new SrvCommandChangeTime());
	addCommand(new SrvCommandLoadArea());
	addCommand(new SrvCommandLoadObjects());
	addCommand(new SrvCommandLoadCreatures());
	addCommand(new SrvCommandLoadTable());
	addCommand(new SrvCommandReloadContent());
	addCommand(new SrvCommandCombat());
	addCommand(new SrvCommandSetLogLevel());

	// player commands
	addCommand(new SrvCommandWho());
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
