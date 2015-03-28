/*
 * bot.cpp
 * Copyright (C) 2006 by Bryan Duff <duff0097@umn.edu>
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

#include "common/net/msgs.h"
#include "common/configmgr.h"
#include "common/xmlmgr.h"
#include "common/tablemgr.h"

#include "bot.h"
#include "botcommand.h"
#include "botinventory.h"
#include "bot/net/botnetmgr.h"
#include "bot/action/bottradeinv.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>


fmBot* Bot = 0;

//redefined later...
void InteractiveModeReadInput();

fmBot::fmBot()
{
	mBotNetworkMgr = 0;	
	mBotCommandMgr = 0;

	autoLogin = false;
	autoJoin = false;
}

void fmBot::OnExit()
{
	#define FREE_MODULE(m) LogDBG("Deleting module: %s", #m); delete m; m = 0;

	// client
	FREE_MODULE(mBotNetworkMgr);
	FREE_MODULE(mBotCommandMgr);
}

bool fmBot::OnInitialize(int argc, char** argv)
{
	// The network message handler
	mBotNetworkMgr = new BotNetworkMgr();
	PERM_ASSERT(mBotNetworkMgr);

    // create command manager and register commands
	mBotCommandMgr = new BotCommandMgr();
	PERM_ASSERT(mBotCommandMgr);
    mBotCommandMgr->registerCommands();

	// Enable interactive commands by default
	mInteractiveMode = true;

    bool configLoaded = ConfigMgr::instance().loadConfigFile("data/bot/botdata.cfg");
    if (!configLoaded) {
        LogERR("Couldn't load config file: %s", "data/bot/botdata.cfg");
        return false;
    }

	// Run startup commands
    string scriptFile = ConfigMgr::instance().getConfigVar("Bot.Runtime.StartUpScript", "-");
    if (scriptFile == "-") {
        LogERR("Couldn't get StartUp Script file name from config file");
    } else
        loadStartupScript(scriptFile.c_str());

	TableMgr::instance().loadFromFile( "data/bot/dialog.xml" );
	//TableMgr::instance().getTable( "greeting" )->printTable();

	return true;
}

bool fmBot::Application()
{
	if ( autoConnect == true )
	{
		string host = ConfigMgr::instance().getConfigVar("Bot.Settings.Hostname", "localhost");
		int port = atoi(ConfigMgr::instance().getConfigVar("Bot.Settings.Port", "20768"));

		///Send initial connect message here.
		if (!mBotNetworkMgr->connectToServer(host.c_str(), port))
		{
			LogERR("Cannot connect to server");
		} else {
			// send initial message
			MsgConnect msg;
			mBotNetworkMgr->sendToServer(msg);
		}
	}

	//Main loop
	while (true)
	{
		Run();
	}

	return true;
}

void fmBot::Run()
{
	// Slow it down a little.
	usleep(100*1000);

    // if in interactive mode, try to read input
    if (mInteractiveMode)
        InteractiveModeReadInput();

    // process incoming messages from the network
    mBotNetworkMgr->processIncomingMsgs();
}

void fmBot::Quit()
{
	//Quiting stuff
	mBotNetworkMgr->disconnect();

	/// Cleanup
	OnExit();
	LogNTC("Bot finished shutting down");
	exit(0);
}


void fmBot::executeCommand(const char* cmd)
{
    if (!cmd || cmd[0] == '\n')
        LogWRN("Empty command, try '/help'");
    else if (cmd[0] == '/') {
        CommandOutput out;
        mBotCommandMgr->execute(cmd+1, PermLevel::ADMIN, out);
        LogNTC("CommandOutput: %s", out.getOutput().c_str() );
    }
    else
        LogWRN("Unknown command '%s' (missing /?), try '/help'", cmd);
}


bool fmBot::loadStartupScript(const char* file)
{
    LogNTC("Executing startup script: %s", file);
    string line;
    ifstream script(file);
    if (script.is_open()) {
        while (!script.eof()) {
            getline(script, line);
            StrTrim(line);
            if (line.empty())
                continue;
            else if (line.at(0) == '#')
				continue;
			else
                line.insert(0, "/");
            LogNTC("> %s", line.c_str());
            executeCommand(line.c_str());
        }
        LogNTC("Startup script completed: %s", file);
        script.close();
        return true;
    } else {
        LogERR("Unable to open script file: %s", strerror(errno));
        return false;
    }
}

BotCommandMgr* fmBot::getCommandMgr()
{
    return mBotCommandMgr;
}

BotNetworkMgr* fmBot::getNetworkMgr()
{
    return mBotNetworkMgr;
}

void InteractiveModeReadInput()
{
    // vars to store a command-line-in-progress, and the character
    // to be read
    string cmd;
    char c;

    // prepare values for select call (fd=1 is stdin)
    int fd = 1;
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(0, &fdset);
    struct timeval tspec = {0, 0};

    switch(select(fd, &fdset, 0, 0, &tspec))
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
                Bot->executeCommand(cmd.c_str());
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


//---------------------------------------------------------------------
// Main function
//---------------------------------------------------------------------
int main(int argc, char* argv[])
{
	fmBot* bot = new fmBot();
	Bot = bot;
	if (Bot->OnInitialize(argc, argv))
	{
		Bot->Application();
	}
	delete bot; bot = 0;

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
