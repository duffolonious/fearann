/*
 * srvdbconnectorpostgresql.h
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

#ifndef __FEARANN_SERVER_DB_CONNECTOR_POSTGRESQL_H__
#define __FEARANN_SERVER_DB_CONNECTOR_POSTGRESQL_H__


#include "srvdbmgr.h"

#ifdef __FreeBSD__
#include <libpq-fe.h>
#else
#include <postgresql/libpq-fe.h>
#endif
#include <string>


/** @ingroup database
 *  @{
 */


/** Implementation of DBResult class for PostgreSQL.
 *
 * @author mafm
 */
class SrvDBPostgresqlResult : public SrvDBResult
{
public:
	/** Default constructor */
	SrvDBPostgresqlResult(PGresult* result);
	/** Destructor */
	virtual ~SrvDBPostgresqlResult();

	/** Get the number of results (for SELECT) */
	virtual size_t getNumberOfRows() const;
	/** Get the number of columns (for SELECT) */
	virtual size_t getNumberOfColumns() const;
	/** Get the number of rows affected (for INSERT, DELETE, UPDATE) */
	virtual size_t getNumberOfAffectedRows() const;
	/** Get the value of the given position in a SELECT result */
	virtual const char* getValue(size_t row, size_t column) const;
	/** Get the column name */
	virtual const char* getColumnName(size_t column) const;

private:
	/// Private, PG result object
	PGresult* mResult;
	/// Number of rows in result (SELECT)
	size_t mNRows;
	/// Number of fields in result (SELECT)
	size_t mNFields;
	/// Number of rows affected in other queries
	size_t mNAffected;
};


/** PostgreSQL database connector.
 *
 * We only redefine the abstract functions of the base class to make it work
 * with PostgreSQL as database.
 *
 * @author mafm
 */
class SrvDBConnectorPostgresql : public SrvDBConnectorBase
{
public:
	/** Overriden from base class */
	virtual bool connectToDB(const char* host,
				 const char* port,
				 const char* dbname,
				 const char* dbuser,
				 const char* dbpass);
	/** Overriden from base class */
	virtual void escapeData(std::string& out,
				const char* data,
				size_t length) const;
	/** Overriden from base class */
	virtual SrvDBResult* executeQuery(const char* cmd) const;

private:
	/** Friend access */
	friend class SrvDBMgr;


	/// Stores a connection object
	PGconn* mConn;


	/** Default constructor */
	SrvDBConnectorPostgresql();
	/** Destructor */
	~SrvDBConnectorPostgresql();
};

/// @}


#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
