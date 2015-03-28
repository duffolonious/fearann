/*
 * configmgr.cpp
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

#include "config.h"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#include "configmgr.h"


//----------------------- ConfigMgr ----------------------------
template <> ConfigMgr* Singleton<ConfigMgr>::INSTANCE = 0;

ConfigMgr::ConfigMgr()
{
}

ConfigMgr::~ConfigMgr()
{
	clear();
}

bool ConfigMgr::loadConfigFile(const char* file)
{
	// check whether we already have the config loaded
	if (!mConfig.empty()) {
		LogERR("Config already loaded, refusing to load it again");
		return false;
	}

	mConfigFilename = file;
	ifstream cfgfile(file);
	if (cfgfile.is_open()) {
		string line;
		while (! cfgfile.eof()) {
			getline(cfgfile, line);
			string varName, varValue;
			bool gotParsed = parseLine(line, varName, varValue);
			if (!gotParsed) {
				continue;
			} else {
				// insert into the storage
				if (mConfig.find(varName) == mConfig.end()) {
					mConfig[varName] = varValue;
				} else {
					LogERR("Config: key '%s' already present (%s), fix it -- aborting",
					       varName.c_str(), (*mConfig.find(varName)).second.c_str());
					exit(EXIT_FAILURE);
				}
			}
		}
		cfgfile.close();
		LogNTC("Loaded config file: %s", file);
		return true;
	} else {
		LogERR("Couldn't load config: %s", strerror(errno));
		return false;
	}
}

void ConfigMgr::clear()
{
	mConfig.clear();
}

const char* ConfigMgr::getConfigVar(const char* varName, const char* defaultValue)
{
	string var(varName);
	if (mConfig.find(var) != mConfig.end()) {
		return mConfig[var].c_str();
	} else {
		return defaultValue;
	}
}

bool ConfigMgr::storeConfigVar(const char* varName, const char* varValue)
{
	// check whether the variable is present
	string result = getConfigVar(varName, "<does not exist>");
	if (result == "<does not exist>") {
		LogERR("Couldn't found var '%s' in file (refusing to add new vars, can only overwrite)",
		       varName);
		return false;
	} else if (result == varValue) {
		/* silently avoiding to overwrite
		LogDBG("Var '%s' already has the intended value '%s' (avoiding unnecessary overwrite)",
		       varName, varValue);
		*/
		return true;
	}

	// retrieving the source file, line by line
	vector<string> sourceLines;
	ifstream cfgfile(mConfigFilename.c_str(), ios::in);
	if (cfgfile.is_open()) {
		string line;
		while (! cfgfile.eof()) {
			getline(cfgfile, line);
			sourceLines.push_back(line);
		}
		cfgfile.close();
	} else {
		LogERR("Couldn't open config to store a var: %s", strerror(errno));
		return false;
	}

	// substituting the line
	for (vector<string>::iterator it = sourceLines.begin(); it != sourceLines.end(); ++it) {

		// trim, so we also act when there is blank space at the start
		StrTrim(*it);

		// replacing it only when the var name is at position zero in
		// the line (i.e., it's not a comment)
		string::size_type pos = it->find(string(varName));
		if (pos == 0) {
			LogDBG("Found '%s' in line '%s', substituting variable name", varName, it->c_str());
			(*it) = varName + string(" = ") + varValue;

			// writing the updated set of lines back to the file
			ofstream cfgfileWrite(mConfigFilename.c_str(), ios::trunc | ios::out);
			if (cfgfileWrite.is_open()) {
				for (vector<string>::iterator it2 = sourceLines.begin(); it2 != sourceLines.end(); ++it2) {
					// trimming always, so we get "cleaner"
					// config files
					StrTrim(*it2);
					cfgfileWrite << *it2 << std::endl;
				}
				cfgfileWrite.close();
				return true;
			} else {
				LogERR("Couldn't open config to store a var: %s", strerror(errno));
				return false;
			}
		}
	}

	// error when we're here
	LogERR("Variable was parsed but it doesn't seem to be present in the file -- nothing done");
	return false;
}

bool ConfigMgr::parseLine(const string& line, string& varName, string& varValue)
{
	string::size_type eqPosition = line.find('=');
	if (line[0] == '#' || eqPosition == line.npos) {
		// line doesn't contain '=' or is a comment (starts with #),
		// ignore
		return false;
	} else {
		// parse key and value parts and insert into our config storage
		// area
		varName = line.substr(0, eqPosition);
		StrTrim(varName);
		varValue = line.substr(eqPosition+1, line.length());
		StrTrim(varValue);
		// LogDBG("CFG PARSED: key='%s', value='%s'", varName.c_str(), varValue.c_str());
		return true;
	}
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
