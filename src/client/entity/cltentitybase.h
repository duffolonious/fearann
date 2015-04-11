/*
 * cltentitybase.h
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

#ifndef __FEARANN_CLIENT_ENTITY_BASE_H__
#define __FEARANN_CLIENT_ENTITY_BASE_H__


#include "common/net/msgs.h"

class BoundingBox;

namespace osg {
	class Node;
	class MatrixTransform;
}

#include <osg/BoundingBox>

/** Base class for entities in the client, which holds the basic data (class,
 * mesh type and so on).
 *
 * The objects created from this class are entities from the game logic point of
 * view, thus they must react to game events when needed, adhere to ruleset and
 * so on.  Basically, all the data depending on the game itself must be kept in
 * the derived classes, for clarity.
 */
class CltEntityBase
{
public:
	/** Default constructor */
	CltEntityBase(const MsgEntityCreate* entityBasicData, const char* className);
	/** Destructor */
	virtual ~CltEntityBase();

	/** Class Name, to identify different entities */
	const char* className() const;

	/** Function to get called every frame, to update the position of the
	 * entity in the world. */
	virtual void updateTransform(double elapsedSeconds) = 0;

	/** Entity name. */
	const char* getName() const;
	/** Entity name. */
	uint64_t getID() const;
	/** Entity transform. */
	osg::MatrixTransform* getTransform() const;
	/** Entity bounding box. */
	const osg::BoundingBox* getBoundingBox() const;
	/** Set the movement properties, typically called when the server sends
	 * this message for our entity. */
	void setMovementProperties(const MsgEntityMove* msg);
	/** Get movement properties */
	const MsgEntityMove* getMovementProperties() const;

protected:
	/// Class name
	std::string mClassName;

	/// The basic entity data
	MsgEntityCreate mEntityBasicData;
	/// The movement data
	MsgEntityMove mEntityMovData;

	/// The engine representation of this entity
	osg::Node* mNode;
	/// The engine representation of the position
	osg::MatrixTransform* mTransform;
	/// The engine representation of the bounding box (for collision
	/// detection with other entities)
	osg::BoundingBox* mBoundingBox;
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
