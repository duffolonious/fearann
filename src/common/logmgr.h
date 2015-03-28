/*
 * logmgr.h
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

#ifndef __FEARANN_COMMON_LOGMGR_H__
#define __FEARANN_COMMON_LOGMGR_H__


/** \file logger
 */

#include "common/patterns/singleton.h"


class LogMgr : public Singleton<LogMgr>
{
public:
	/** The different levels of the log message */
	enum LogMsgType { DEBUG = 1, INFO, WARNING, ERROR, FATAL, LEVEL_COUNT };

	/**
	 * Modify the level of the readable log messages
	 * @param level defines the highest message level you want to get
	 * @return true if modified false elsewhere
	 */
	bool setLogMsgLevel(LogMsgType level);
	/**
	 * Modify the level of the readable log messages
	 * @param level defines the highest message level you want to get
	 * @return true if modified false elsewhere
	 */
	bool setLogMsgLevel(const char* level);

private:
	/** Singleton friend access */
	friend class Singleton<LogMgr>;

	/* Adding the Log functions as friends of this class */
	friend void LogFATAL(const char* msg, ...);
	friend void LogERR(const char* msg, ...);
	friend void LogWRN(const char* msg, ...);
	friend void LogNTC(const char* msg, ...);
	friend void LogDBG(const char* msg, ...);

	/** the log severity level the instance will respect */
	LogMsgType logLevel;


	/**
	 * Default Constructor
	 * @author Arnaud Fleurentdidier Messaoudi (fken)
	 */
	LogMgr();
	/** Destructor */
	~LogMgr();

	/**
	 * Common function to use by the specific methods
	 *
	 * @author Arnaud Fleurentdidier Messaoudi (fken)
	 * @param severity the severity level of the message you want to send
	 * @param msg the message you want to send
	 */
	void Log(LogMsgType severity, const char* msg);

	/** Translate log level to string, to print it or whatever */
	const char* translateToString(LogMsgType level) const;
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
