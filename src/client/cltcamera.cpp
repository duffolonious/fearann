/*
 * cltcamera.cpp
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

#include "config.h"
#include "client/cltconfig.h"

#include "client/cltviewer.h"

#include "cltcamera.h"

#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/Vec3>
#include <osg/BoundingSphere>
#include <osgGA/CameraManipulator>


//-------------------------- CltCameraManipulator -------------------------
const char* CltCameraManipulator::className() const
{
	return "FearannCameraManipulator";
}

#include <unistd.h>
bool CltCameraManipulator::handle(const osgGA::GUIEventAdapter& ea,
				  osgGA::GUIActionAdapter& /* aa */)
{
	switch (ea.getEventType()) {
	case (osgGA::GUIEventAdapter::FRAME):
	{
		static double lastTime = 0.0;
		if (lastTime != 0.0)
			CltCameraMgr::instance().getActiveCameraMode().updateCamera(CltViewer::instance().getCamera());
		lastTime = ea.time();
		return false;
	}
	default:
		return false;
	}

	return false;
}

void CltCameraManipulator::setByMatrix(const osg::Matrixd& matrix)
{
	mMatrix = matrix;
}

void CltCameraManipulator::setByInverseMatrix(const osg::Matrixd& matrix)
{
	mMatrix = mMatrix.inverse(matrix);
}

osg::Matrixd CltCameraManipulator::getMatrix() const
{
	return mMatrix;
}

osg::Matrixd CltCameraManipulator::getInverseMatrix() const
{
	return mMatrix.inverse(mMatrix);
}


//-------------------------- CltCameraMode -------------------------
const osg::Vec3 CltCameraMode::DEFAULT_GROUND_TO_EYE(0.0f, 0.0f, 1.60f);

CltCameraMode::CltCameraMode() :
	mName("Base"),
	mZoomingIn(false), mZoomingOut(false),
	mLookingUp(false), mLookingDown(false),
	mLookingLeft(false), mLookingRight(false),
	mRequestToCenter(false),
	mTargetTransform(0)
{
}

const char* CltCameraMode::getName() const
{
	return mName.c_str();
}

void CltCameraMode::setTargetTransform(const osg::MatrixTransform* targetTransform)
{
	mTargetTransform = targetTransform;
}

void CltCameraMode::setZoomingIn(bool b)
{
	mZoomingIn = b;
}

void CltCameraMode::setZoomingOut(bool b)
{
	mZoomingOut = b;
}

void CltCameraMode::setLookingUp(bool b)
{
	mLookingUp = b;
}

void CltCameraMode::setLookingDown(bool b)
{
	mLookingDown = b;
}

void CltCameraMode::setLookingLeft(bool b)
{
	mLookingLeft = b;
}

void CltCameraMode::setLookingRight(bool b)
{
	mLookingRight = b;
}

void CltCameraMode::setRequestToCenter(bool b)
{
	mRequestToCenter = b;
}


//-------------------------- CltCameraModeFollow -------------------------
const float CltCameraModeFollow::ZOOM_SPEED = 12.0f; // 12m/s
const float CltCameraModeFollow::ZOOM_MAX_DISTANCE = -24.0f; // 24m behind
const float CltCameraModeFollow::ZOOM_MIN_DISTANCE = 0.5f; // 0.5m before, to bypass the mesh
const float CltCameraModeFollow::LOOK_SPEED = PI_NUMBER/2.0f; // 4s for full revolution
const float CltCameraModeFollow::LOOK_MAX_ANGLE = PI_NUMBER/2.0f; // PI/2
const float CltCameraModeFollow::DEFAULT_HORIZONTAL_ROT = 0.0f; // degrees
const float CltCameraModeFollow::DEFAULT_VERTICAL_ROT = 0.0f; // degrees
const float CltCameraModeFollow::DEFAULT_ZOOM_DISTANCE = ZOOM_MAX_DISTANCE/3.0f; // m

CltCameraModeFollow::CltCameraModeFollow() :
	mZoomDistance(0.0f), mHorizRot(0.0f), mVertRot(0.0f)
{
	mName = "Follow";
	reset();
}

void CltCameraModeFollow::reset()
{
	mZoomDistance = DEFAULT_ZOOM_DISTANCE;
	mHorizRot = DEFAULT_HORIZONTAL_ROT;
	mVertRot = DEFAULT_VERTICAL_ROT;
}

