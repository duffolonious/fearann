/*
 * cltentityobject.h
 * Copyright (C) 2006 by Bryan Duff <duff0097@umn.edu>
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

#ifndef __FEARANN_CLIENT_ENTITY_OBJECT_H__
#define __FEARANN_CLIENT_ENTITY_OBJECT_H__


#include "cltentitybase.h"

/** Controls a regular player entity (other than the main player), which handles
 * most of the game logic.
 */
class CltEntityObject : public CltEntityBase
{
public: 
	/** Default constructor */
	CltEntityObject(const MsgEntityCreate* entityBasicData);
	/** Destructor */
	~CltEntityObject();

	/** Load the model */
	osg::Node* loadModel();
	/** Set movement properties (overriden from base class, because we have
	 * to deal with animations and so on too). */
	void setMovementProperties(const MsgEntityMove* entityMovData);

	/** Function to get called every frame, to update the position of
	 * the entity in the world. */
	virtual void updateTransform(double elapsedSeconds);

protected:
	/// osg model
	osg::Node* mModel;

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
