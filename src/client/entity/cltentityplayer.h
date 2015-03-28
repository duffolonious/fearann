/*
 * cltentityplayer.h
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

#ifndef __FEARANN_CLIENT_ENTITY_PLAYER_H__
#define __FEARANN_CLIENT_ENTITY_PLAYER_H__


#include "cltentitybase.h"

namespace osgCal {
	class Model;
}


/** Controls a regular player entity (other than the main player), which handles
 * most of the game logic.
 */
class CltEntityPlayer : public CltEntityBase
{
public:
	/** Actions of the player, for Cal3D animations. */
	enum Cal3DActions {
		ACTION_IDLE = 0,
		ACTION_WALK,
		ACTION_RUN,
	};

public: 
	/** Default constructor */
	CltEntityPlayer(const MsgEntityCreate* entityBasicData);
	/** Default destructor */
	~CltEntityPlayer();

	/** Load the model */
	osgCal::Model* loadModel();
	/** Set movement properties (overriden from base class, because we have
	 * to deal with animations and so on too). */
	void setMovementProperties(const MsgEntityMove* entityMovData);

	/** Function to get called every frame, to update the position of the
	 * entity in the world. */
	virtual void updateTransform(double elapsedSeconds);

protected:
	/// osgCal3D model
	osgCal::Model* mOSGCal3DModel;

	/// Speed when walking
	static const float WALK_SPEED;
	/// Speed when running
	static const float RUN_SPEED;
	/// Speed when rotating left of right
	static const float ROTATE_SPEED;

	/** Set the action (animation) to perform */
	void setAction(Cal3DActions action);
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