void CltCameraModeFollow::updateCamera(osg::Camera& cameraView)
{
	/** FIXME: hack to assume 30 FPS on how much time has elapsed because this
	 * function should only take the Camera ptr */
	double elapsedSeconds = 0.033;
	// whether we're already in the game with a node assigned
	if (!mTargetTransform)
		return;

	// react to events
	if (mRequestToCenter) {
		// reset when requested
		reset();
		mRequestToCenter = false;
	} else {
		// apply displacement due to zoom
		if (mZoomingIn) {
			mZoomDistance += ZOOM_SPEED * elapsedSeconds;
			// zoom distance limit to the position of the player
			// (Less because < 0)
			ensureLessOrEqual(mZoomDistance, ZOOM_MIN_DISTANCE);
		}
		if (mZoomingOut) {
			mZoomDistance -= ZOOM_SPEED * elapsedSeconds;
			// distance limit (Greater because < 0)
			ensureGreaterOrEqual(mZoomDistance, ZOOM_MAX_DISTANCE);
		}

		// apply "loot-at" rotations, only when we have 1st p. view
		if (mZoomDistance == ZOOM_MIN_DISTANCE) {
			// treating only full zoom, ~= 1st person mode
			if (mLookingUp) {
				mVertRot += LOOK_SPEED * elapsedSeconds;
				ensureLessOrEqual(mVertRot, LOOK_MAX_ANGLE);
			}
			if (mLookingDown) {
				mVertRot -= LOOK_SPEED * elapsedSeconds;
				ensureGreaterOrEqual(mVertRot, -LOOK_MAX_ANGLE);
			}

			if (mLookingLeft) {
				mHorizRot += LOOK_SPEED * elapsedSeconds;
				ensureLessOrEqual(mHorizRot, LOOK_MAX_ANGLE);
			}
			if (mLookingRight) {
				mHorizRot -= LOOK_SPEED * elapsedSeconds;
				ensureGreaterOrEqual(mHorizRot, -LOOK_MAX_ANGLE);
			}
		} else {
			// reset when we're not in 1st person view
			mHorizRot = DEFAULT_HORIZONTAL_ROT;
			mVertRot = DEFAULT_VERTICAL_ROT;
		}
	}

	// basic position
	osg::Matrix targetMatrix = osg::Matrix::translate(DEFAULT_GROUND_TO_EYE)
		* mTargetTransform->getMatrix();

	// look matrix
	osg::Matrix lookMatrix = osg::Matrix::rotate(mVertRot, osg::Vec3(1, 0, 0))
			* osg::Matrix::rotate(mHorizRot, osg::Vec3(0, 0, 1));

	// zoom distance-away
	osg::Matrix finalMatrix = osg::Matrix::translate(0, mZoomDistance, 0)
		* (lookMatrix * targetMatrix);

	// set the camera view
	osg::Matrix lookAt;
	if (mZoomDistance != ZOOM_MIN_DISTANCE) {
		// clipping -- only when reasonably far away
		if (mZoomDistance < -0.5f) {
			osg::Matrix clipSourceMatrix = osg::Matrix::translate(DEFAULT_GROUND_TO_EYE + osg::Vec3(0, -1, 0))
				* (targetMatrix);
			osg::Vec3 clipFinal;
			CltViewer::instance().cameraClipping(clipSourceMatrix.getTrans(),
							     finalMatrix.getTrans(),
							     clipFinal);
			finalMatrix.setTrans(clipFinal);
		}

		lookAt.makeLookAt(finalMatrix.getTrans(),
				  targetMatrix.getTrans(),
				  osg::Z_AXIS);
	} else {
		// with full zoom (= 1st person view), override to not cause
		// artifacts
		osg::Matrix fakeTargetMatrix = osg::Matrix::translate(osg::Y_AXIS)
			* finalMatrix;
		lookAt.makeLookAt(finalMatrix.getTrans(),
				  fakeTargetMatrix.getTrans(),
				  osg::Z_AXIS);
	}
	cameraView.setViewMatrix(osg::Matrix::inverse(lookAt));
}


//-------------------------- CltCameraModeOrbital -------------------------
const float CltCameraModeOrbital::RADIUS_SPEED = 12.0f; // 12m/s
const float CltCameraModeOrbital::RADIUS_MAX_DISTANCE = 24.0f; // 24m
const float CltCameraModeOrbital::RADIUS_MIN_DISTANCE = 2.0f; // 2m
const float CltCameraModeOrbital::ROTATION_SPEED = PI_NUMBER/2.0f; // 4s for full revolution
const float CltCameraModeOrbital::DEFAULT_HORIZONTAL_ROT = 0.0f; // degrees
const float CltCameraModeOrbital::DEFAULT_VERTICAL_ROT = -PI_NUMBER/24.0f; // degrees
const float CltCameraModeOrbital::DEFAULT_RADIUS_DISTANCE = RADIUS_MAX_DISTANCE/3.0f; // m

CltCameraModeOrbital::CltCameraModeOrbital() :
	mRadiusDistance(0.0f), mHorizRot(0.0f), mVertRot(0.0f)
{
	mName = "Orbital";
	reset();
}

void CltCameraModeOrbital::reset()
{
	mRadiusDistance = DEFAULT_RADIUS_DISTANCE;
	mHorizRot = DEFAULT_HORIZONTAL_ROT;
	mVertRot = DEFAULT_VERTICAL_ROT;
}

