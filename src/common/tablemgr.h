/*
 * tablemgr.h
 * Copyright (C) 2006-2008 by Bryan Duff <duff0097@umn.edu>
 *                            Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#ifndef __FEARANN_COMMON_TABLE_MGR_H__
#define __FEARANN_COMMON_TABLE_MGR_H__


#include <map>
#include <vector>
#include <string>

#include "common/patterns/singleton.h"


class XMLNode;


/** This Table class is a simple implementation of what everybody understands as
 * a table, just adding "key" value which is kind of a primary key in the SQL
 * world: it's the master value, such as 'stregth', to which the rest of columns
 * are related (in example, the carrying capacity of a given creature based on
 * the strength).
 */
class Table
{
public:
	/** Constructor with mandatory parameter for the name of the table. */
	Table(const std::string& name,
	      const std::string& key,
	      const std::vector<std::string>& header);
	/** Destructor */
	~Table();

	/** Clear everything from the table (leave as after constructor) */
	void clear();

	/** Get name of the table */
	std::string getName() const;
	/** Get the key (master column) of the table */
	std::string getKey() const;
	/** Get the number of rows of the table */
	uint32_t getNumRows() const;

	/** Add a row */
	void addRow(const std::vector<std::string>& row);

	/** Get the value of the cell */
	const char* getValue(const std::string& keyVal, const std::string& column) const;
	/** Get the value of the cell */
	int getValueAsInt(const std::string& keyVal, const std::string& column) const;

	/** Print table (for debugging purposes) */
	void printTable() const;

private:
	/// Table name
	std::string mTableName;
	/// Table key
	std::string mKey;
	/// Header
	std::vector<std::string> mHeader;
	/// Data
	std::vector<std::vector<std::string> > mData;
};


/** Manages static table (usually/always from d20 ruleset) for reference.  The
 * data is loaded from XML files.
 */
class TableMgr : public Singleton<TableMgr>
{
public:
	/** Load table(s) from file */
	bool loadFromFile(const char* file);

	/** Get the given table */
	const Table* getTable(const char* tableName);

private:
	/** Singleton friend access */
	friend class Singleton<TableMgr>;

	/// Collection of the tables loaded
	std::map<std::string, Table*> mTables;


	/** Default constructor */
	TableMgr();
	/** Destructor */
	~TableMgr();

	/** Load table with given XML node */
	bool loadTable(const XMLNode* tableNode);
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
