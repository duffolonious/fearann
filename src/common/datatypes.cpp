/*
 * datatypes.cpp
 * Copyright (C) 2006 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#include "datatypes.h"

#include <math.h>

/** Convert from degrees (0..360) to radians (0..2*PI).
 */
float DegreesToRadians(float deg)
{
	return deg * PI_NUMBER / 180.0f;
}

/** Convert from radians to degrees.
 */
float RadiansToDegrees(float rad)
{
	return rad * 180.0f / PI_NUMBER;
}



//-------------------------- Vector3 ----------------------------------
Vector3 Vector3::operator+(const Vector3& u) const
{
	return Vector3(x + u.x, y + u.y, z + u.z); 
}

Vector3 Vector3::operator-(const Vector3& u) const
{
	return Vector3(x - u.x, y - u.y, z - u.z); 
}

Vector3 Vector3::operator-(void) const
{
	return Vector3(-x, -y, -z);
}

float Vector3::operator*(const Vector3& u) const
{
	return (x * u.x + y * u.y + z * u.z);
}

Vector3 Vector3::operator/(float f) const
{
	return 	Vector3(x/f, y/f, z/f);
}

float Vector3::distance(const Vector3& u) const
{
	return sqrt(power2(x - u.x) + power2(y - u.y) + power2(z - u.z));
}

void Vector3::Normalize(void)
{
	float const tol = 0.0000000001f;
	float mag = sqrtf(x * x + y * y + z * z);
	if(mag <= tol)
		mag = 1;

	x /= mag;
	y /= mag;
	z /= mag;

	if(fabs(x) < tol)
		x = 0.0f;
	if(fabs(y) < tol)
		y = 0.0f;
	if(fabs(z) < tol)
		z = 0.0f;
}


//-------------------------- InventoryItem ----------------------------
InventoryItem::InventoryItem() :
	load(-1.0f)
{
}

InventoryItem::InventoryItem(const string& id, const string& type, const string& subtype, float l) :
	itemID(id), meshType(type), meshSubtype(subtype), load(l)
{
}

InventoryItem::InventoryItem(uint32_t id, const string& type, const string& subtype, float l) :
	meshType(type), meshSubtype(subtype), load(l)
{
	setItemID(id);
}

const char* InventoryItem::getItemID() const
{
	return itemID.c_str();
}

void InventoryItem::setItemID(const string& id)
{
	setItemID(id.c_str());
}

void InventoryItem::setItemID(const char* id)
{
	itemID = id;
}

void InventoryItem::setItemID(uint32_t id)
{
	itemID = StrFmt("%u", id);
}

const char* InventoryItem::getType() const
{
	return meshType.c_str();
}

void InventoryItem::setType(const string& type)
{
	setType(type.c_str());
}

void InventoryItem::setType(const char* type)
{
	meshType = type;
}

const char* InventoryItem::getSubtype() const
{
	return meshSubtype.c_str();
}

void InventoryItem::setSubtype(const string& subtype)
{
	setSubtype(subtype.c_str());
}

void InventoryItem::setSubtype(const char* subtype)
{
	meshSubtype = subtype;
}

float InventoryItem::getLoad() const
{
	return load;
}

void InventoryItem::setLoad(float _load)
{
	load = _load;
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
