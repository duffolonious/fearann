/*
 * cltinput.cpp
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

#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/Vec3>

#include "common/net/msgs.h"

#include "cltcamera.h"
#include "cltviewer.h"
#include "cltentitymgr.h"
#include "client/cegui/cltceguidrawable.h"
#include "client/cegui/cltceguiinitial.h"
#include "client/cegui/cltceguimgr.h"
#include "client/cegui/cltceguiminimap.h"
#include "client/cegui/cltceguiactionmenu.h"
#include "client/entity/cltentitymainplayer.h"
#include "client/net/cltnetmgr.h"

#include "cltinput.h"

using namespace osg;


//--------------------------- CltKeyboardHandler ----------------------
CltKeyboardHandler::CltKeyboardHandler() :
	mClassName("KeyboardEventHandler")
{
	mCEGUIEventHandler = new CltCEGUIEventHandler();
}

CltKeyboardHandler::~CltKeyboardHandler()
{
	delete mCEGUIEventHandler;
}

const char* CltKeyboardHandler::className() const
{
	return mClassName.c_str();
}

bool CltKeyboardHandler::handle(const osgGA::GUIEventAdapter& ea,
				osgGA::GUIActionAdapter& aa,
				osg::Object* /* o */,
				osg::NodeVisitor* /* nv */)
{
	return handle(ea, aa);
}

