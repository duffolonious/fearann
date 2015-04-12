/*
 * tablemgr.cpp
 * Copyright (C) 2006-2008 by Bryan Duff <duff0097@umn.edu>
 *			      Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#include "common/xmlmgr.h"

#include "tablemgr.h"

#include <string>
#include <cstdlib>


//----------------------- Table -------------------------------
Table::Table(const string& name, const string& key, const vector<string>& header) :
	mTableName(name), mKey(key), mHeader(header)
{
}

Table::~Table()
{
	clear();
}

void Table::clear()
{
	mData.clear();
}

string Table::getName() const
{
	return mTableName;
}

string Table::getKey() const
{
	return mKey;
}

uint32_t Table::getNumRows() const
{
	return mData.size();
}

void Table::addRow(const vector<string>& row)
{
	if (row.size() != mHeader.size()) {
		LogWRN("Table: addRow: row size %zu doesn't match the size %zu of the table '%s'",
		       row.size(), mHeader.size(), mTableName.c_str());
	} else {
		mData.push_back(row);
	}
}

const char* Table::getValue(const string& keyVal, const string& col) const
{
	// get the row index
	int r = -1;
	for (size_t i = 0; i < mData.size(); ++i) {
		vector<string> tmp = mData[i];
		for (size_t h = 0; h < tmp.size(); ++h) {
			if (mHeader[h] == mKey && keyVal == tmp[h]) {
				r = i;
				break;
			}
		}
	}

	// get the column index
	int c = -1;
	for (size_t h = 0; h < mHeader.size(); ++h) {
		if (col == mHeader[h]) {
			c = h;
			break;
		}
	}

	if (r == -1
	    || c == -1
	    || static_cast<unsigned int>(r) > mData.size()
	    || static_cast<unsigned int>(c) > mData[r].size()) {
		LogDBG("Requested cell not found in table ('%s'): (%d, %d)",
			mTableName.c_str(), r, c);
		return 0;
	} else {
		return mData[r][c].c_str();
	}
}

int Table::getValueAsInt(const string& keyVal, const string& col) const
{
	return atoi(getValue(keyVal, col));
}

void Table::printTable() const
{
	LogNTC("Print table '%s'", mTableName.c_str() );

	// header
	string tmp;
	for (size_t h = 0; h < mHeader.size(); ++h) {
		tmp += "\t | " + mHeader[h];
	}
	LogNTC("%s\t |", tmp.c_str());

	// data
	for (size_t r = 0; r < mData.size(); ++r) {
		for (size_t c = 0; c < mData[r].size(); ++c) {
			tmp += "\t | " + mData[r][c];
		}
		LogNTC("%s\t |", tmp.c_str());
	}

	LogNTC("end table");
}


//----------------------- TableMgr ----------------------------
template <> TableMgr* Singleton<TableMgr>::INSTANCE = 0;

TableMgr::TableMgr()
{
}

TableMgr::~TableMgr()
{
	for (map<string, Table*>::iterator it = mTables.begin(); it != mTables.end(); ++it) {
		delete (*it).second;
	}
	mTables.clear();
}

const Table* TableMgr::getTable(const char* tableName)
{
	map<string, Table*>::iterator it = mTables.find(string(tableName));
	if (it != mTables.end()) {
		return it->second;
	} else {
		LogWRN("TableMgr: getTable: table '%s' not found", tableName);
		return 0;
	}
}

bool TableMgr::loadFromFile(const char* file)
{
	LogDBG("TableMgr::loadFromFile: Trying to load tables from file '%s'", file);
	const XMLNode* tableFile = XMLMgr::instance().loadXMLFile(file);
	if (!tableFile)
		return false;

	// check whether it contains one or more tables
	if (tableFile->getAttrValueAsStr("key").empty()) {
		// multiple
		for (int i = 0; i < tableFile->getChildListSize(); ++i) {
			const XMLNode* table = tableFile->getChildAt(i);
			if (!table) {
				// bogus #text node or somethink, we just skip
				continue;
			}

			if (!loadTable(table)) {
				delete table;
				delete tableFile;
				return false;
			}

			delete table;
		}
	} else {
		// single
		if (!loadTable(tableFile))
			return false;
	}

	// Cleanup XML manager - only do when done with file.
	XMLMgr::instance().clear();
	delete tableFile;

	return true;
}

bool TableMgr::loadTable(const XMLNode* tableNode)
{
	// get the table name and do some sanity checks
	string tableName = tableNode->getName();
	if (tableName.empty()) {
		LogERR("TableMgr: loadTable: trying to load table, but name empty");
		return false;
	} else if (mTables.find(tableName) != mTables.end()) {
		LogERR("TableMgr: loadTable: table '%s' already loaded", tableName.c_str());
		return false;
	}

	// get key, needed for lookups
	string key = tableNode->getAttrValueAsStr("key");
	if (key.empty()) {
		LogERR("TableMgr: loadTable: No 'key' in table");
		return false;
	}

	// get headers, needed to index by column name; 1 is the first node
	// except from the #text one of XML DOM specification, and the table
	// should have at least that one,
	vector<string> header;
	const XMLNode* firstNode = tableNode->getChildAt(1);
	if (!firstNode) {
		// bogus #text node or somethink, we just skip
		LogERR("TableMgr: loadTable: The table '%s' is empty", tableName.c_str());
		return false;
	} else {
		for (size_t i = 0; i < firstNode->getAttributesLength(); ++i) {
			string column = firstNode->getAttrNameAt(i);
			header.push_back(column);
		}
	}
	delete firstNode;

	// do create
	Table* newTable = new Table(tableName, key, header);
	// LogDBG("Loading table: name '%s', key '%s'", newTable->getName().c_str(), newTable->getKey().c_str());

	// fill the table with data
	for (int i = 0; i < tableNode->getChildListSize(); ++i) {
		const XMLNode* tmp = tableNode->getChildAt(i);
		if (!tmp) {
			// bogus #text node, we just skip
			// LogWRN("TableMgr: loadTable: Child %d for table '%s' is null", i, tableName.c_str());
			continue;
		}

		// add a row
		vector<string> row;
		for (size_t j = 0; j < header.size(); ++j) {
			string attrValue = tmp->getAttrValueAsStr(header[j].c_str());
			// LogDBG("- %zu %s='%s'", j, header[j].c_str(), attrValue.c_str());
			row.push_back(attrValue);
		}
		newTable->addRow(row);

		delete tmp;
	}

	mTables[tableName.c_str()] = newTable;
	return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
