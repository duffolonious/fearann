/*
 * logger.h
 * Copyright (C) 2005-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#ifndef __FEARANN_COMMON_LOGGER_H__
#define __FEARANN_COMMON_LOGGER_H__

/** \file logger
 *
 * This file has all needed elements to access the log.  It would be more
 * elegant with a singleton class; but this way saves a lot of typing (it's used
 * everywhere, obviously).
 */

/** Log a fatal error */
void LogFATAL(const char* msg, ...) __attribute__((format(printf, 1, 2)));

/** Log an error */
void LogERR(const char* msg, ...) __attribute__((format(printf, 1, 2)));

/** Log a warning */
void LogWRN(const char* msg, ...) __attribute__((format(printf, 1, 2)));

/** Log a notice */
void LogNTC(const char* msg, ...) __attribute__((format(printf, 1, 2)));

/** Log a debug message */
void LogDBG(const char* msg, ...) __attribute__((format(printf, 1, 2)));

#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
