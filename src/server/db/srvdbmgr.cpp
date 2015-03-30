/*
 * srvdbmgr.cpp
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

#include "config.h"

#include "srvdbmgr.h"

#include "common/configmgr.h"

#ifdef HAVE_POSTGRESQL
#include "server/db/srvdbconnectorpostgresql.h"
#endif
#if (!defined HAVE_POSTGRESQL)
#error "You must choose at least one SQL database type"
#endif

#include <cstdlib>
#include <cstring>


/*******************************************************************************
 * SrvDBResult
 ******************************************************************************/
void SrvDBResult::getValue(size_t row, const char* columnName, string& value) const
{
	if (row > getNumberOfRows()) {
		LogERR("Asked for row '%zu', nresults='%zu'",
		       row, getNumberOfRows());
		value = "<row out of bounds>";
		return;
	}

	// Only useful if the column was used and the value initialized
	size_t nColumns = getNumberOfColumns();
	for (size_t column = 0; column < nColumns; ++column) {
		if (string(columnName) == getColumnName(column)) {
			value = getValue(row, column);
			return;
		}
	}
	LogERR("Column '%s' not found, numcolumns=%zu",
	       columnName, nColumns);
	value = "<column not found>";
}

void SrvDBResult::getValue(size_t row, const char* columnName, int& value) const
{
	string str = "<initialized>";
	getValue(row, columnName, str);
	value = atoi(str.c_str());
}

void SrvDBResult::getValue(size_t row, const char* columnName, float& value) const
{
	string str = "<initialized>";
	getValue(row, columnName, str);
	value = atof(str.c_str());
}


/*******************************************************************************
 * SrvDBQuery
 ******************************************************************************/
SrvDBQuery::SrvDBQuery() :
	mResult(0)
{
}

SrvDBQuery::~SrvDBQuery()
{
	if (mResult)
		delete mResult;
}

void SrvDBQuery::setTables(const string& tables)
{
	mTables = tables;
}

void SrvDBQuery::setTables(const char* tables)
{
	mTables = tables;
}

void SrvDBQuery::setCondition(const string& cond)
{
	mCond = cond;
}

void SrvDBQuery::setCondition(const char* cond)
{
	mCond = cond;
}

void SrvDBQuery::setOrder(const string& order)
{
	mOrder = order;
}

void SrvDBQuery::setOrder(const char* order)
{
	mOrder = order;
}

void SrvDBQuery::addColumnWithValue(const char* name, const char* value, bool escape)
{
	string val;
	if (escape) {
		string value_escaped = "<initialized>";
		SrvDBMgr::instance().escape(value_escaped, value);
		// variable values have to be quoted, too
		val = StrFmt("'%s'", value_escaped.c_str());
	} else {
		val = value;
	}

	mFieldPair.push_back(NameValuePair(string(name), val));
}

void SrvDBQuery::addColumnWithValue(const char* name, const string& value, bool escape)
{
	addColumnWithValue(name, value.c_str(), escape);
}

void SrvDBQuery::addColumnWithoutValue(const char* name)
{
	addColumnWithValue(name, "<uninitialized>", false);
}
  
size_t SrvDBQuery::getNumberOfColumns() const
{
	return mFieldPair.size();
}

void SrvDBQuery::getColumnName(size_t column, string& name) const
{
	// Only retrieve it if the column was used
	if (column < mFieldPair.size()) {
		name = mFieldPair[column].name;
	} else {
		LogERR("Asked for column %zu, numcolumns=%zu",
		       column, mFieldPair.size());
		name = "<column out of range>";
	}
}

void SrvDBQuery::getColumnValue(const char* columnName, string& value) const
{
	// Only useful if the column was used and the value initialized
	for (size_t column = 0; column < mFieldPair.size(); ++column) {
		if (mFieldPair[column].name == columnName) {
			value = mFieldPair[column].value;
			return;
		}
	}
	LogERR("Column '%s' not found, numcolumns=%zu",
	       columnName, mFieldPair.size());
	value = "<column not found>";
}

void SrvDBQuery::getColumnValue(size_t column, string& value) const
{
	// Only retrieve it if the column was used
	if (column < mFieldPair.size()) {
		value = mFieldPair[column].value;
	} else {
		LogERR("Asked for column %zu, numcolumns=%zu",
		       column, mFieldPair.size());
		value = "<column out of range>";
	}
}

const SrvDBResult* SrvDBQuery::getResult() const
{
	return mResult;
}

void SrvDBQuery::setResult(SrvDBResult* result)
{
	mResult = result;
}


/*******************************************************************************
 * SrvDBMgr
 ******************************************************************************/
template <> SrvDBMgr* Singleton<SrvDBMgr>::INSTANCE = 0;

