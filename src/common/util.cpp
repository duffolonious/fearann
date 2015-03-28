/*
 * util.cpp
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

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

#include "logger.h"

#include "util.h"

using namespace std;


const int STRFMT_LENGTH = 256;


//--------------------------- StrFmt -------------------------
const char* StrFmt(const char* fmt, ...)
{
	static char strfmtBuffer[STRFMT_LENGTH];
	va_list arg;
	va_start(arg, fmt);
	int charsWritten = vsnprintf(strfmtBuffer, sizeof(strfmtBuffer), fmt, arg);
	if (static_cast<size_t>(charsWritten) >= sizeof(strfmtBuffer)) {
		LogWRN("String truncated: '%s', would write %d instead of the limit %d",
		       strfmtBuffer, charsWritten, sizeof(strfmtBuffer));
	}
	va_end(arg);
	return strfmtBuffer;
}


//--------------------------- StrTrim -------------------------
void StrTrim(string& source)
{
	string::size_type index = source.find_last_not_of(" \t\n\r");
	if (index != string::npos) {
		source = source.substr(0, index+1);
	} else {
		// we only found blanks, we can safely quit
		source.clear();
		return;
	}

	index = source.find_first_not_of(" \t\n\r");
	if (index != string::npos) {
		source = source.substr(index, string::npos);
	}
}


//--------------------------- StrToUInt64 -------------------------
uint64_t StrToUInt64(const std::string& str)
{
	return strtoull(str.c_str(), NULL, 10);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
