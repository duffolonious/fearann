/*
 * command.h
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

#ifndef __FEARANN_COMMON_COMMAND_H__
#define __FEARANN_COMMON_COMMAND_H__

#include <cstdarg>
#include <vector>
#include <map>

/** Permission level, to execute commands
 */
class PermLevel
{
public:
	enum LEVEL
		{
			NOTSET = 0,
			PLAYER,
			ADMIN
		};
};


/** Output for a command
 */
class CommandOutput
{
public:
	/** Append given string to the output of a command */
	void appendLine(const std::string& line) {
		if (!textPool.empty())
			textPool.append("\n");
		textPool.append(line);
	}
	/** Get the output of a command */
	const string& getOutput() const { return textPool; }

private:
	/// Pool of text to send
	std::string textPool;
};


/** A base class of a command, needs to define the execute() method, and also
 * create a constructor to call the constructor of the base class with the
 * appropriate parameters (constructors themselves cannot be virtual).
 */
class Command
{
public:
	/** Constructor with some basics needed when creating any command. */
	Command(PermLevel::LEVEL level,
		const char* name,
		const char* descr);
	/** Destructor */
	virtual ~Command() { }

	/** The name of the command */
	const char* getName() const;
	/** Get a one-line description of the command */
	const char* getDescription() const;
	/** Get the name of the arguments */
	void getArgNames(std::vector<std::string>& argNames) const;
	/** Check for permissions (true if allowed to execute the command with
	 * the given level) */
	bool permissionAllowed(PermLevel::LEVEL level) const;
	/** Execute the command (fake, calls the private function of the same
	 * name that will be defined in the derived classes, and performs the
	 * permission check and maybe other) */
	void execute(std::vector<std::string>& args,
		     PermLevel::LEVEL level,
		     CommandOutput& output);
protected:
	/// The name of the command
	std::string mName;
	/// Description, one line only if possible 
	std::string mDescr;
	/// The names of the arguments
	std::vector<std::string> mArgNames;
	/// Permission required to use the command
	PermLevel::LEVEL mPermNeeded;

private:
	/** Execute the command */
	virtual void execute(std::vector<std::string>& args,
			     CommandOutput& output) = 0;
};


/** A command manager, meant to be used in client and server, each one with a
 * unique set of commands.  It has to be completed to be instantiated.
 */
class CommandMgr
{
public:
	/** Execute the given command line, with given permission, and add the
	 * result to the given output. */
	void execute(const char* commandLine,
		     PermLevel::LEVEL level,
		     CommandOutput& output);
	/** Show global help */
	void showHelp(const PermLevel::LEVEL level, CommandOutput& output);
	/** Show help about the given command */
	void showHelp(const std::string& commandName,
		      const PermLevel::LEVEL level,
		      CommandOutput& output);

protected:
	/** Default constructor */
	CommandMgr();
	/** Destructor */
	virtual ~CommandMgr();

	/** Abstract method which has to be defined in client and server, to
	 * create and register the commands */
	virtual void registerCommands() = 0;
	/** Add a command (accesed by the registerCommands method) */
	void addCommand(Command* command);

private:
	/// The commands in this set
	std::map<std::string, Command*> mCommandList;

	/** Find a command by name (returns 0 if not found) */
	Command* findCommand(const std::string& commandName) const;
	/** Parse command line, putting each piece into the list of arguments */
	void parseCommandLine(const char* cmdline,
			      std::vector<std::string>& args);
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
