/*
 * botcommand.cpp
 * Copyright (C) 2006 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *						Bryan Duff <duff0097@umn.edu>
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
#include "common/logmgr.h"
#include "common/xmlmgr.h"
#include "common/sha1.h"
#include "common/util.h"

#include "bot/net/botnetmgr.h"

#include "bot.h"
#include "botcommand.h"
#include "botinventory.h"
#include "action/bottradeinv.h"
#include "action/botmove.h"

#include <cstdlib>


/** Quit, stop the server
 */
class BotCommandQuit : public Command
{
public:
	BotCommandQuit() :
		Command(PermLevel::ADMIN,
			  "quit",
			  "Stop the bot") {
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 0) {
			out.appendLine("This command doesn't accept arguments, ignoring");
		}
		out.appendLine("Bot shutting down...");
		Bot->Quit();
	}
};


class BotCommandAuto : public Command
{
public:
	BotCommandAuto() :
		Command(PermLevel::PLAYER,
			  "auto",
			  "Automatic commands - for startup script only")
	{
		mArgNames.push_back(string("connect|login|join"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 2) {
			out.appendLine("This command accepts only 2 arguments maximum");
		}

		if (args[0].compare("connect") == 0)
		{
			Bot->setAutoConnect(true);
			out.appendLine("Auto connect enabled");
		} 
		else if (args[0].compare("login") == 0)
		{
			Bot->setAutoLogin(true);
			out.appendLine("Auto login enabled");
		} 
		else if (args[0].compare("join") == 0)
		{
			Bot->setAutoJoin(true);
			out.appendLine("Auto join enabled");
		}
		else if (args[0].compare("mode") == 0)
		{
			if (args[1].compare("npc") == 0)
			{
				Bot->setMode(fmBot::NPC);
				out.appendLine("NPC mode enabled");
			}
		}
	}
};


class BotCommandConnect : public Command
{
public:
	BotCommandConnect() :
		Command(PermLevel::PLAYER,
			  "connect",
			  "Connect to the server")
	{
		mArgNames.push_back(string("host|list"));
		mArgNames.push_back(string("port"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 2 || args.size() == 0) {
			out.appendLine("/connect host port || /connect config");
		}

		string host;
		int port;
		if ( args[0].compare("config") == 0 )
		{
			host = ConfigMgr::instance().getConfigVar("Bot.Settings.Hostname", "localhost");
			port = atoi(ConfigMgr::instance().getConfigVar("Bot.Settings.Port", "20768"));
		}
		else if ( args[0].compare("list") == 0 )
		{
			out.appendLine("Listing servers...");
			const XMLNode * list = XMLMgr::instance().loadXMLFile("data/client/servers.xml");
			LogDBG("node name: %s, children: %d", list->getName(), list->getChildListSize() );
			for (int i = 0; i < list->getChildListSize(); i++)
			{
				const XMLNode * tmp = list->getChildAt( i );
				if ( !tmp )
					continue;
				LogNTC("%s - name = %s, port = %d",
                    tmp->getName(),
                    tmp->getAttrValueAsStr("name").c_str(),
										tmp->getAttrValueAsInt("port") );
				delete tmp; tmp = 0;
			}
			XMLMgr::instance().clear();
			delete list;
			return;
		}
		else
		{
			host = args[0].c_str();
			port = atoi(args[1].c_str());
		}

		out.appendLine("Bot Connecting...");
    	///Send initial connect message here.
	    if (!Bot->getNetworkMgr()->connectToServer(host.c_str(), port))
    	{
	        LogERR("Cannot connect to server");
    	} else {
        	// send initial message
	        MsgConnect msg;
    	    Bot->getNetworkMgr()->sendToServer(msg);
	    }
	}
};


class BotCommandLogin : public Command
{
public:
	BotCommandLogin() :
		Command(PermLevel::PLAYER,
			  "login",
			  "Login to the server")
	{
		mArgNames.push_back(string("config || [username password]"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 2) {
			out.appendLine("/login username password || /login config");
		}

		if (args.front().compare("config") == 0)
		{
			out.appendLine("Bot logging in using config...");
			MsgLogin login;
			login.username = ConfigMgr::instance().getConfigVar("Bot.Settings.User", "");
			login.pw_md5sum = ConfigMgr::instance().getConfigVar("Bot.Settings.Password", "");
			Bot->getNetworkMgr()->sendToServer(login);
		}
		else if (args.size() == 2)
		{
			out.appendLine("Bot logging in...");
			MsgLogin login;
			string tmp;
			login.username = args[0];
			SHA1::encode(args[1].c_str(), tmp);
			LogDBG("sha1 hash: '%s'\n", tmp.c_str() );
			login.pw_md5sum = tmp;
			Bot->getNetworkMgr()->sendToServer(login);
		}
		else
		{
			out.appendLine("Login command invalid");		
		}
	}
};


class BotCommandJoin : public Command
{
public:
	BotCommandJoin() :
		Command(PermLevel::PLAYER,
			  "join",
			  "Join the world")
	{
		mArgNames.push_back(string("character name"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 1) {
			out.appendLine("This command doesn't accept arguments, ignoring");
		}
		out.appendLine("Joining the world...");
		MsgJoin join;
		join.charname = args.front();
		Bot->setName( args.front().c_str() );
		Bot->getNetworkMgr()->sendToServer(join);
	}
};


class BotCommandSay : public Command
{
public:
    BotCommandSay() :
        Command(PermLevel::PLAYER,
              "say",
              "Send a chat message to the chat channel") {
        mArgNames.push_back(string("..."));
    }