void CltCameraModeOrbital::updateCamera(osg::Camera& cameraView)
{
	/** FIXME: hack to assume 30 FPS on how much time has elapsed because this
	 * function should only take the Camera ptr */
	double elapsedSeconds = 0.033;
	// mafm: getting the node to follow some point in the orbit and so on
	// isn't difficult, but at the time to implement this I couldn't get an
	// easy way to rotate the camera towards the player, so the player is
	// always in the center of the image.
	//
	// This alternative implementation seems to be simpler and more
	// efficient: we just rotate in any direction (like the look-at of the
	// main camera mode, but without any restriction), and in the end we
	// "push" the camera back the given radius.  It seems to work flawlessly
	// :)

	// whether we're already in the game with a node assigned
	if (!mTargetTransform)
		return;

	// react to events
	if (mRequestToCenter) {
		// reset when requested
		reset();
		mRequestToCenter = false;
	} else {
		// radius/zoom distance
		if (mZoomingIn) {
			mRadiusDistance -= RADIUS_SPEED * elapsedSeconds;
			ensureGreaterOrEqual(mRadiusDistance, RADIUS_MIN_DISTANCE);
		}
		if (mZoomingOut) {
			mRadiusDistance += RADIUS_SPEED * elapsedSeconds;
			ensureLessOrEqual(mRadiusDistance, RADIUS_MAX_DISTANCE);
		}

		// orbit rotations
		if (mLookingUp) {
			mVertRot += ROTATION_SPEED * elapsedSeconds;
		}
		if (mLookingDown) {
			mVertRot -= ROTATION_SPEED * elapsedSeconds;
		}
		if (mLookingLeft) {
			mHorizRot += ROTATION_SPEED * elapsedSeconds;
		}
		if (mLookingRight) {
			mHorizRot -= ROTATION_SPEED * elapsedSeconds;
		}
	}

	// basic position
	osg::Matrix targetMatrix = osg::Matrix::translate(DEFAULT_GROUND_TO_EYE)
		* mTargetTransform->getMatrix();

	// orbit matrix
	osg::Matrix lookMatrix = osg::Matrix::rotate(mVertRot, osg::Vec3(1, 0, 0))
		* osg::Matrix::rotate(mHorizRot, osg::Vec3(0, 0, 1));

	// "pushing" the camera the given radius away
	osg::Vec3f radiusDispl = osg::Vec3(0, -mRadiusDistance, 0);
	osg::Matrix finalMatrix = osg::Matrix::translate(radiusDispl)
		* (lookMatrix * targetMatrix);

	// clipping
	osg::Matrix clipSourceMatrix = osg::Matrix::translate(radiusDispl/mRadiusDistance)
		* (lookMatrix * targetMatrix);
	osg::Vec3 clipFinal;
	CltViewer::instance().cameraClipping(clipSourceMatrix.getTrans(),
					     finalMatrix.getTrans(),
					     clipFinal);
	finalMatrix.setTrans(clipFinal);

	// set the camera view
	osg::Matrix lookAt;
	lookAt.makeLookAt(finalMatrix.getTrans(),
			  targetMatrix.getTrans(),
			  osg::Z_AXIS);
	cameraView.setViewMatrix(osg::Matrix::inverse(lookAt));
}


//-------------------------- CltCameraMgr -------------------------
template <> CltCameraMgr* Singleton<CltCameraMgr>::INSTANCE = 0;

CltCameraMgr::CltCameraMgr()
{
	// create camera modes that we'll use (default is the first one, it will
	// get in the front of the list)
	mCameraModeList.push_back(new CltCameraModeFollow());
	mCameraModeList.push_back(new CltCameraModeOrbital());
}

CltCameraMgr::~CltCameraMgr()
{
	while (!mCameraModeList.empty()) {
		delete mCameraModeList.front();
		mCameraModeList.pop_front();
	}
}

CltCameraMode& CltCameraMgr::getActiveCameraMode()
{
	return *(mCameraModeList.front());
}

void CltCameraMgr::setTargetTransform(const osg::MatrixTransform* targetTransform)
{
	for (deque<CltCameraMode*>::iterator it = mCameraModeList.begin(); it != mCameraModeList.end(); ++it) {
		(*it)->setTargetTransform(targetTransform);
	}
}

void CltCameraMgr::cycleCameraMode()
{
	// cycle mode
	mCameraModeList.push_back(mCameraModeList.front());
	mCameraModeList.pop_front();

	// notify to the interested classes
	notifyListenersCameraModeChanged(mCameraModeList.front());
}

void CltCameraMgr::addListener(CltCameraListener* listener)
{
	// adding element, not checking for duplicates
	mListenerList.push_back(listener);
}

void CltCameraMgr::removeListener(CltCameraListener* listener)
{
	// removing element, including duplicates
	mListenerList.remove(listener);
}

void CltCameraMgr::notifyListenersCameraModeChanged(const CltCameraMode* mode)
{
	for (list<CltCameraListener*>::iterator it = mListenerList.begin(); it != mListenerList.end(); ++it) {
		(*it)->cameraModeChanged(mode);
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