SrvDBMgr::SrvDBMgr() :
	mConnector(0)
{
	string dbtype = ConfigMgr::instance().getConfigVar("Server.Database.Type", "");
	string host = ConfigMgr::instance().getConfigVar("Server.Database.Host", "");
	string port = ConfigMgr::instance().getConfigVar("Server.Database.Port", "");
	string dbname = ConfigMgr::instance().getConfigVar("Server.Database.DatabaseName", "");
	string dbuser = ConfigMgr::instance().getConfigVar("Server.Database.Username", "");
	string dbpass = ConfigMgr::instance().getConfigVar("Server.Database.Password", "");
	if (dbtype.empty() || host.empty() || port.empty()
	    || dbname.empty() || dbuser.empty() || dbpass.empty()) {
		LogERR("Couldn't read necessary config values for the DB");
	}

#ifdef HAVE_POSTGRESQL
	if (dbtype == "postgresql")
		mConnector = new SrvDBConnectorPostgresql();
#endif

	// do connect
	mConnector->connectToDB(host.c_str(), port.c_str(),
				dbname.c_str(),
				dbuser.c_str(), dbpass.c_str());
}

SrvDBMgr::~SrvDBMgr()
{
}

void SrvDBMgr::finalize()
{
	delete mConnector;
}

void SrvDBMgr::escape(std::string& out, const std::string& in) const
{
	mConnector->escapeData(out, in.c_str(), in.size());
}

string SrvDBMgr::escape(const std::string& in) const
{
	string out;
	mConnector->escapeData(out, in.c_str(), in.size());
	return out;
}

bool SrvDBMgr::execute(const char* cmd) const
{
	SrvDBResult* res = mConnector->executeQuery(cmd);
	if (res) {
		delete res;
		return true;
	} else {
		return false;
	}
}

bool SrvDBMgr::queryInsert(const SrvDBQuery* query) const
{
	// auxiliar
	string colname = "<initialized>";
	string value = "<initialized>";

	// start to build the query
	size_t numcolumns = query->getNumberOfColumns();
	string qry = "INSERT INTO " + query->mTables + " (";
	for (size_t column = 0; column < numcolumns; ++column) {
		query->getColumnName(column, colname);
		if (0 != column) qry += ",";
		qry += colname;
	}
	qry += ") VALUES (";
	for (size_t column = 0; column < numcolumns; ++column) {
		query->getColumnValue(column, value);
		if (0 != column) 
			qry += ",";
		qry += value;
	}
	qry += ")";

	// final processing
	SrvDBResult* res = mConnector->executeQuery(qry.c_str());
	if (!res) {
		return false;
	} else {
		bool result = (res->getNumberOfAffectedRows() > 0);
		delete res;
		return result;
	}
}

int SrvDBMgr::queryUpdate(const SrvDBQuery* query) const
{
	// auxiliar
	string colname = "<initialized>";
	string value = "<initialized>";

	// start to build the query
	string qry = "UPDATE " + query->mTables + " SET ";
	for (size_t column = 0; column < query->getNumberOfColumns(); ++column) {
		query->getColumnName(column, colname);
		query->getColumnValue(column, value);
		if (0 != column) qry += ",";
		qry += colname + "=" + value;
	}

	// condition?
	if (query->mCond.size() > 0)
		qry += " WHERE " + query->mCond;

	// final processing
	SrvDBResult* res = mConnector->executeQuery(qry.c_str());
	if (!res) {
		return -1;
	} else {
		int affected = res->getNumberOfAffectedRows();
		delete res;
		return affected;
	}
}

int SrvDBMgr::queryDelete(const SrvDBQuery* query) const
{
	// starting to build the query
	string qry = "DELETE FROM " + query->mTables;

	// condition ?
	if (query->mCond.size() > 0)
		qry += " WHERE " + query->mCond;

	// final processing
	SrvDBResult* res = mConnector->executeQuery(qry.c_str());
	if (!res) {
		return -1;
	} else {
		int affected = res->getNumberOfAffectedRows();
		delete res;
		return affected;
	}
}

int SrvDBMgr::querySelect(SrvDBQuery* query) const
{
	// auxiliar
	string colname = "<initialized>";

	// starting to build the query
	string qry = "SELECT ";
	for (size_t column = 0; column < query->getNumberOfColumns(); ++column) {
		query->getColumnName(column, colname);
		if (0 != column) qry += ",";
		qry += colname;
	}
	qry += " FROM " + query->mTables;

	// condition?
	if (query->mCond.size() > 0)
		qry += " WHERE " + query->mCond;

	// order?
	if (query->mOrder.size() > 0)
		qry += " ORDER BY " + query->mOrder;

	// final processing
	SrvDBResult* result = mConnector->executeQuery(qry.c_str());
	if (!result) {
		return -1;
	} else {
		query->setResult(result);
		return result->getNumberOfRows();
	}
}

bool SrvDBMgr::queryMatch(const SrvDBQuery* query) const
{
	int numResults = queryMatchNumber(query);
	if (numResults <= 0) {
		return false;
	} else {
		return true;
	}
}

int SrvDBMgr::queryMatchNumber(const SrvDBQuery* query) const
{
	// base
	string qry = "SELECT count(*) FROM " + query->mTables;

	// add condition
	if (query->mCond.size() > 0)
		qry += " WHERE " + query->mCond;

	// execute the query itself
	SrvDBResult* result = mConnector->executeQuery(qry.c_str());
	if (!result) {
		return -1;
	} else {
		int count = 0;
		result->getValue(0, "count", count);
		delete result;
		return count;
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