bool CltKeyboardHandler::handle(const osgGA::GUIEventAdapter& ea,
				osgGA::GUIActionAdapter& aa)
{
	// passing events to CEGUI first

	/// \todo mafm: CEGUI not working, disable to avoid segfaults
	/*
	  bool handled = mCEGUIEventHandler->handle(ea, aa);
	  if (handled)
		return true;
	*/

	switch (ea.getEventType()) {
	case (osgGA::GUIEventAdapter::KEYDOWN) :
	{
		if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F1) {
			// toggle fullscreen
			CltViewer::instance().setFullScreen(!CltViewer::instance().isFullScreen());
			return true;
		} else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Up || 
		    ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Up) {
			// start moving forward
			CltMainPlayerManipulator::instance().setMovingForward(true);
			return true;
		} else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Down || 
			   ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Down) {
			// start moving backward
			CltMainPlayerManipulator::instance().setMovingBackward(true);
			return true;
		} else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Left || 
			   ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Left) {
			// start rotating left
			CltMainPlayerManipulator::instance().setRotatingLeft(true);
			return true;
		} else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Right || 
			   ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Right) {
			// start rotating right
			CltMainPlayerManipulator::instance().setRotatingRight(true);
			return true;
		} else if (ea.getKey() ==  osgGA::GUIEventAdapter::KEY_Shift_L
			   || ea.getKey() == osgGA::GUIEventAdapter::KEY_Shift_R) {
			// set running mode
			CltMainPlayerManipulator::instance().setRunning(true);
			return true;
		} else if (ea.getKey() == 'R' || ea.getKey() == 'r') {
			// autorunning (toggle, nothing when unpressed)
			CltMainPlayerManipulator::instance().toggleAutoRunning();
			return true;
		} else if (ea.getKey() == 'C' || ea.getKey() == 'c') {
			// cycle camera mode
			CltCameraMgr::instance().cycleCameraMode();
			return true;
		} else if (ea.getKey() == 'E' || ea.getKey() == 'e') {
			// instruct camera to be centered
			CltCameraMgr::instance().getActiveCameraMode().setRequestToCenter(true);
			return true;
		} else if (ea.getKey() == 'Z' || ea.getKey() == 'z') {
			// start zooming in
			CltCameraMgr::instance().getActiveCameraMode().setZoomingIn(true);
			return true;
		} else if (ea.getKey() == 'Q' || ea.getKey() == 'q') {
			// start zooming out
			CltCameraMgr::instance().getActiveCameraMode().setZoomingOut(true);
			return true;
		} else if (ea.getKey() == 'W' || ea.getKey() == 'w') {
			// start looking up
			CltCameraMgr::instance().getActiveCameraMode().setLookingUp(true);
			return true;
		} else if (ea.getKey() == 'S' || ea.getKey() == 's') {
			// start looking down
			CltCameraMgr::instance().getActiveCameraMode().setLookingDown(true);
			return true;
		} else if (ea.getKey() == 'A' || ea.getKey() == 'a') {
			// start looking left
			CltCameraMgr::instance().getActiveCameraMode().setLookingLeft(true);
			return true;
		} else if (ea.getKey() == 'D' || ea.getKey() == 'd') {
			// start looking right
			CltCameraMgr::instance().getActiveCameraMode().setLookingRight(true);
			return true;
		} else if (ea.getKey() == 'I' || ea.getKey() == 'i') {
			// inventory window (toggle)

			/// \todo mafm: CEGUI not working, disable to avoid segfaults
			//CltCEGUIMgr::instance().ToggleWindow_Inventory();

			return true;
		} else if (ea.getKey() == '+' ||
			   ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Add) {
			// minimap zoom in

			/// \todo mafm: CEGUI not working, disable to avoid segfaults
			//CltCEGUIMinimap::instance().increaseZoomLevel();

			return true;
		} else if (ea.getKey() == '-' ||
			   ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Subtract) {
			// minimap zoom out

			/// \todo mafm: CEGUI not working, disable to avoid segfaults
			//CltCEGUIMinimap::instance().decreaseZoomLevel();

			return true;
		} else {
			return false;
		}
		break;
	}
	case (osgGA::GUIEventAdapter::KEYUP) :
	{
		if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Up || 
		    ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Up) {
			// stop moving forward
			CltMainPlayerManipulator::instance().setMovingForward(false);
			return true;
		} else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Down || 
			   ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Down) {
			// stop moving backward
			CltMainPlayerManipulator::instance().setMovingBackward(false);
			return true;
		} else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Left || 
			   ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Left) {
			// stop rotating left
			CltMainPlayerManipulator::instance().setRotatingLeft(false);
			return true;
		} else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Right || 
			   ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Right) {
			// stop rotating right
			CltMainPlayerManipulator::instance().setRotatingRight(false);
			return true;
		} else if (ea.getKey() ==  osgGA::GUIEventAdapter::KEY_Shift_L
			   || ea.getKey() == osgGA::GUIEventAdapter::KEY_Shift_R) {
			// set running mode
			CltMainPlayerManipulator::instance().setRunning(false);
			return true;
		} else if (ea.getKey() == 'Z' || ea.getKey() == 'z') {
			// stop zooming in
			CltCameraMgr::instance().getActiveCameraMode().setZoomingIn(false);
			return true;
		} else if (ea.getKey() == 'Q' || ea.getKey() == 'q') {
			// stop zooming out
			CltCameraMgr::instance().getActiveCameraMode().setZoomingOut(false);
			return true;
		} else if (ea.getKey() == 'W' || ea.getKey() == 'w') {
			// stop looking up
			CltCameraMgr::instance().getActiveCameraMode().setLookingUp(false);
			return true;
		} else if (ea.getKey() == 'S' || ea.getKey() == 's') {
			// stop looking down
			CltCameraMgr::instance().getActiveCameraMode().setLookingDown(false);
			return true;
		} else if (ea.getKey() == 'A' || ea.getKey() == 'a') {
			// stop looking left
			CltCameraMgr::instance().getActiveCameraMode().setLookingLeft(false);
			return true;
		} else if (ea.getKey() == 'D' || ea.getKey() == 'd') {
			// stop looking right
			CltCameraMgr::instance().getActiveCameraMode().setLookingRight(false);
			return true;
		} else {
			return false;
		}
		break;
	}
	case (osgGA::GUIEventAdapter::PUSH):
	{
		if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) {
			LogDBG("Left mouse button clicked.");
			CltViewer::instance().pick(ea.getX(), ea.getY());
		}
		break;
	}
	case (osgGA::GUIEventAdapter::RELEASE):
	{
		if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) {
			LogDBG("Left mouse button released.");
		} else if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) {
			LogDBG("Right mouse button released.");
			///\todo: duffolonious: mesh selection.
			/*
			  CltCEGUIActionMenu::instance().ShowAt(ea.getXnormalized(),
							      ea.getYnormalized() );
			*/
		}
		break;
	}
	case (osgGA::GUIEventAdapter::FRAME):
	{
		static double lastTime = 0.0;

		if (lastTime != 0.0) {
			double elapsedSeconds = ea.time() - lastTime;

			// main player movement
			if (CltEntityMainPlayer::isInitialized()) {
				CltMainPlayerManipulator::instance().tick(elapsedSeconds);
			}

			// updating other entities
			CltEntityMgr::instance().updateTransforms(elapsedSeconds);

			// incoming network messages
			CltNetworkMgr::instance().processIncomingMsgs();

			if (!CltEntityMainPlayer::isInitialized()) {
				// for the pings in the connection screen.  we
				// have to proccess it immediately to avoid
				// spurious delays (reply arrived but we don't
				// know the timestamp difference because it's
				// unprocessed).

				/// \todo mafm: CEGUI not working, disable to avoid segfaults
				//CltCEGUIInitial::instance().Connect_ProcessPingReplies(ea.time());

				// to send pings, it will send pings every 3
				// seconds.
				static double lastPing = -10.0;
				if (ea.time() > lastPing+3.0) {
					lastPing = ea.time();

					/// \todo mafm: CEGUI not working, disable to avoid segfaults
					//CltCEGUIInitial::instance().Connect_SendPings(lastPing);
				}
			}
		}
		lastTime = ea.time();
		return false;
	}
	default:
		return false;
	}
	return false;
}