    virtual void execute(vector<string>& args, CommandOutput& out) {
        if (args.size() < 1) {
            out.appendLine("This command requires at least one argument");
            return;
        }

        MsgChat msg;
        for (size_t i = 0; i < args.size(); i++) {
            msg.text.append(args[i]);
            msg.text.append(" ");
        }

        if (!msg.text.empty())
            Bot->getNetworkMgr()->sendToServer(msg);
        else
            out.appendLine("Refusing to send an empty say message");
    }
};


/** Private message to some player
 */
class BotCommandPrivateMessage : public Command
{
public:
    BotCommandPrivateMessage() :
        Command(PermLevel::PLAYER,
              "pm",
              "Sends a private message to another player") {
        mArgNames.push_back(string("target"));
        mArgNames.push_back(string("..."));
    }

    virtual void execute(vector<string>& args, CommandOutput& out) {
        if (args.size() < 2) {
            out.appendLine("This command requires at least two arguments");
            return;
        }

        MsgChat msg;
        msg.target = args[0];
        for (size_t i = 1; i < args.size(); i++) {
            msg.text.append(args[i]);
            msg.text.append(" ");
        }

        if (!msg.text.empty())
            Bot->getNetworkMgr()->sendToServer(msg);
        else
            out.appendLine("Refusing to send an empty private message");
    }
};


/** Pickup item in the world - still need to be near it.
 */
class BotCommandPickup : public Command
{
public:
    BotCommandPickup() :
        Command(PermLevel::PLAYER,
              "pickup",
              "Attempts to pickup item") {
        mArgNames.push_back(string("item #"));
    }

    virtual void execute(vector<string>& args, CommandOutput& out) {
        if (args.size() < 1) {
            out.appendLine("This command requires one argument");
            return;
        }
		out.appendLine("Attempting pickup...");
        MsgInventoryGet msg;
		msg.itemID = atoi(args[0].c_str());

		Bot->getNetworkMgr()->sendToServer(msg);
    }
};


/** Drop item in the world.
 */
class BotCommandDrop : public Command
{
public:
    BotCommandDrop() :
        Command(PermLevel::PLAYER,
              "drop",
              "Attempts to drop an item") {
        mArgNames.push_back(string("item #"));
    }

