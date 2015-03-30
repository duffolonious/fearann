/*
 * cltcamera.h
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

#ifndef __FEARANN_CLIENT_CAMERA_H__
#define __FEARANN_CLIENT_CAMERA_H__


#include "common/patterns/singleton.h"

#include <osgGA/CameraManipulator>
#include <osg/Vec3>

#include <deque>
#include <list>


/** Event handler controlling the camera, linking between our application and
 * the engine.
 */
class CltCameraManipulator : public osgGA::CameraManipulator
{
public:
	/** Return the name, to be identified */
	virtual const char* className() const;

	/** Handle events */
	virtual bool handle(const osgGA::GUIEventAdapter& ea,
			    osgGA::GUIActionAdapter& aa);

	/** Set the matrix of the manipulator */
	virtual void setByMatrix(const osg::Matrixd& matrix);
	/** Set the matrix of the manipulator */
	virtual void setByInverseMatrix(const osg::Matrixd& matrix);
	/** Get the matrix of the manipulator */
	virtual osg::Matrixd getMatrix() const;
	/** Get the matrix of the manipulator */
	virtual osg::Matrixd getInverseMatrix() const;

protected:
	/// The matrix to manipulate the camera
	osg::Matrix mMatrix;
};


/** Camera mode, implementing the actions asked to the camera and behaving
 * different according to its own nature.
 *
 * This is an abstract class that it's implemented by several modes.  We keep
 * common functionality implemented here, like the node that all cameras follow
 * (they will calculate the offset and looking-at with the target node as bae).
 */
class CltCameraMode : public CltCameraManipulator
{
public:
	/** Default constructor */
	CltCameraMode();
	/** Destructor */
	virtual ~CltCameraMode() { }

	/** Get the name */
	const char* getName() const;

	/** Set flag for this camera action */
	void setZoomingIn(bool b);
	/** Set flag for this camera action */
	void setZoomingOut(bool b);
	/** Set flag for this camera action */
	void setLookingUp(bool b);
	/** Set flag for this camera action */
	void setLookingDown(bool b);
	/** Set flag for this camera action */
	void setLookingLeft(bool b);
	/** Set flag for this camera action */
	void setLookingRight(bool b);
	/** Set flag for this camera action */
	void setRequestToCenter(bool b);

	/** Reset to home position. */
	virtual void reset() = 0;
	/** It's called every frame via camera manager, with the milliseconds
	 * elapsed since last update, so we move the camera of the engine
	 * (depending on camera mode and commands issued by player), and thus
	 * control how do we view the scene.  This is where each camera mode
	 * behaves different. */
	virtual void updateCamera(osg::Matrix* cameraView, double elapsedSeconds) = 0;

protected:
	/// Default
	static const osg::Vec3 DEFAULT_GROUND_TO_EYE;

	/// Name of the mode
	std::string mName;

	/// Flag for camera action
	bool mZoomingIn;
	/// Flag for camera action
	bool mZoomingOut;
	/// Flag for camera action
	bool mLookingUp;
	/// Flag for camera action
	bool mLookingDown;
	/// Flag for camera action
	bool mLookingLeft;
	/// Flag for camera action
	bool mLookingRight;
	/// Flag for camera action
	bool mRequestToCenter;

	/// The transformation of the node that we should follow
	const osg::MatrixTransform* mTargetTransform;

private:
	/// Friend access to the camera manager, so it can set some functions
	/// for all the cameras that shouldn't be globally accessable
	friend class CltCameraMgr;

	/** Set the transformation of the node that the cameras should follow
	 * (typically the player).  This function is private with the camera
	 * manager accessing as friend, so the external code sets the target
	 * globally and not for each camera mode. */
	void setTargetTransform(const osg::MatrixTransform* targetTransform);
};


/** Follow camera mode.  The behavior of this camera mode is that zooming in
 * leads to first person view (where is possible to look up, down, left and
 * right), and zoom out leads to a distant point above and behind the player.
 */
class CltCameraModeFollow : public CltCameraMode
{
public:
	/** Default constructor */
	CltCameraModeFollow();