//--------------------------- CltMainPlayerManipulator ----------------------
const float CltMainPlayerManipulator::WALK_SPEED = 2.0f; // 2m/s = 7.2km/h
const float CltMainPlayerManipulator::RUN_SPEED = 5.0f; // 5m/s = 18km/h
const float CltMainPlayerManipulator::ROTATE_SPEED = (2*PI_NUMBER)/4.0f; // in rad/s, 4s for full revolution

template <> CltMainPlayerManipulator* Singleton<CltMainPlayerManipulator>::INSTANCE = 0;

CltMainPlayerManipulator::CltMainPlayerManipulator()
{
	mMovingForward = false;
	mMovingBackward = false;
	mRunning = false;
	mRotatingLeft = false;
	mRotatingRight = false;
	mPlayerTransform = new osg::MatrixTransform();
}

CltMainPlayerManipulator::~CltMainPlayerManipulator()
{
}

void CltMainPlayerManipulator::setPosition(const osg::Vec3& position, float rotation)
{
	// mafm: translation is absolute, not depending on the rotation, so it's
	// set in different order than the rest of similar situations in this
	// class
	float terrainHeight = CltViewer::instance().getTerrainHeight(position);
	osg::Vec3 p(position.x(), position.y(), terrainHeight);
	osg::Matrix matrix = osg::Matrix::rotate(rotation, osg::Vec3(0, 0, 1))
		* osg::Matrix::translate(p);
	mPlayerTransform->setMatrix(matrix);

	LogNTC("Position of the main player changed to: (%.1f, %.1f, %.1f), rot=%.1f",
	       p.x(), p.y(), p.z(), rotation);
}

void CltMainPlayerManipulator::tick(double elapsedSeconds)
{
	// updating position and so on
	updateTransform(elapsedSeconds);

	// updating 3D model states
	update3DModel(elapsedSeconds);

	// updating every 5s if not moving
	static double elapsedSinceServerUpdate = 0.0;
	elapsedSinceServerUpdate += elapsedSeconds;
	if (elapsedSinceServerUpdate > 5.0) {
		sendStateToServer();
		elapsedSinceServerUpdate = 0.0;
	// updating every 0.5s if it's moving, to reduce innacuracies
	} else if (elapsedSinceServerUpdate > 0.5 && !isIdle()) {
		sendStateToServer();
		elapsedSinceServerUpdate = 0.0;
	}
}

void CltMainPlayerManipulator::updateTransform(double elapsedSeconds)
{
	// mafm: To calculate the new position of the player, we start by
	// calculating the displacement from the current position.  To achieve
	// this, we calculate the speed (walking or running), then whether is
	// moving forward or backward (y=1 and y=-1 respectively), and applying
	// the rotation; all of this related to the time elapsed.
	//
	// Once we get this, we have to apply the modification over the previous
	// position and look-at "attitude".

	if (isIdle()) {
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
		string selfName = CltEntityMainPlayer::instance().getName();
		osg::Vec3 source = mPlayerTransform->getMatrix().getTrans();
		// make a matrix for the new position (with rotation around Z
		// axis)
		osg::Matrix destMatrix = osg::Matrix::translate(linearDisplacement)
			* osg::Matrix::rotate(rotation, osg::Vec3(0, 0, 1))
			* mPlayerTransform->getMatrix();
		osg::Vec3 dest = destMatrix.getTrans();
		osg::Vec3 finalPos;
		CltViewer::instance().applyCollisionAndTerrainHeight(selfName, source, dest, finalPos);
		destMatrix.setTrans(finalPos);

		// finally, set the new position
		mPlayerTransform->setMatrix(destMatrix);
	} else if (rotation != 0.0) {
		// only rotation
		osg::Matrix destMatrix = osg::Matrix::rotate(rotation, osg::Vec3(0, 0, 1))
			* mPlayerTransform->getMatrix();
		mPlayerTransform->setMatrix(destMatrix);
	}
}

