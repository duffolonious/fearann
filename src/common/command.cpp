/*
 * command.cpp
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

#include "config.h"

#include "command.h"

#include <cctype>


//-------------------- Command ------------------------
Command::Command(PermLevel::LEVEL level,
		 const char* name,
		 const char* descr)
{
	mName = name;
	mDescr = descr;
	mPermNeeded = level;
}

const char* Command::getName() const
{
	return mName.c_str();
}

const char* Command::getDescription() const
{
	return mDescr.c_str();
}

void Command::getArgNames(vector<string>& argNames) const
{
	argNames = mArgNames;
}

bool Command::permissionAllowed(PermLevel::LEVEL level) const
{
	return mPermNeeded <= level;
}

void Command::execute(vector<string>& args,
		      PermLevel::LEVEL level,
		      CommandOutput& output)
{
	// check for permissions
	if (!permissionAllowed(level)) {
		string msg = StrFmt("Insufficient permissions to execute command '%s'.",
				    mName.c_str());
		output.appendLine(msg.c_str());
		return;
	}

	// now execute the command, defined in derived classes
	execute(args, output);
}


//------------------------ CommandMgr ---------------------------
CommandMgr::CommandMgr()
{
}

CommandMgr::~CommandMgr()
{
	for (map<string, Command*>::iterator it = mCommandList.begin();
	     it != mCommandList.end(); ++it) {
		delete (*it).second;
	}
}

void CommandMgr::addCommand(Command* command)
{
	string commandName(command->getName());
	if (findCommand(commandName)) {
		LogWRN("CommandMgr: Adding command to set: '%s' already exists",
		       command->getName());
	} else {
		mCommandList[commandName] = command;
	}
}

Command* CommandMgr::findCommand(const string& commandName) const
{
	// the <map> returns the last element when the element with the key is
	// not found, so we have to make this to behave like usual functions and
	// not to do this special check on the caller. IMPORTANT: note that
	// map[] returns the element when it *exists*, but otherwise creates a
	// new entry with an empty value, so be careful with this
	if (mCommandList.find(commandName) == mCommandList.end()) {
		return 0;
	} else {
		return (*mCommandList.find(commandName)).second;
	}
}

void CommandMgr::parseCommandLine(const char* cL, vector<string>& args)
{
	string commandLine(cL);

	// tokenize the string into arguments
	while (commandLine.length() > 0) {
		// remove whitespace around
		StrTrim(commandLine);
		// LogDBG("cL before tokenizing: '%s'", commandLine.c_str());

		string::size_type pos = commandLine.find_first_of(' ');
		if (pos != string::npos) {
			string newArg = commandLine.substr(0, pos);
			args.push_back(newArg);
			commandLine = commandLine.substr(pos, commandLine.length()-1);
			// LogDBG("Argument recognized: '%s'", newArg.c_str());
		} else {
			// no spaces left, last argument
			args.push_back(commandLine);
			break;
		}
	}
}

void CommandMgr::execute(const char* commandLine,
			 PermLevel::LEVEL level,
			 CommandOutput& output)
{
	// try to parse the command line into arguments
	vector<string> args;
	parseCommandLine(commandLine, args);
	if (args.size() == 0) {
		string msg = StrFmt("Cannot parse: '%s' (too long?)", commandLine);
		output.appendLine(msg);
		return;
	}

	// pop the command name
	string commandName = args[0];
	args.erase(args.begin());

	// we handle here the /help meta command
	if (commandName == "help") {
		if (args.size() == 0) {
			showHelp(level, output);
		} else {
			// only considering one argument
			showHelp(args[0], level, output);
		}
		return;
	}

	// search for a "real" command
	Command* command = findCommand(commandName);
	if (!command) {
		string msg = StrFmt("No such command '%s', try '/help' for a list.",
				    commandName.c_str());
		output.appendLine(msg);
	} else if (!command->permissionAllowed(level)) {
		string msg = StrFmt("Not allowed to execute command '%s'.",
				    commandName.c_str());
		output.appendLine(msg);
	} else {
		// execute the command at last
		command->execute(args, level, output);
	}
}

void CommandMgr::showHelp(const PermLevel::LEVEL level, CommandOutput& output)
{
	// header
	string msg = StrFmt("Available commands (%zu total)", mCommandList.size());
	output.appendLine(msg);
	// short description of the existing commands (for the given level) in
	// the middle
	for (map<string, Command*>::iterator it = mCommandList.begin();
	     it != mCommandList.end(); ++it) {
		if ((*it).second->permissionAllowed(level)) {
			showHelp((*it).first, level, output);
		}
	}
	// footer
	output.appendLine(string("Use /help <command> for more info."));
}

void CommandMgr::showHelp(const string& commandName,
			  const PermLevel::LEVEL level,
			  CommandOutput& output)
{
	// get the command and check if exists
	Command* command = findCommand(commandName);
	if (!command || !command->permissionAllowed(level)) {
		string msg = StrFmt("No help on '%s'. Try '/help' for a list.",
				    commandName.c_str());
		output.appendLine(msg);
		return;
	}

	// add to output name and arguments
	string line = "  ";
	line += command->getName();
	vector<string> argNames;
	command->getArgNames(argNames);
	for (size_t i = 0; i < argNames.size(); ++i) {
		line += " <";
		line += argNames[i];
		line += ">";
	}

	// "tab" to column 40 in a line
	while (line.size() < 40)
		line += " ";

	// add short description
	line += command->getDescription();
	output.appendLine(line);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
