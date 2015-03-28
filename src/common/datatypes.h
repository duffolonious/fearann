/*
 * datatypes.h
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

#ifndef __FEARANN_COMMON_DATATYPES_H__
#define __FEARANN_COMMON_DATATYPES_H__


#include <string>
#include <stdint.h>


/// EntityID
typedef uint64_t EntityID;


/** Minimal representation of a vector of 3 elements, needed for some messages
 *
 * @author mafm
 */
class Vector3
{
public:
	Vector3() :
		x(0.0f), y(0.0f), z(0.0f) { }
	Vector3(float X, float Y, float Z) :
		x(X), y(Y), z(Z) { }
  
	Vector3 operator+(const Vector3& u) const;
	Vector3 operator-(const Vector3& u) const;
	Vector3 operator-(void) const;
	float operator*(const Vector3& u) const;
	Vector3 operator/(float f) const;
	float distance(const Vector3& u) const;
	void Normalize(void);

	float x;
	float y;
	float z;
};


/** Name-value pair, simple container
 *
 * @author mafm
 */
class NameValuePair
{
public:
	/** Constructor to create an empty structure */
	NameValuePair() { }

	/** Constructor to create an initialized pair */
	NameValuePair(const std::string& n, const std::string& v) :
		name(n), value(v) { }

	/** First element of the pair, name */
	std::string name;
	/** Second element of the pair, value */
	std::string value;
};


/** Representation of an item in the inventory
 */
class InventoryItem
{
public:
	InventoryItem();
	InventoryItem(const std::string& id,
		      const std::string& type,
		      const std::string& subtype,
		      float load);
	InventoryItem(uint32_t id,
		      const std::string& type,
		      const std::string& subtype,
		      float load);

	/// Set item ID
	void setItemID(const std::string& id);
	/// Set item ID
	void setItemID(const char* id);
	/// Set item ID
	void setItemID(uint32_t id);
	/// Get item ID
	const char* getItemID() const;
	/// Set mesh type
	void setType(const std::string& type);
	/// Set mesh type
	void setType(const char* type);
	/// Get mesh type
	const char* getType() const;
	/// Set mesh subtype
	void setSubtype(const std::string& subtype);
	/// Set mesh subtype
	void setSubtype(const char* subtype);
	/// Get mesh subtype
	const char* getSubtype() const;
	/// Set load
	void setLoad(float load);
	/// Get load
	float getLoad() const;

private:
	/// Item identifier
	std::string itemID;
	/// Item mesh type
	std::string meshType;
	/// Item mesh subtype
	std::string meshSubtype;
	/// Load
	float load;
};


/** NPC dialog option
 */
class NPCDialogOption
{
public:
	NPCDialogOption(uint32_t id, const std::string& _text) :
		option(id),text(_text) {}

	/** Get id */
	uint32_t getID() const { return option; };
	/** Get text */
	const std::string& getText() const { return text; };

private:
	uint32_t option;
	std::string text;
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
