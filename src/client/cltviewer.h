/*
 * cltviewer.h
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

/** \file cltviewer.h
 *
 * @author mafm
 *
 * Classes for the viewer, i.e., the 3D client application.
 */

#ifndef __FEARANN_CLIENT_VIEWER_H__
#define __FEARANN_CLIENT_VIEWER_H__


#include "common/patterns/singleton.h"

#include <osg/Vec3>

class BoundingSphere;

namespace osg {
	class Group;
	class LightModel;
	class LightSource;
	class Node;
	class ShapeDrawable;
}
namespace osgViewer {
	class Viewer;
}
namespace osgGA {
	class CameraManipulator;
}
namespace osgParticle {
	class PrecipitationEffect;
}


/** Client 3D application.
 *
 * @author mafm
 */
class CltViewer : public Singleton<CltViewer>
{
public:
	/** Get window dimension (or fullscreen resolution). */
	uint32_t getWindowWidth() const;
	/** Get window dimension (or fullscreen resolution). */
	uint32_t getWindowHeight() const;

	/** Get terrain height. */
	float getTerrainHeight(const osg::Vec3& position) const;
	/** Clip the camera to a suitable position.
	 *
	 * The viewer casts a ray into the scene, with the given endpoints,
	 * source being the center of the camera, such as player head,
	 * destination where the camera should be in the case that there's
	 * nothing in the middle.
	 *
	 * The third parameter is used to return the final position, which is
	 * either the destination (in the case that there's nothing in the
	 * middle), or a position just before the object in the middle and on
	 * the ground, which is what the camera mode should use. */
	void cameraClipping(const osg::Vec3& source, const osg::Vec3& dest, osg::Vec3& finalPos);
	/** Collision detection, simple fashion.
	 *
	 * This takes the source and the destination intended, and gives the
	 * resulting final position, including setting the position on the
	 * ground, which is what the function to move entities should use.
	 *
	 * \remark This solution is simplistic, since it doesn't take into
	 * account collision detection when the object is not positioned
	 * on the ground (fences, in example). */
	void applyCollisionAndTerrainHeight(const std::string& selfName,
					    const osg::Vec3& source,
					    const osg::Vec3& dest,
					    osg::Vec3& finalPos);

	/** Set up the initial window with our desired data. */
	void setup();
	/** Set full screen. */
	void setFullScreen(bool b);
	/** Whether we're in full screen mode. */
	bool isFullScreen() const;
	/** Load the scene.  It should be called when receiving the 'create main
	 * player' message, so we load the area and set up the main player
	 * related classes. */
	void loadScene(const std::string& area);
	/** Add node to the scene. */
	void addToScene(osg::Node* node);
	/** Remove a node from the scene. */
	void removeFromScene(osg::Node* node);
	/** Start rendering, calls the renderLoop. */
	void start();
	/** Stop rendering, the window quits. */
	void stop();
	/** Change the environment according with the time of the day. */
	void setEnvironment(uint32_t timeOfDay);
	/** Select something in the environment */
	uint32_t pick(float x, float y);

private:
	/** Singleton friend access */
	friend class Singleton<CltViewer>;


	/// The engine viewer
	osgViewer::Viewer* mViewer;

	/// The scene, root as group of nodes
	osg::Group* mScene;

	/// The camera manipulator
        osgGA::CameraManipulator* mCameraManipulator;

	/// Terrain node, to be able to find the height of it
	osg::Node* mTerrainNode;

	/// Sky dome
	osg::ShapeDrawable* mSkyDome;
	/// Sun lightsource
	osg::LightSource* mSun;
	/// Precipitations
	osgParticle::PrecipitationEffect* mPrecipitation;

	/// Light model of the scene
	osg::LightModel* mLightModel;

	/// Window dimension/resolution
	unsigned int mWindowWidth;
	/// Window dimension/resolution
	unsigned int mWindowHeight;


	/** Default constructor */
	CltViewer();
	/** Destructor */
	~CltViewer();

	/** The render loop. */
	void renderLoop();

	/** Create a sun light. */
	osg::Node* createSun(const osg::Vec3& position);

	/** Change the lights according with the time of the day. */
	void setLights(uint32_t timeOfDay);
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
