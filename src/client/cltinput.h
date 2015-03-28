/*
 * cltinput.h
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

/** \file cltinput.h
 *
 * Classes to handle keyboard and mouse input.
 */

#ifndef __FEARANN_CLIENT_INPUT_H__
#define __FEARANN_CLIENT_INPUT_H__


#include "common/patterns/singleton.h"

#include <osgGA/GUIEventHandler>


class CltCEGUIEventHandler;


/** Event handler to capture keyboard events, controlling the camera and so on.
 */
class CltKeyboardHandler : public osgGA::GUIEventHandler 
{
public:
	/** Default constructor */
	CltKeyboardHandler();

	/** Default destructor */
	~CltKeyboardHandler();

	/** Class name of this handler */
	virtual const char* className() const;
	/** Handle the events */
	virtual bool handle(const osgGA::GUIEventAdapter& ea,
			    osgGA::GUIActionAdapter& aa,
			    osg::Object* o,
			    osg::NodeVisitor* nv);
	/** Handle the events */
	virtual bool handle(const osgGA::GUIEventAdapter& ea,
			    osgGA::GUIActionAdapter& aa);

private:
	/// Name that will be used as identifier
	std::string mClassName;
	/// Slave handler, for CEGUI
	CltCEGUIEventHandler* mCEGUIEventHandler;

};


/** Controls main player movement and actions, based on the input.
 */
class CltMainPlayerManipulator : public Singleton<CltMainPlayerManipulator>
{
public:
	/** Feeded every frame to know the time since last update sent */
	void tick(double elapsedSeconds);

	/** Set the full position of the player in the given area. */
	void setPosition(const osg::Vec3& position, float rotation);

	/** Set this action state. */
	void setMovingForward(bool b);
	/** Set this action state. */
	void setMovingBackward(bool b);
	/** Set this action state. */
	void setRunning(bool b);
	/** Toggle autorunning. */
	void toggleAutoRunning();
	/** Set this action state. */
	void setRotatingLeft(bool b);
	/** Set this action state. */
	void setRotatingRight(bool b);
	/** Find out whether the action is idle, or it's doing something
	 * (shortcut, returning OR of the boolean values). */
	bool isIdle();

	/** Get the transformation matrix, for other classes which might need to
	 * view or manipulate it. */
	osg::MatrixTransform* getMatrixTransform() const;

private:
	/** Singleton friend access */
	friend class Singleton<CltMainPlayerManipulator>;

	/// The transformation representing the position of the player, and
	/// where's looking at
	osg::MatrixTransform* mPlayerTransform;

	/// Speed when walking
	static const float WALK_SPEED;
	/// Speed when running
	static const float RUN_SPEED;
	/// Speed when rotating left of right
	static const float ROTATE_SPEED;

	/// Action state
	bool mMovingForward;
	/// Action state
	bool mMovingBackward;
	/// Action state
	bool mRunning;
	/// Action state
	bool mRotatingLeft;
	/// Action state
	bool mRotatingRight;


	/** Default constructor */
	CltMainPlayerManipulator();
	/** Default destructor */
	~CltMainPlayerManipulator();

	/** Manipulate transformation (position and so on) according with the
	 * current state (called every frame, to update it if needed). */
	void updateTransform(double elapsedSeconds);

	/** Manipulate model according with the current state (called every
	 * frame, to update it if needed). */
	void update3DModel(double elapsedSeconds);

	/** Send information to the server (done internally in predefined
	 * periods). */
	void sendStateToServer();
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