    virtual void execute(vector<string>& args, CommandOutput& out) {
        if (args.size() < 1) {
            out.appendLine("This command requires one argument");
            return;
        }

		out.appendLine("Attempting drop...");

        MsgInventoryDrop msg;
		msg.itemID = atoi(args[0].c_str());

		Bot->getNetworkMgr()->sendToServer(msg);
    }
};


/** Drop item in the world.
 */
class BotCommandInvList : public Command
{
public:
    BotCommandInvList() :
        Command(PermLevel::PLAYER,
              "list",
			  "List inventory contents") {
    }

    virtual void execute(vector<string>& args, CommandOutput& out) {
        if (args.size() > 0) {
            out.appendLine("This command had no arguments");
            return;
        }

		out.appendLine("Listing inventory...");
		BotInventory::instance().ListInventory();
    }
};


/** Trade command.
 */
class BotCommandTrade : public Command
{
public:
    BotCommandTrade() :
        Command(PermLevel::PLAYER,
              "trade",
              "trade with another player") {
        mArgNames.push_back(string("target"));
        mArgNames.push_back(string("state"));
    }

    virtual void execute(vector<string>& args, CommandOutput& out) {
        if (args.size() < 1) {
            out.appendLine("This command requires at least one argument");
            return;
        }

		/** duffolonious: the trading messages are as follows:
		* /trade start <target> - start a trade with the target player.
		* /trade end - end trade (or decline trade).
		* /trade accept - target accepts trade with you.
		* /trade update <id> <id> <id> ... - update trading inventory.
		* /trade select player <id> ... target <id> ... - select items.
		* /trade commit - initiate trade commit.
		* /trade commit_accept - accept and complete trade.
		* /trade commit_reject - reject trade.
		* /trade status
		*/

        MsgTrade msg;
		msg.type = MsgTrade::START;
		msg.listSize = msg.plListSize = msg.tgListSize = 0;

		//LogDBG("number of args: %d", args.size());

		std::string tmp = args[0];
		if (tmp.compare("start") == 0)
		{
			const char* target = args[1].c_str();
			msg.type = MsgTrade::START;
			msg.target = target;
			/// Add initial trade info.
			BotTradeInv::instance().setState( MsgTrade::START );
			BotTradeInv::instance().setTarget( target );
		}
		else if (tmp.compare("end") == 0)
		{
			msg.type = MsgTrade::END;
			/// Clear trade info.
			BotTradeInv::instance().setState( MsgTrade::END );
		}
		else if (tmp.compare("accept") == 0)
		{
			const char* target = args[1].c_str();
			msg.type = MsgTrade::ACCEPT;
			/// Add initial trade info.
			BotTradeInv::instance().setState( MsgTrade::ACCEPT );
			BotTradeInv::instance().setTarget( target );
		}
		else if (tmp.compare("update") == 0)
		{
			msg.type = MsgTrade::UPDATE_LIST;
			msg.target = BotTradeInv::instance().getTarget();
			for( size_t i = 1; i < args.size(); i++ )
			{
				int itemid = atoi(args[i].c_str());
				InventoryItem* item = BotInventory::instance().
															getItem( itemid );
				if (!item)
					continue;
				msg.addItem( item );
				BotTradeInv::instance().AddItem( item, false );
				LogDBG("Adding item %d to the update list.", itemid);
			}
			LogDBG("msg itemList size: %lu", msg.itemList.size() );
		}
		else if (tmp.compare("select") == 0)
		{
			//Select items to trade
			bool player = false;
			for( size_t i = 1; i < args.size(); i++ )
			{
				LogDBG("select arg: '%s'", args[i].c_str() );
				if ( args[i].compare("player") == 0 )
				{
					player = true;
					continue;
				}

				if ( args[i].compare("target") == 0 )
				{
					player = false;
					continue;
				}

				if ( player )
				{
					int itemid = atoi(args[i].c_str());
					LogDBG("player selected items...");
					if (BotTradeInv::instance().hasItem( itemid ) )
					{
						LogDBG("itemid %d selected", itemid);
						msg.addPlayerSelectedItem( itemid );
						BotTradeInv::instance().selectItem( itemid );
					}
				}
				else
				{
					int itemid = atoi(args[i].c_str());
					LogDBG("target selected items...");
					if (BotTradeInv::instance().hasItem( itemid ) )
					{
						msg.addTargetSelectedItem( itemid );
						BotTradeInv::instance().selectItem( itemid );
					}
				}
			}
			return;
		}
		else if (tmp.compare("commit") == 0)
		{
			msg.type = MsgTrade::COMMIT;
			msg.target = BotTradeInv::instance().getTarget();

			/// Add selected items
			vector<int> playerSelInv = BotTradeInv::instance().playerSelectedInventory;
			vector<int> targetSelInv = BotTradeInv::instance().targetSelectedInventory;
			vector<int>::iterator it;
			for (it = playerSelInv.begin(); it != playerSelInv.end(); it++)
	        {
				int itemID = *it;
        	    LogDBG("adding player selected item: %d", *it );
				msg.addPlayerSelectedItem( itemID );
	        }

			for (it = targetSelInv.begin(); it != targetSelInv.end(); it++)
	        {
				int itemID = *it;
        	    LogDBG("adding target selected item: %d", *it );
				msg.addTargetSelectedItem( itemID);
	        }
		}
		else if (tmp.compare("commit_accept") == 0)
		{
			msg.type = MsgTrade::COMMIT_ACCEPT;
			msg.target = BotTradeInv::instance().getTarget();
		}
		else if (tmp.compare("commit_reject") == 0)
		{
			msg.type = MsgTrade::COMMIT_REJECT;
			msg.target = BotTradeInv::instance().getTarget();
		}
		else if (tmp.compare("status") == 0)
		{
			/// output status info.
			BotTradeInv::instance().ListInventory();
			out.appendLine("Trade status.");
			return;
		}
		else
		{
			return;
		}

		out.appendLine("Trade message sent.");

		Bot->getNetworkMgr()->sendToServer(msg);
    }
};


class BotCommandCombat : public Command
{
public:
	BotCommandCombat() :
		Command(PermLevel::PLAYER,
			  "combat",
			  "combat actions")
	{
		mArgNames.push_back(string(""));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() > 2) {
			out.appendLine("");
		}