void CltMainPlayerManipulator::update3DModel(double elapsedSeconds)
{
	// position + linear movement
	MsgEntityMove msg;

	osg::Vec3 trans = mPlayerTransform->getMatrix().getTrans();
	msg.position.x = trans.x();
	msg.position.y = trans.y();
	msg.position.z = trans.z();

	/** \todo mafm: do something to retrieve this, it's not straightforward
	 * with OSG 1.0 (maybe it is with 1.1, matrix have additional functions
	 * to get rotations).
	 *
	 * duffolonious: fixed with OSG 1.2.
	 * rotation(left: +1, right: -1) * angle(0 - 1 radian)
	 * I think the initial direction (0 rotation) is vec(0, -1, 0);
	 */
	osg::Vec3f rotation;
	double rotation_angle;
	mPlayerTransform->getMatrix().getRotate().getRotate(rotation_angle, rotation);
	msg.rot = rotation.z() * rotation_angle;
	//LogDBG("rot: %f", msg.rot);

	// action
	if (mMovingForward) {
		msg.mov_fwd = true;
		msg.mov_bwd = false;
	} else if (mMovingBackward) {
		msg.mov_fwd = false;
		msg.mov_bwd = true;
	} else {
		msg.mov_fwd = false;
		msg.mov_bwd = false;
	}

	msg.run = mRunning && mMovingForward;

	if (mRotatingLeft) {
		msg.rot_left = true;
		msg.rot_right = false;
	} else if (mRotatingRight) {
		msg.rot_left = false;
		msg.rot_right = true;
	} else {
		msg.rot_left = false;
		msg.rot_right = false;
	}

	// update the model with the data gathered
	CltEntityMainPlayer::instance().setMovementProperties(&msg);
}

void CltMainPlayerManipulator::sendStateToServer()
{
	// copying to avoid const qualifier
	MsgEntityMove msg = *CltEntityMainPlayer::instance().getMovementProperties();
	CltNetworkMgr::instance().sendToServer(msg);

/** \todo mafm: tests for combat/trader manager. I put them here
 * because when here we know that we're connected

	MsgCombat msg2;
	msg2.player = "BogusPlayer";
	msg2.target = "BogusTarget";
	msg2.state = MsgCombat::START;
	CltNetworkMgr::instance().sendToServer(msg2);

	MsgCombat msg3;
	msg3.player = "Elfinha";
	msg3.target = "Elfinha";
	msg3.state = MsgCombat::START;
	msg3.type = MsgCombat::DUEL;
	CltNetworkMgr::instance().sendToServer(msg3);
*/
}

void CltMainPlayerManipulator::setMovingForward(bool b)
{
	mMovingForward = b;
}

void CltMainPlayerManipulator::setMovingBackward(bool b)
{
	mMovingBackward = b;
}

void CltMainPlayerManipulator::setRunning(bool b)
{
	mRunning = b;
}

void CltMainPlayerManipulator::toggleAutoRunning()
{
	if (mRunning) {
		mRunning = false;
		mMovingForward = false;
	} else {
		mRunning = true;
		mMovingForward = true;
	}
}

void CltMainPlayerManipulator::setRotatingRight(bool b)
{
	mRotatingRight = b;
}

void CltMainPlayerManipulator::setRotatingLeft(bool b)
{
	mRotatingLeft = b;
}

bool CltMainPlayerManipulator::isIdle()
{
	return ! (mMovingForward
		  || mMovingBackward
		  || mRotatingLeft
		  || mRotatingRight);
}

osg::MatrixTransform* CltMainPlayerManipulator::getMatrixTransform() const
{
	return mPlayerTransform;
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
