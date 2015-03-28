/*
 * bot.h
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

#ifndef __FEARANN_BOT_MAIN_H__
#define __FEARANN_BOT_MAIN_H__

#include <string>
#include <map>

class fmBot;
class BotNetworkMgr;
class BotCommandMgr;

extern fmBot* Bot;

class fmBot
{
public:
	enum MODE { NPC=1, DEBUG };

	fmBot();

	/** Init all startup info.
	 */
	bool OnInitialize(int argc, char** argv);

	///on exit cleanup...
	void OnExit();

	/// Setup
	bool Application();

	/// Loop
	void Run();

	/// Called with command /quit
	void Quit();

	/// Execute bot command
	void executeCommand(const char* cmd);

	/// Get protocol version (to know what clients can connect)
	//const char* GetProtocolVersion() const;

	BotCommandMgr* getCommandMgr();
	BotNetworkMgr* getNetworkMgr();

	/// Set ability to automagically login/join at startup
	bool getAutoConnect() { return autoConnect; };
	void setAutoConnect(bool _connect) { autoConnect = _connect; };
	bool getAutoLogin() { return autoLogin; };
	void setAutoLogin(bool _login) { autoLogin = _login; };
	bool getAutoJoin() { return autoJoin; };
	void setAutoJoin(bool _join) { autoJoin = _join; };

	const char * getName() { return mName.c_str(); };
	void setName( const char * name ) { mName = name; };

	bool isMode( MODE mode )
	{
		if ( mode == mMode )
			return true;
		return false;
	};

	void setMode( MODE mode ) { mMode = mode; };

private:

	/// The message handler
	BotNetworkMgr* mBotNetworkMgr;

	/// Command manager
	BotCommandMgr* mBotCommandMgr;

	bool mInteractiveMode;

	/// Load startup command script
	bool loadStartupScript(const char* file);

	/// Auto vars loaded in config file.
	bool autoConnect;
	bool autoLogin;
	bool autoJoin;

	std::string mName;

	MODE mMode;

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
