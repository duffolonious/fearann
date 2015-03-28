/*
 * cltentityplayer.cpp
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
#include "client/cltconfig.h"

#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgCal/CoreModel>
#include <osgCal/Model>
#include <cal3d/coremodel.h>

#include "common/net/msgs.h"

#include "client/cltcamera.h"
#include "client/cegui/cltceguiinventory.h"
#include "client/content/cltcontentloader.h"
#include "client/net/cltnetmgr.h"
#include "client/cltviewer.h"

#include "cltentitycreature.h"


//----------------------- CltEntityCreature ----------------------------
const float CltEntityCreature::WALK_SPEED = 2.0f; // 2m/s = 7.2km/h
const float CltEntityCreature::RUN_SPEED = 5.0f; // 5m/s = 18km/h
const float CltEntityCreature::ROTATE_SPEED = (2*PI_NUMBER)/4.0f; // in rad/s, 4s for full revolution

CltEntityCreature::CltEntityCreature(const MsgEntityCreate* entityBasicData) :
	CltEntityBase(entityBasicData, "Creature"), mOSGCal3DModel(0)
{
	// bounding box for players (maybe we need to tweak it for every race,
	// but it should be OK at least for testing)
	mBoundingBox->set(-0.33f, -0.33f, 0.0f,
			  0.33f, 0.33f, 1.75f);
}

CltEntityCreature::~CltEntityCreature()
{
}

osgCal::Model* CltEntityCreature::loadModel()
{
	// load model with the help of the content loader
	mOSGCal3DModel = new osgCal::Model();
	mNode = mOSGCal3DModel;
	osgCal::CoreModel* coreModel = CltContentLoader::instance().loadCal3DCoreModel(mEntityBasicData.meshType,
										       mEntityBasicData.meshSubtype);
	if (!mOSGCal3DModel->setCoreModel(coreModel)) {
		LogERR("Loading cal3d model: %s", CalError::getLastErrorDescription().c_str());
		return 0;
	}

	for (int i = 0; i < mOSGCal3DModel->getCalCoreModel()->getCoreMeshCount(); ++i) {
		mOSGCal3DModel->getCalModel()->attachMesh(i);
	}

	// set the material set of the whole model
	mOSGCal3DModel->getCalModel()->setMaterialSet(0);

	// creating a concrete model using the core template
	if (!mOSGCal3DModel->create()) {
		LogERR("Creating cal3d model: %s", CalError::getLastErrorDescription().c_str());
		return 0;
	}

	// set the node name as player name
	mOSGCal3DModel->setName(mEntityBasicData.entityName);

	return mOSGCal3DModel;
}

void CltEntityCreature::setMovementProperties(const MsgEntityMove* entityMovData)
{
	mEntityMovData = *entityMovData;

	// action
	if (mEntityMovData.run && mEntityMovData.mov_fwd)
		setAction(ACTION_RUN);
	else if (mEntityMovData.mov_fwd || mEntityMovData.mov_bwd)
		setAction(ACTION_WALK);
	else
		setAction(ACTION_IDLE);

	// mafm: translation is absolute, not depending on the rotation, so it's
	// set in different order than the rest of similar situations in this
	// class
	osg::Vec3 p(mEntityMovData.position.x, mEntityMovData.position.y, mEntityMovData.position.z);
	float terrainHeight = CltViewer::instance().getTerrainHeight(p);
	p[2] = terrainHeight;
	mEntityMovData.position.z = terrainHeight;
	osg::Matrix matrix = osg::Matrix::rotate(mEntityMovData.rot, osg::Vec3(0, 0, 1))
		* osg::Matrix::translate(p);
	mTransform->setMatrix(matrix);

	/*
	LogNTC("Position of player '%s' changed to: (%.1f, %.1f, %.1f), rot=%.1f",
	       getName(), p.x(), p.y(), p.z(), mEntityMovData.rot);
	*/
}

void CltEntityCreature::setAction(Cal3DActions action)
{
	mOSGCal3DModel->getCalModel()->getMixer()->clearCycle(ACTION_IDLE, 0.2f);
	mOSGCal3DModel->getCalModel()->getMixer()->clearCycle(ACTION_WALK, 0.2f);
	mOSGCal3DModel->getCalModel()->getMixer()->clearCycle(ACTION_RUN, 0.2f);
	mOSGCal3DModel->getCalModel()->getMixer()->blendCycle(action, 0.8f, 0.5f);
}

