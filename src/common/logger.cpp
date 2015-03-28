/*
 * logger.cpp
 * Copyright (C) 2005-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *			      Arnaud Fleurentdidier Messaoudi <fken42@gmail.com>
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

#include <cstdarg>
#include <ctime>
#include <cstdio>
#include <string>

#include "logmgr.h"

#include "logger.h"

using namespace std;

#define LOGSTR_LENGTH 256
// Macro to parse printf-like arguments of a method into fixed string
#define VARARG_PARSE()				\
	va_list arg;				\
	va_start(arg, msg);			\
	char formatBuffer[LOGSTR_LENGTH] = { 0 };			\
	vsnprintf(formatBuffer, sizeof(formatBuffer), msg, arg);	\
	va_end(arg);


void LogFATAL(const char* msg, ...)
{
	VARARG_PARSE();
	LogMgr::instance().Log(LogMgr::FATAL, formatBuffer);
}

void LogERR(const char* msg, ...)
{
	VARARG_PARSE();
	LogMgr::instance().Log(LogMgr::ERROR, formatBuffer);
}

void LogWRN(const char* msg, ... )
{
	VARARG_PARSE();
	LogMgr::instance().Log(LogMgr::WARNING, formatBuffer);
}

void LogNTC(const char* msg, ... )
{
	VARARG_PARSE();
	LogMgr::instance().Log(LogMgr::INFO, formatBuffer);
}

void LogDBG(const char* msg, ... )
{
	VARARG_PARSE();
	LogMgr::instance().Log(LogMgr::DEBUG, formatBuffer);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
