/*
 * configmgr.h
 * Copyright (C) 2006-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *			      Bryan Duff <duff0097@umn.edu>
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

#ifndef __FEARANN_COMMON_CONFIG_MGR_H__
#define __FEARANN_COMMON_CONFIG_MGR_H__

#include <map>
#include <string>

#include "common/patterns/singleton.h"


/** This class reads and writes configuration files, considering only the lines
 * containing "=" symbols, and tries to parse the key and the value, to the left
 * and the right sides of the equal symbol, removing blanks surrounding the
 * characters.  It stores the parsed key-value pairs so they can be retrieved
 * later, asking for the variable name (the "key" component).
 *
 * This class makes several assumptions, which will make the program to work
 * unreliably or abort if not met:
 *
 * - There is only one config file, so there are not ambiguities when it comes
 * to decide where to save variables.
 * 
 * - Each key can be present only once, so we don't get conflicting values, and
 * when we overwrite the value we have only one place to do it.
 *
 * - Variables are never removed after being added, unless special methods to
 * reset the manager are called (all them are cleared).
 *
 * - To store a variable means to save it inmmediately to the file, and in the
 * runtime structure.  The variable must be present, that is, it only admits to
 * overwrite variables, not to append them.  So when storing a variable, it will
 * override the same value in the same line of the file -- no need to change the
 * layout of the file or touch the commentaries or blank lines, we just
 * substitute a line.
 *
 *
 * These impositions may seem too restrictive, but after the experience of using
 * a similar class from CrystalSpace engine with lots of fancy functionalities,
 * it became clear that the added flexibility doesn't come to help, but to cause
 * subtle bugs: shadowing variables with new values defined in different places,
 * saving variables in different files (which leads to the previous problem),
 * etc.  After all, having a few to dozen different variables should be enough,
 * and this hardly justifies to have complex schemes that bring headaches way
 * too often.
 */
class ConfigMgr : public Singleton<ConfigMgr>
{
public:
	/** Load the given file, incorporating the key-value pairs found.  Only
	 * one file can be added, read the rationale behind this in the comments
	 * to the class. */
	bool loadConfigFile(const char* file);
	/** Clear (reset) the state of the manager, useful if we need to restart
	 * the aplication or similar situations. */
	void clear();
	/** Get the variable with the given key, returning default value if not
	 * found */
	const char* getConfigVar(const char* varName, const char* defaultValue);
	/** Store the given variable in the file and in the runtime structure,
	 * the variable must be there.  Read the comments to the class to
	 * understand the rationale behind this.  Returns whether the request
	 * could be performed or not. */
	bool storeConfigVar(const char* varName, const char* varValue);

private:
	/** Singleton friend access */
	friend class Singleton<ConfigMgr>;

	/// Name of the config file (used when storing vars)
	std::string mConfigFilename;
	/// Variables held in config file(s)
	std::map<std::string, std::string> mConfig;

	/** Default constructor */
	ConfigMgr();
	/** Destructor */
	~ConfigMgr();

	/** Parse a line, returning true when it could get a <key,value> pair,
	 * false otherwise.  When returning true, the varName and varValue
	 * parameters are filled with the values parsed: varName is the left
	 * part of the first '=' (stripping surrounding blanks), varValue is the
	 * right side (stripping surrounding blanks, too). */
	bool parseLine(const std::string& line, std::string& varName, std::string& varValue);

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