		/** duffolonious: the combat commands are as follows:
		* /combat start <target> <duel>- initiate (duel's only atm)
		* /combat end <target> - end combat (you can't if you've accepted).
		* /combat accept <target> (duel's only)
		* /combat <action> - only current action is attack (default action).
		*/

		MsgCombat msg;
		msg.target = StrToUInt64( args[1].c_str() );

		if (args[0].compare("start") == 0)
		{
			LogNTC("Starting new combat with '%lu'", msg.target );
			// start combat with another player
			//if ( args[2].compare("duel") )
			msg.type = MsgCombat::DUEL;
			//else
			//	msg.type = MsgCombat::NORMAL;

			msg.state = MsgCombat::START;
		}
		else if (args[0].compare("end") == 0)
		{
			// deny combat
			msg.state = MsgCombat::END;
		}
		else if (args[0].compare("accept") == 0)
		{
			//Accept combat from another player
			msg.state = MsgCombat::ACCEPTED;
		}
		else
		{
			out.appendLine("Combat command invalid");
			return;
		}

		Bot->getNetworkMgr()->sendToServer(msg);
	}
};


/** Move to coords in world.
 */
class BotCommandLook : public Command
{
public:
    BotCommandLook() :
        Command(PermLevel::PLAYER,
              "look",
			  "look at coords.") {
        mArgNames.push_back(string("x coord"));
        mArgNames.push_back(string("y coord"));
    }

