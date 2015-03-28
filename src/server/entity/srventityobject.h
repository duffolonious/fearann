/*
 * srventityobject.h
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

#ifndef __FEARANN_SERVER_ENTITY_OBJECT_H__
#define __FEARANN_SERVER_ENTITY_OBJECT_H__


#include "srventitybase.h"


/** Representation of an object in the server
 *
 * @author mafm
 */
class SrvEntityObject : public SrvEntityBase
{
public:
	/** Default constructor
	 *
	 * @param basic Basic data to pass to the base class
	 *
	 * @param mov Movement data to pass to the base class
	 */
	SrvEntityObject(const MsgEntityCreate& basic,
			const MsgEntityMove& mov);
	/** Destructor */
	virtual ~SrvEntityObject();

	/** Set the load value */
	void setLoad(float load);
	/** Get the load value */
	float getLoad() const;

protected:
	/// Load value, basically the weight for dense objects and some
	/// estimated value for the other.  This is determined in files defining
	/// objects
	float mLoad;
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
