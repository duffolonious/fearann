/*
 * cltcommand.cpp
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

#include "common/net/msgs.h"

#include "client/net/cltnetmgr.h"

#include "cltcommand.h"


/** Send a command to the server
 */
class CltCommandServer : public Command
{
public:
	CltCommandServer() :
		Command(PermLevel::PLAYER,
			"server",
			"Send a command to the server") {
		mArgNames.push_back(string("server_cmd"));
		mArgNames.push_back(string("..."));
	}

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() < 1) {
			out.appendLine("This command requires at least one argument");
			return;
		}

		MsgCommand msg;
		msg.command = args[0];
		for (size_t i = 1; i < args.size(); ++i) {
			msg.command.append(" ");
			msg.command.append(args[i]);
		}

		CltNetworkMgr::instance().sendToServer(msg);
	}
};


/** Say command, saying a phrase in the local chat channel (the real action
 * might depend on the channel, distance with other players, and other
 * restrictions imposed by the server).
 */
class CltCommandSay : public Command
{
public:
	CltCommandSay() :
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
		for (size_t i = 0; i < args.size(); ++i) {
			msg.text.append(args[i]);
			msg.text.append(" ");
		}

		if (!msg.text.empty())
			CltNetworkMgr::instance().sendToServer(msg);
		else
			out.appendLine("Refusing to send an empty say message");
	}
};


/** Private message to some player
 */
class CltCommandPrivateMessage : public Command
{
public:
	CltCommandPrivateMessage() :
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
		for (size_t i = 1; i < args.size(); ++i) {
			msg.text.append(args[i]);
			msg.text.append(" ");
		}

		if (!msg.text.empty())
			CltNetworkMgr::instance().sendToServer(msg);
		else
			out.appendLine("Refusing to send an empty private message");
	}
};


/** List players in the server.
 */
class CltCommandWho : public Command
{
public:
	CltCommandWho() :
		Command(PermLevel::PLAYER,
			"who",
			"Show the current players in the server") { }

	virtual void execute(vector<string>& args, CommandOutput& out) {
		if (args.size() != 0) {
			out.appendLine("/who command doesn't require arguments, ignoring");
		}

		MsgCommand msg;
		msg.command = "who";
		CltNetworkMgr::instance().sendToServer(msg);
	}
};


//----------------------- CltCommandMgr ----------------------------
template <> CltCommandMgr* Singleton<CltCommandMgr>::INSTANCE = 0;

CltCommandMgr::CltCommandMgr()
{
	registerCommands();
}

CltCommandMgr::~CltCommandMgr()
{
}

void CltCommandMgr::registerCommands()
{
	// mafm: The procedure is quite simple, for each command that we want to
	// support we have to create an instance and add it to the manager, the
	// base class takes care of the rest (including the deletion of the
	// instances in the destructor).  The data for each command is
	// especified in the constructor, and the argument names are a
	// convenience way to show some understandable info in the help (such as
	// "pm target ...", indicating that you have to especify the target
	// player and then whatever you want to tell her/him).

	addCommand(new CltCommandServer());
	addCommand(new CltCommandSay());
	addCommand(new CltCommandPrivateMessage());
	addCommand(new CltCommandWho());
}




// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