	/** @see CltCameraMode */
	virtual void reset();
	/** @see CltCameraMode */
	virtual void updateCamera(osg::Matrix* cameraView, double elapsedSeconds);

private:
	/// Speed of the zoom in/out
	static const float ZOOM_SPEED;
	/// Maximum zoom distance
	static const float ZOOM_MAX_DISTANCE;
	/// Minimum zoom distance
	static const float ZOOM_MIN_DISTANCE;
	/// Speed of the "look" rotation
	static const float LOOK_SPEED;
	/// Maximum angle of the rotation (left-right, up-down)
	static const float LOOK_MAX_ANGLE;
	/// Default
	static const float DEFAULT_HORIZONTAL_ROT;
	/// Default
	static const float DEFAULT_VERTICAL_ROT;
	/// Default
	static const float DEFAULT_ZOOM_DISTANCE;

	/// Zoom distance
	float mZoomDistance;
	/// Horizontal rotation
	float mHorizRot;
	/// Vertical rotation
	float mVertRot;
};


/** Orbital camera mode.  The behavior of this camera mode is that it orbits the
 * player, having it at the center, with zoom to control the radius and the keys
 * to look up/down/left/right controlling movement from "pole to pole" and
 * "ecuator".  This mode fulfills special needs that the other mode can't cover,
 * so you can turn around to see your character from any angle.
 */
class CltCameraModeOrbital : public CltCameraMode
{
public:
	/** Default constructor */
	CltCameraModeOrbital();

	/** @see CltCameraMode */
	virtual void reset();
	/** @see CltCameraMode */
	virtual void updateCamera(osg::Matrix* cameraView, double elapsedSeconds);

private:
	/// Speed of the zoom in/out (to set the radius)
	static const float RADIUS_SPEED;
	/// Maximum radius distance
	static const float RADIUS_MAX_DISTANCE;
	/// Minimum radius distance
	static const float RADIUS_MIN_DISTANCE;
	/// Speed of the rotation
	static const float ROTATION_SPEED;
	/// Default
	static const float DEFAULT_HORIZONTAL_ROT;
	/// Default
	static const float DEFAULT_VERTICAL_ROT;
	/// Default
	static const float DEFAULT_RADIUS_DISTANCE;

	/// Radius distance
	float mRadiusDistance;
	/// Horizontal rotation
	float mHorizRot;
	/// Vertical rotation
	float mVertRot;
};


/** Listener for the event of changing the active camera.
 */
class CltCameraListener
{
public:
	/** Notification that the camera mode changed */
	virtual void cameraModeChanged(const CltCameraMode* newMode) = 0;
	/** Default destructor */
	virtual ~CltCameraListener() { }
};


/** Governs the camera: modes being used, etc.  The main loop of the client has
 * to call this function every frame before rendering, so the manager passes
 * down the info to the active camera mode, which in turn tells the engine
 * camera to render the scene (with the parameters given by the camera mode).
 */
class CltCameraMgr : public Singleton<CltCameraMgr>
{
public:
	/** Set the transformation of the node that the cameras should follow
	 * (typically the player) */
	void setTargetTransform(const osg::MatrixTransform* targetTransform);
  
	/** Get the active camera mode */
	CltCameraMode& getActiveCameraMode();
	/** Cycle the camera mode to the next one */
	void cycleCameraMode();

	/** Add a listener for events */
	void addListener(CltCameraListener* listener);
	/** Remove a listener */
	void removeListener(CltCameraListener* listener);

private:
	/** Singleton friend access */
	friend class Singleton<CltCameraMgr>;

	/// List of camera modes that we can use (front is the active one)
	std::deque<CltCameraMode*> mCameraModeList;

	/// List of listeners subscribed to our events
	std::list<CltCameraListener*> mListenerList;


	/** Default constructor */
	CltCameraMgr();
	/** Destructor */
	~CltCameraMgr();

	/** Notify the listeners that the camera mode changed */
	void notifyListenersCameraModeChanged(const CltCameraMode* mode);
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