    virtual void execute(vector<string>& args, CommandOutput& out) {
        if (args.size() < 2) {
            out.appendLine("This command requires 2 arguments");
            return;
        }

		LogNTC("Look at coords: ");

    MsgEntityMove msg;

    float x = atof(args[0].c_str());
    float y = atof(args[1].c_str());
    Vector3 position(x, y, 0.0f);
//set rotation and movement.
    BotMoveMgr::instance().lookAt(position);

    msg = BotMoveMgr::instance().getMove();

		Bot->getNetworkMgr()->sendToServer(msg);
    }
};


/** Move to coords in world.
 */
class BotCommandMoveList : public Command
{
public:
    BotCommandMoveList() :
        Command(PermLevel::PLAYER,
              "movelist",
			  "List more info.") {
    }

    virtual void execute(vector<string>& args, CommandOutput& out) {
        if (args.size() > 0) {
            out.appendLine("This command has no arguments");
        }

		BotMoveMgr::instance().listMoveInfo();
    }
};


/** Move to coords in world.
 */
class BotCommandSetBound : public Command
{
public:
    BotCommandSetBound() :
        Command(PermLevel::PLAYER,
              "bound",
			  "Set bot bounds.") {
    }

    virtual void execute(vector<string>& args, CommandOutput& out) {
        if (args.size() < 6) {
            out.appendLine("This command requires 6 arguments");
            return;
        }

    float x1 = atof(args[0].c_str());
    float y1 = atof(args[1].c_str());
    Vector3 position1(x1, y1, 0.0f);
    float x2 = atof(args[2].c_str());
    float y2 = atof(args[3].c_str());
    Vector3 position2(x2, y2, 0.0f);
    float x3 = atof(args[4].c_str());
    float y3 = atof(args[5].c_str());
    Vector3 position3(x3, y3, 0.0f);
		BotMoveMgr::instance().setZone(position1, position2, position3);
    }
};


/** Modify server's log message level. */
class BotCommandSetLogLevel : public Command{
	public:
	BotCommandSetLogLevel() :
	Command(PermLevel::ADMIN, "loglevel", "Modify the Log Message level"){
		mArgNames.push_back(string("DEBUG|INFO|WARNING|ERROR"));
	}

	virtual void execute(vector<string>& args, CommandOutput& out){
		if (args.size() != 1){
			out.appendLine("This command needs exactly one argument ( which must be DEBUG, INFO, WARNING or ERROR ), aborting");
			return;
		}

		if (!LogMgr::instance().setLogMsgLevel(args[0].c_str()))
			out.appendLine(StrFmt("ERROR: modifying log level '%s' (wrong parameter?)", args[0].c_str()));
		else
			out.appendLine(StrFmt("Log level %s succesfully set", args[0].c_str()));
	}
};


void BotCommandMgr::registerCommands()
{
	// mafm: The procedure is quite simple, for each command that
	// we want to support we have to create an instance and add it
	// to the manager, the base class takes care of the rest
	// (including the deletion of the instances in the
	// destructor).  The data for each command is especified in
	// the constructor, and the argument names are a convenience
	// way to show some understandable info in the help (such as
	// "pm target ...", indicating that you have to especify the
	// target player and then whatever you want to tell her/him).

	// bot commands
	addCommand(new BotCommandQuit());
	addCommand(new BotCommandAuto());
	addCommand(new BotCommandConnect());
	addCommand(new BotCommandLogin());
	addCommand(new BotCommandJoin());
	addCommand(new BotCommandSay());
	addCommand(new BotCommandPrivateMessage());
	addCommand(new BotCommandPickup());
	addCommand(new BotCommandDrop());
	addCommand(new BotCommandInvList());
	addCommand(new BotCommandTrade());
	addCommand(new BotCommandCombat());
	addCommand(new BotCommandLook());
	addCommand(new BotCommandMoveList());
	addCommand(new BotCommandSetBound());
	addCommand(new BotCommandSetLogLevel());
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