void CltEntityCreature::updateTransform(double elapsedSeconds)
{
	// mafm: This is similar to the control of the main player, but it's not
	// autonomous.
	//
	// To calculate the new position of the player, we start by calculating
	// the displacement from the current position.  To achieve this, we
	// calculate the speed (walking or running), then whether is moving
	// forward or backward (y=1 and y=-1 respectively), and applying the
	// rotation; all of this related to the time elapsed.
	//
	// Once we get this, we have to apply the modification over the previous
	// position and look-at "attitude".

	bool mRunning = mEntityMovData.run;
	bool mMovingForward = mEntityMovData.mov_fwd;
	bool mMovingBackward = mEntityMovData.mov_bwd;
	bool mRotatingLeft = mEntityMovData.rot_left;
	bool mRotatingRight = mEntityMovData.rot_right;

	// idle?
	if (!mEntityMovData.mov_fwd && !mEntityMovData.mov_bwd
	    && !mEntityMovData.rot_left && mEntityMovData.rot_right) {
		return;
	}

	// basic speed (not used if not moving)
	float linearSpeed = WALK_SPEED * elapsedSeconds;
	if (mRunning && mMovingForward)
		linearSpeed = RUN_SPEED * elapsedSeconds;

	// rotation (affecting final displacement vector, and rotation of the 3D
	// model)
	float rotation = 0.0f;
	if (mRotatingLeft) {
		rotation = ROTATE_SPEED * elapsedSeconds;
	} else if (mRotatingRight) {
		rotation = ROTATE_SPEED * elapsedSeconds * (-1);
	}

	// vector of linear displacement (when walking forward, backwards is the
	// opposite), not counting rotation
	osg::Vec3 linearDisplacement(0, 0, 0);
	if (mMovingForward) {
		// rotation influence in linear movement:
		// - x is the sin(rotation)*y, y=1*linearSpeed
		// - y is the cos(rotation)*y, y=1*linearSpeed
		// - z axis not affected
		linearDisplacement[0] = sin(rotation) * linearSpeed;
		linearDisplacement[1] = cos(rotation) * linearSpeed;
	} else if (mMovingBackward) {
		// rotation is inverted, to result how player expects
		rotation = -rotation;

		// rotation influence in linear movement:
		// - x is the sin(rotation)*y, y=-1*linearSpeed
		// - y is the cos(rotation)*y, y=-1*linearSpeed
		// - z axis not affected
		linearDisplacement[0] = sin(rotation) * linearSpeed * (-1);
		linearDisplacement[1] = cos(rotation) * linearSpeed * (-1);
	}

	// do collision det and terrain height, only when moving and not
	// rotating (saves from warning messages about invalid ray segments of
	// length zero)
	if (linearDisplacement != osg::Vec3(0, 0, 0)) {
		osg::Vec3 source = mTransform->getMatrix().getTrans();
		// make a matrix for the new position (with rotation around Z
		// axis)
		osg::Matrix destMatrix = osg::Matrix::translate(linearDisplacement)
			* osg::Matrix::rotate(rotation, osg::Vec3(0, 0, 1))
			* mTransform->getMatrix();
		osg::Vec3 dest = destMatrix.getTrans();
		osg::Vec3 finalPos;
		CltViewer::instance().applyCollisionAndTerrainHeight(mEntityBasicData.entityName,
								     source, dest, finalPos);
		destMatrix.setTrans(finalPos);

		// finally, set the new position
		mTransform->setMatrix(destMatrix);
	} else if (rotation != 0.0) {
		// only rotation
		osg::Matrix destMatrix = osg::Matrix::rotate(rotation, osg::Vec3(0, 0, 1))
			* mTransform->getMatrix();
		mTransform->setMatrix(destMatrix);
	}

/*
	osg::Vec3 p = mTransform->getMatrix().getTrans();
	LogDBG("Player '%s' position internal update: (%.1f, %.1f, %.1f)",
	       getName(), p.x(), p.y(), p.z());
*/
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
