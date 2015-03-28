/*
 * srvdbmgr.h
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

#ifndef __FEARANN_SERVER_DB_MGR_H__
#define __FEARANN_SERVER_DB_MGR_H__


#include "common/patterns/singleton.h"
#include "common/datatypes.h"

#include <vector>


/** @defgroup database Database Group
 *
 * This group contains all database related classes.
 *
 * @{
 */


/** Data returned from a database query, abstract so it can be performed by
 * several DB backends.
 *
 * @author mafm
 */
class SrvDBResult
{
public:
	/** Destructor. */
	virtual ~SrvDBResult() { }

	/** Get the number of results (for SELECT) */
	virtual size_t getNumberOfRows() const = 0;
	/** Get the number of columns (for SELECT) */
	virtual size_t getNumberOfColumns() const = 0;
	/** Get the number of rows affected (for INSERT, DELETE, UPDATE) */
	virtual size_t getNumberOfAffectedRows() const = 0;
	/** Get the value of the given position in a SELECT result */
	virtual const char* getValue(size_t row, size_t column) const = 0;
	/** Get the column name */
	virtual const char* getColumnName(size_t column) const = 0;

	/** Get the value of the row.columnName as string */
	void getValue(size_t row, const char* columnName, std::string& value) const;
	/** Get the value of the row.columnName, as an int */
	void getValue(size_t row, const char* columnName, int& value) const;
	/** Get the value of the row.columnName, as a float */
	void getValue(size_t row, const char* columnName, float& value) const;
};


/** Query structure, to use with high level queries.
 *
 * It's for all functions where we need to pass or retrieve data from/to the DB,
 * so they are put in a tuple similar to the one in the DB and we can retrieve
 * the results by column name and so on.
 *
 * @author mafm
 */
class SrvDBQuery
{
	friend class SrvDBMgr;

public:
	/** Default constructor. */
	SrvDBQuery();
	/** Destructor. */
	~SrvDBQuery();

	/** Set the table for the query */
	void setTables(const char* tables);
	/** Set the table for the query */
	void setTables(const std::string& tables);
	/** Set the condition for the query */
	void setCondition(const char* cond);
	/** Set the condition for the query */
	void setCondition(const std::string& cond);
	/** Set the order for the SELECT result */
	void setOrder(const char* order);
	/** Set the order for the SELECT result */
	void setOrder(const std::string& order);
	/** Add a element for INSERT+UPDATE operations: column and value to
	 * use */
	void addColumnWithValue(const char* name, const char* value, bool escape=true);
	/** Add a element for INSERT+UPDATE operations: column and value to
	 * use */
	void addColumnWithValue(const char* name, const std::string& value, bool escape=true);
	/** Add a element for SELECT operation: column to retrieve */
	void addColumnWithoutValue(const char* name);
	/** Get the number of columns used */
	size_t getNumberOfColumns() const;
	/** Get the name of the column */
	void getColumnName(size_t index, std::string& name) const;
	/** Get the value of the column */
	void getColumnValue(const char* name, std::string& value) const;
	/** Get the value of the column */
	void getColumnValue(size_t index, std::string& value) const;

	/** Get the result
	 *
	 * \returns 0 if no result is available, only SELECT queries have
	 * result.
	 */
	const SrvDBResult* getResult() const;
	/** Set the result */
	void setResult(SrvDBResult* result);

private:
	/// The tables for the query
	std::string mTables;
	/// The condition for the query
	std::string mCond;
	/// The order for the query
	std::string mOrder;
	/// The structure of columns/fields for DB queries
	std::vector<NameValuePair> mFieldPair;
	/// The result (for SELECT queries only)
	SrvDBResult* mResult;
};


/** Database connector, different for each DB backend
 */
class SrvDBConnectorBase
{
public:
	/** Connect to the DB */
	virtual bool connectToDB(const char* hostname,
				 const char* port,
				 const char* dbname,
				 const char* dbuser,
				 const char* dbpass) = 0;
	/** Make strings and binary data safe to insert in a query */
	virtual void escapeData(std::string& out,
				const char* data,
				size_t length) const = 0;
	/** Execute a query and return output (returns 0 if failure).
	 *
	 * \note The caller is responsible for delete'ing object when
	 * finished. */
	virtual SrvDBResult* executeQuery(const char* sqlcmd) const = 0;

protected:
	/** Friend access */
	friend class SrvDBMgr;


	/** Default constructor */
	SrvDBConnectorBase() { }
	/** Destructor */
	virtual ~SrvDBConnectorBase() { }

};


/** Database manager, abstract so it can be performed by several DB backends.
 * It contains some comfortable functions to perform the operations which should
 * be used whenever possible.
 */
class SrvDBMgr : public Singleton<SrvDBMgr>
{
public:
	/** Finalize, do whatever cleanup needed when the server shuts down. */
	void finalize();

	/** Helper for escapeData */
	void escape(std::string& out, const std::string& in) const;
	/** Helper for escapeData */
	std::string escape(const std::string& in) const;

	/** Fancy command to insert new data, returns if it was succesful */
	bool queryInsert(const SrvDBQuery* query) const;
	/** Fancy command to update, returns affected rows */
	int queryUpdate(const SrvDBQuery* query) const;
	/** Fancy command to delete data, returns affected rows */
	int queryDelete(const SrvDBQuery* query) const;
	/** Fancy command to get data from the DB, returns number of results */
	int querySelect(SrvDBQuery* query) const;
	/** Simple match function, to know if there are any row in a table which
	 * matches the asked conditions */
	bool queryMatch(const SrvDBQuery* query) const;
	/** Match function, to know the number of rows in a table which match
	 * the asked conditions */
	int queryMatchNumber(const SrvDBQuery* query) const;

private:
	/** Singleton friend access */
	friend class Singleton<SrvDBMgr>;

	/** Default constructor */
	SrvDBConnectorBase* mConnector;


	/** Default constructor */
	SrvDBMgr();
	/** Destructor. */
	~SrvDBMgr();

	/** Execute a query where we don't care about the output */
	bool execute(const char* sqlcmd) const;
};

#endif


/// @}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
