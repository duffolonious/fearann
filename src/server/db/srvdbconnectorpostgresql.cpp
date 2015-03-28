/*
 * srvdbconnectorpostgresql.cpp
 * Copyright (C) 2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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
 
#ifdef HAVE_POSTGRESQL

#include "config.h"

#include "srvdbconnectorpostgresql.h"


/*******************************************************************************
 * SrvDBPostgresqlResult
 ******************************************************************************/
SrvDBPostgresqlResult::SrvDBPostgresqlResult(PGresult* result)
{
	mResult = result;
	mNRows = PQntuples(mResult);
	mNFields = PQnfields(mResult);
	mNAffected = atoi(PQcmdTuples(mResult));
}

SrvDBPostgresqlResult::~SrvDBPostgresqlResult()
{
	PQclear(mResult);
}

const char* SrvDBPostgresqlResult::getValue(size_t row, size_t column) const
{
	if (row > mNRows || column > mNFields) {
		LogERR("DB PostgreSQL: getValue out of range (row '%zu', column '%zu')",
		       row, column);
		return 0;
	} else {
		return PQgetvalue(mResult, row, column);
	}
}

const char* SrvDBPostgresqlResult::getColumnName(size_t column) const
{
	if (column > mNFields) {
		LogERR("DB PostgreSQL: getColumnName out of range (column '%zu')",
		       column);
		return 0;
	} else {
		return PQfname(mResult, column);
	}
}

size_t SrvDBPostgresqlResult::getNumberOfRows() const
{
	return mNRows;
}

size_t SrvDBPostgresqlResult::getNumberOfColumns() const
{
	return mNFields;
}

size_t SrvDBPostgresqlResult::getNumberOfAffectedRows() const
{
	return mNAffected;
}


/*******************************************************************************
 * SrvDBConnectorPostgresql
 ******************************************************************************/
SrvDBConnectorPostgresql::SrvDBConnectorPostgresql() :
	mConn(0)
{
}

SrvDBConnectorPostgresql::~SrvDBConnectorPostgresql()
{
	PQfinish(mConn);
}

bool SrvDBConnectorPostgresql::connectToDB(const char* host,
					   const char* port,
					   const char* dbname,
					   const char* dbuser,
					   const char* dbpass)
{
	string connstring = StrFmt("host=%s port=%s dbname=%s user=%s password=%s connect_timeout=1",
				   host, port, dbname, dbuser, dbpass);

	LogNTC("Connecting to PostgreSQL");
	mConn = PQconnectdb(connstring.c_str());
	if (PQstatus(mConn) == CONNECTION_BAD) {
		LogERR("DB failed: '%s'", PQerrorMessage(mConn));
		return false;
	} else {
		return true;
	}
}

SrvDBResult* SrvDBConnectorPostgresql::executeQuery(const char* sqlcmd) const
{
	PGresult* res = PQexec(mConn, sqlcmd);

	// check to see that the backend connection was successfully made
	if (! (PQresultStatus(res) == PGRES_COMMAND_OK
	       || PQresultStatus(res) == PGRES_TUPLES_OK)) {
		LogERR("DB failed: '%s'", PQresultErrorMessage(res));
		return 0;
	} else {
		return new SrvDBPostgresqlResult(res);
	}
}

void SrvDBConnectorPostgresql::escapeData(string& out, const char* data, size_t length) const
{
	char escData[(length*2) + 1];
	escData[0] = '\0';
	PQescapeString(escData, data, length);
	out = escData;
}


#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
