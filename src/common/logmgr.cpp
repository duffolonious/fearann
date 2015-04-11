/*
 * logmgr.cpp
 * Copyright (C) 2008 by Arnaud Fleurentdidier Messaoudi <fken42@gmail.com>
 *                       Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>  
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

#include <cstdarg>
#include <ctime>
#include <cstdio>
#include <string>

#include "logmgr.h"

const size_t TIMESTAMP_LENGTH = sizeof("YYYYmmdd HH:MM:SS");
const size_t LOGSTR_LENGTH = 256;


template <> LogMgr* Singleton<LogMgr>::INSTANCE = 0;

LogMgr::LogMgr() :
	logLevel(DEBUG)
{
}

LogMgr::~LogMgr()
{
}

void LogMgr::Log(LogMsgType type, const char* msg)
{
	if (type >= logLevel) {
		/* first of all, it defines the message severity token you'll
		 * see at the left side of the log message. It depends on the
		 * LogMsgType level of course */
		const char* severity = translateToString(type);

		/* define when the message has been sent */
		char ts[TIMESTAMP_LENGTH];
		time_t now = time(0);
		strftime(ts, sizeof(ts), "%Y%m%d %H:%M:%S", localtime(&now));

		/* create the message <time>::<severity>::<message>*/
		char fullMsg[LOGSTR_LENGTH] = { 0 };
		snprintf(fullMsg, sizeof(fullMsg), "%s :: %s :: %s\n", ts, severity, msg);

		/* print the message in the standard error stream */
		fprintf(stderr, "%s", fullMsg);
	}
}

bool LogMgr::setLogMsgLevel(LogMsgType level)
{
	/* when you want to modify what kind of message you're ready to see,
	 * you'll precise the level with a constant string. This method modifies
	 * the severity level thanks to the user's token */
	logLevel = level;
	return true;
}

bool LogMgr::setLogMsgLevel(const char* level)
{
	string levelStr(level);
	for (int l = DEBUG; l < LEVEL_COUNT; ++l) {
		if (levelStr == translateToString(static_cast<LogMsgType>(l))) {
			logLevel = static_cast<LogMsgType>(l);
			return true;
		}
	}
	return false;
}

const char* LogMgr::translateToString(LogMsgType level) const
{
	switch (level) {
	case DEBUG:
		return "DEBUG";
	case INFO:
		return "INFO";
	case WARNING:
		return "WARNING";
	case ERROR:
		return "ERROR";
	case FATAL:
		return "FATAL";
	default:
		// This case should not appear. If it appears, then it means
		// there is bug in the message logger
		return "INVALID LOG LEVEL";
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
