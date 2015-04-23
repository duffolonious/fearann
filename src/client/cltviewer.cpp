/*
 * cltviewer.cpp
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

/** \file cltviewer.cpp
 *
 * @author mafm
 *
 * Classes for the viewer, i.e., the 3D client application.
 */

#include "config.h"
#include "client/cltconfig.h"

#include "cltviewer.h"

#include "common/configmgr.h"
#include "common/net/msgs.h"

#include "cltentitymgr.h"
#include "client/cegui/cltceguimgr.h"
#include "client/cegui/cltceguidrawable.h"
#include "client/content/cltcontentloader.h"
#include "client/entity/cltentitymainplayer.h"
#include "client/net/cltnetmgr.h"
#include "cltcamera.h"
#include "cltinput.h"

#include <unistd.h>

#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Fog>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Math>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Switch>
#include <osg/Texture2D>
#include <osg/LightSource>
#include <osg/LightModel>
#include <osg/TexGenNode>
#include <osg/Timer>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgUtil/IntersectVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgText/Text>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgCal/Model>
#include <osgParticle/PrecipitationEffect>


/** Helper class to make the render loop sleep when the FPS goes to high.  Huge
 * rendering speeds are of no benefit for the eye, while consuming energy and
 * slowing down CPU.  A limit of 60FPS (more than the double than TV update
 * frequencies) should be more than reasonable limitation.
 *
 * @author mafm
 */
class FPSLimitator
{
public:
	/** Constructor with target FPS limit as parameter */
	FPSLimitator(float fps) {
		mInterval = static_cast<uint32_t>(1000000.0f/fps);
	}

	/** Read the time when the frame begins (so the limitation takes the
	 * processing time into accoount) */
	void readTime() {
		mLastTick = osg::Timer::instance()->tick();
	}

	/** Sleep the necessary amount of time to effectively achieve the FPS
	 * limitation. */
	void sleep() {
		int32_t sleepFor = mInterval - (osg::Timer::instance()->tick() - mLastTick);
		if (sleepFor > 0) {
			usleep(sleepFor);
		}
		/*
		LogDBG("FPS: %.1f (%.1f)",
			   1000000.0f/static_cast<float>(osg::Timer::instance()->tick() - mLastTick),
			   static_cast<float>(osg::Timer::instance()->tick() - mLastTick));
		*/
	}

private:
	/// The interval between frames (microseconds) for FPS limit
	uint32_t mInterval;
	/// Tick when the frame was started
	static osg::Timer_t mLastTick;
};
osg::Timer_t FPSLimitator::mLastTick = 0;


//--------------------------- CltViewer ----------------------
template <> CltViewer* Singleton<CltViewer>::INSTANCE = 0;

CltViewer::CltViewer() :
	mViewer(0), mScene(0), mCameraManipulator(0),
	mTerrainNode(0), mSkyDome(0), mSun(0), mPrecipitation(0),
	mWindowWidth(0), mWindowHeight(0)
{
}

CltViewer::~CltViewer()
{
	//FIXME: causes a segfault on exit - maybe switch to a ref_ptr and release?
	//delete mViewer;
}

uint32_t CltViewer::getWindowWidth() const
{
	return mWindowWidth;
}

uint32_t CltViewer::getWindowHeight() const
{
	return mWindowHeight;
}

void CltViewer::setup()
{
	// check whether we already have the window set up
	if (mViewer != 0) {
		LogWRN("Refusing to set up the viewer more than once");
		return;
	}

	// read options from the config file
	mWindowWidth = atoi(ConfigMgr::instance().getConfigVar("Window.ScreenWidth", "0"));
	mWindowHeight = atoi(ConfigMgr::instance().getConfigVar("Window.ScreenHeight", "0"));
	if (mWindowWidth == 0 || mWindowHeight == 0) {
		LogWRN("Can't get the window dimensions from the config file");
		return;
	}
	bool fullscreen = false;
	if (string("yes") == ConfigMgr::instance().getConfigVar("Window.FullScreen", "no")) {
		fullscreen = true;
	}

	// window characteristics
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 0;
	traits->y = 0;
	traits->width = mWindowWidth;
	traits->height = mWindowHeight;
	traits->windowDecoration = true;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->useCursor = false; //Use the CEGUI cursor instead.
	traits->windowName = "Fearann Muin Client";
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

	// set up viewer and camera
	mViewer = new osgViewer::Viewer();
	osg::Camera* camera = mViewer->getCamera();
	camera->setGraphicsContext(gc.get());
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
	camera->setDrawBuffer(buffer);
	camera->setReadBuffer(buffer);

	// duffolonious: doesn't show anything without this line:
	mViewer->addSlave(camera,
			  osg::Matrixd::translate(0, 0, 0),
			  osg::Matrixd());

	/// \note mafm: quick exit, mainly for devel purposes
	mViewer->setKeyEventSetsDone(osgGA::GUIEventAdapter::KEY_Escape);

	// camera manipulator
	mCameraManipulator = new CltCameraManipulator();
	mViewer->setCameraManipulator(mCameraManipulator);

	// set up the event handler.
	//
	// mafm: Note that it seems to be an issue with the way that this is
	// handled (only one seems to be active at a time), and we need a
	// fine-tuning for this (layer-like, CEGUI being the first to receive
	// the input events).  So it's this primary event handler who calls the
	// other ones in the correct order.
	mViewer->addEventHandler(new CltKeyboardHandler());
	mViewer->addEventHandler((osgGA::GUIEventHandler *)mCameraManipulator);

	// create the window, switch to fullscreen if apropriate
	mViewer->realize();
	PERM_ASSERT(mViewer->isRealized());
	setFullScreen(fullscreen);

	// create the scene
	mScene = new osg::Group();
	mScene->getOrCreateStateSet()->setMode(GL_LIGHTING,
					       osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	// sky dome
	{
		// variables
		float radius = 1024.0f;
		osg::Vec3 center(0.0f, 0.0f, 0.0f);
		osg::Vec4 color(0.0f, 0.0f, 0.0f, 1.0f);

		mSkyDome = new osg::ShapeDrawable(new osg::Sphere(center, radius));
		mSkyDome->setColor(color);
		osg::Geode* geode = new osg::Geode();
		geode->setName("SkyDome");
		geode->addDrawable(mSkyDome);
		geode->getOrCreateStateSet()->setMode(GL_LIGHTING,
						      osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		mScene->addChild(geode);
	}

	// sun
	osg::Vec3 sunPosition(-1000, 100, 1000);
	mScene->addChild(createSun(sunPosition));

	// fog
	{/** \todo mafm: doesn't work

		float radius = 1024.0f;
		osg::Vec4 fogColor(0.8f, 0.8f, 0.8f, 1.0f);
		osg::Fog* fog = new osg::Fog();
		fog->setFogCoordinateSource(osg::Fog::FRAGMENT_DEPTH);
		fog->setMode(osg::Fog::LINEAR);
		fog->setStart(-50.0f);
		fog->setEnd(-radius);
		fog->setDensity(0.1f);
		fog->setColor(fogColor);
		osg::Geode* geode = new osg::Geode();
		geode->setName("Fog");
		geode->addDrawable(mSkyDome);
		geode->getOrCreateStateSet()->setAttributeAndModes(fog,
								   osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		mScene->addChild(geode);
	 */}

	// precipitation
	{/** \todo mafm WIP

		mPrecipitation = new osgParticle::PrecipitationEffect();
		mPrecipitation->snow(0.5);
		mPrecipitation->rain(0.5);
		mPrecipitation->getFog()->setDensity(0.5);
		osg::Vec4 color(1, 1, 1, 0.5);
		mPrecipitation->setNearTransition(1.0f);
		mPrecipitation->setFarTransition(50.0f);
		mPrecipitation->setParticleColor(color);
		mScene->addChild(mPrecipitation);

		LogDBG("%g %g", mPrecipitation->getNearTransition(), mPrecipitation->getFarTransition());
	 */}

	// add the GUI
	CltCEGUIDrawable* ceguiDrawable = new CltCEGUIDrawable(mWindowWidth, mWindowHeight);
	mScene->addChild(ceguiDrawable->getNode());

	// finally, set up the scene in the viewer
	mViewer->setSceneData(mScene);

	/* mafm: for testing, readily available world without login into the
        * server
	*/
	bool connected = CltNetworkMgr::instance().connectToServer("localhost",
								   20768);

	MsgConnect msgConnect;
	CltNetworkMgr::instance().sendToServer(msgConnect);

	MsgLogin msgLogin;
        msgLogin.username = "testuser";
	msgLogin.pw_md5sum = "0e0e68cc27a6334256e0752d1243c4d894e56869";
	CltNetworkMgr::instance().sendToServer(msgLogin);

	MsgJoin msgJoin;
        msgJoin.charname = "testchar";
	CltNetworkMgr::instance().sendToServer(msgJoin);

	/*
	MsgNewUser msgNewUser;
        msgNewUser.username = "mafm";
	msgNewUser.pw_md5sum = "0e0e68cc27a6334256e0752d1243c4d894e56869";
        msgNewUser.email = "mafm@fearann.muin";
        msgNewUser.realname = "Manuel Montecelo";
	CltNetworkMgr::instance().sendToServer(msgNewUser);

	MsgNewChar msgNewChar;
        msgNewChar.charname = "Elfo";
	msgNewChar.race = "elf";
	msgNewChar.gender = "m";
	msgNewChar.playerClass = "sorcerer";
	msgNewChar.ab_choice_str = 13;
	msgNewChar.ab_choice_con = 13;
	msgNewChar.ab_choice_dex = 13;
	msgNewChar.ab_choice_int = 13;
	msgNewChar.ab_choice_wis = 13;
	msgNewChar.ab_choice_cha = 13;
	CltNetworkMgr::instance().sendToServer(msgNewChar);

	MsgEntityCreate fakeMainPlayer;
	fakeMainPlayer.entityID = 0;
	fakeMainPlayer.position = Vector3(-35, -65, 0);
	fakeMainPlayer.rot = 2.5f;
	fakeMainPlayer.area = "tmprotmar";
	fakeMainPlayer.meshType = "elf";
	fakeMainPlayer.meshSubtype = "m";
	fakeMainPlayer.entityName = "Test Player";
	fakeMainPlayer.entityClass = "MainPlayer";
	CltEntityMgr::instance().entityCreate(&fakeMainPlayer);
	*/
}

void CltViewer::start()
{
	renderLoop();
}

void CltViewer::stop()
{
	mViewer->setDone(true);
}

void CltViewer::renderLoop()
{
	// simple limitator to make it sleep when we reach > 60FPS
	FPSLimitator fpsLim(60.0f);

	// main loop
	while (!mViewer->done()) {
		// read the time when the frame begins
		fpsLim.readTime();

		// render a complete new frame
		mViewer->frame();

		// sleep for some time, to achieve limitation
		fpsLim.sleep();
	}
}

void CltViewer::setFullScreen(bool b)
{
	/// \todo mafm: reimplement in newer versions of OSG

	//mRenderSurface->fullScreen(b);
}

bool CltViewer::isFullScreen() const
{
	/// \todo mafm: reimplement in newer versions of OSG

	//return mRenderSurface->isFullScreen();

	return false;
}

void CltViewer::addToScene(osg::Node* node)
{
	mScene->addChild(node);
}

void CltViewer::removeFromScene(osg::Node* node)
{
	mScene->removeChild(node);
}

void CltViewer::loadScene(const std::string& area)
{
	LogNTC("Loading scene:");

	// load the terrain and buildings
	osg::Node* buildings = 0;
	CltContentLoader::instance().loadArea(area, &mTerrainNode, &buildings);
	mTerrainNode->getOrCreateStateSet()->setMode(GL_LIGHTING,
						osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	mTerrainNode->addDescription("terrain");
	mScene->addChild(mTerrainNode);
	buildings->getOrCreateStateSet()->setMode(GL_LIGHTING,
					  	osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	mScene->addChild(buildings);
	LogNTC(" - Area '%s' loaded successfully", area.c_str());

	// main player
	osg::Node* playerNode = CltEntityMainPlayer::instance().loadModel();
	playerNode->getOrCreateStateSet()->setMode(GL_LIGHTING,
					   osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	playerNode->addDescription(StrFmt("%lu", CltEntityMainPlayer::instance().getID()));
	osg::MatrixTransform* playerTransform = CltMainPlayerManipulator::instance().getMatrixTransform();
	playerTransform->addChild(playerNode);
	mScene->addChild(playerTransform);
	LogNTC(" - Main player loaded successfully");
}

osg::Node* CltViewer::createSun(const osg::Vec3& position)
{
	// refusing to create the sun more than once
	if (mSun)
		return 0;

	// create light source
	osg::Group* group = new osg::Group;
	mSun = new osg::LightSource();

	osg::Light* light = mSun->getLight();
	light->setLightNum(0);
	light->setPosition(osg::Vec4(position, 1.0f));
	light->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	light->setSpecular(osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f));
	light->setConstantAttenuation(0.0f);
	light->setLinearAttenuation(0.001f);
	light->setQuadraticAttenuation(0.0f);

	osg::Material* material = new osg::Material;
	material->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 0.0f, 0.0f));
	mSun->getOrCreateStateSet()->setAttributeAndModes(material, osg::StateAttribute::ON);

	mLightModel = new osg::LightModel();
	mSun->getOrCreateStateSet()->setAttribute(mLightModel);

	group->addChild(mSun);

	osg::Vec3 direction(0.0f, 1.0f, -1.0f);
	float angle = 45.0f;

	// create tex gen
	osg::Vec3 up(0.0f, 0.0f, 1.0f);
	up = (direction ^ up) ^ direction;
	up.normalize();

	osg::TexGenNode* texgenNode = new osg::TexGenNode;
	texgenNode->setTextureUnit(1);
	osg::TexGen* texgen = texgenNode->getTexGen();
	texgen->setMode(osg::TexGen::EYE_LINEAR);
	texgen->setPlanesFromMatrix(osg::Matrixd::lookAt(position, position+direction, up)*
					osg::Matrixd::perspective(angle, 1.0f, 0.1f, 100));

	group->addChild(texgenNode);

	return group;
}

void CltViewer::setEnvironment(uint32_t timeOfDay)
{
	// set the lights of the different elements
	setLights(timeOfDay);
}

void CltViewer::setLights(uint32_t timeOfDay)
{
	// mafm: we save the time of the last update, so we know whether the
	// last update was performed or not.  It might be not performed when: 1)
	// it's the first update since we connected to the client; or 2) the
	// admin changed the time in the server (which can happen when testing).
	//
	// Knowing this means that we avoid to set lightning colors again and
	// again when not needed, which is during most of the time -- we only
	// change colors around dawn and dusk.  Thus we save precious FPS,
	// because lightning changes are very expensive.
	static int previousTimeOfDay = -10000; // -1 may conflict with comparison
	bool previousUpdateWasPerformed = false;
	if (previousTimeOfDay == (static_cast<int>(timeOfDay)-1)
		|| (timeOfDay == 0 && previousTimeOfDay == 1440)) {
		previousUpdateWasPerformed = true;
	}
	previousTimeOfDay = timeOfDay;

	// initializing variables to be used
	const float noTransparency = 1.0f;
	osg::Vec4 sunColor(0.0f, 0.0f, 0.0f, noTransparency);
	osg::Vec4 ambientColor(0.0f, 0.0f, 0.0f, noTransparency);
	osg::Vec4 skyColor(0.0f, 0.0f, 0.0f, noTransparency);

	// magic numbers obtained by experimentation, it gives neat lightning
	// effects especially at dawn/dusk
	float sun_night_red = 0.123f;
	float sun_night_green = 0.123f;
	float sun_night_blue = 0.12f;
	osg::Vec4 sunNight(sun_night_red, sun_night_green, sun_night_blue, noTransparency);

	float sun_dawn_red = 0.9f;
	float sun_dawn_green = 0.45f;
	float sun_dawn_blue = 0.45f;
	osg::Vec4 sunDawn(sun_dawn_red, sun_dawn_green, sun_dawn_blue, noTransparency);

	float sun_day_red = 1.16f;
	float sun_day_green = 1.13f;
	float sun_day_blue = 1.1f;
	osg::Vec4 sunDay(sun_day_red, sun_day_green, sun_day_blue, noTransparency);

	float sun_dusk_red = 0.9f;
	float sun_dusk_green = 0.65f;
	float sun_dusk_blue = 0.45f;
	osg::Vec4 sunDusk(sun_dusk_red, sun_dusk_green, sun_dusk_blue, noTransparency);

	float ambient_night_red = 0.115f;
	float ambient_night_green = 0.1f;
	float ambient_night_blue = 0.15f;
	osg::Vec4 ambientNight(ambient_night_red, ambient_night_green, ambient_night_blue, noTransparency);

	float ambient_dawn_red = 0.21f;
	float ambient_dawn_green = 0.2f;
	float ambient_dawn_blue = 0.23f;
	osg::Vec4 ambientDawn(ambient_dawn_red, ambient_dawn_green, ambient_dawn_blue, noTransparency);

	float ambient_day_red = 0.43f;
	float ambient_day_green = 0.4f;
	float ambient_day_blue = 0.46f;
	osg::Vec4 ambientDay(ambient_day_red, ambient_day_green, ambient_day_blue, noTransparency);

	float ambient_dusk_red = 0.21f;
	float ambient_dusk_green = 0.2f;
	float ambient_dusk_blue = 0.23f;
	osg::Vec4 ambientDusk(ambient_dusk_red, ambient_dusk_green, ambient_dusk_blue, noTransparency);

	float sky_night_red = 0.055f;
	float sky_night_green = 0.055f;
	float sky_night_blue = 0.174f;
	osg::Vec4 skyNight(sky_night_red, sky_night_green, sky_night_blue, noTransparency);

	float sky_dawn_red = 0.292f;
	float sky_dawn_green = 0.198f;
	float sky_dawn_blue = 0.433f;
	osg::Vec4 skyDawn(sky_dawn_red, sky_dawn_green, sky_dawn_blue, noTransparency);

	float sky_day_red = 0.15f;
	float sky_day_green = 0.3f;
	float sky_day_blue = 1.0f;
	osg::Vec4 skyDay(sky_day_red, sky_day_green, sky_day_blue, noTransparency);

	float sky_dusk_red = 0.46f;
	float sky_dusk_green = 0.16f;
	float sky_dusk_blue = 0.018f;
	osg::Vec4 skyDusk(sky_dusk_red, sky_dusk_green, sky_dusk_blue, noTransparency);

	if (timeOfDay >= 1320 || timeOfDay < 300) {
		// night, 22:00 to 05:00
		if (previousUpdateWasPerformed && timeOfDay > 1320) {
			return;
		}
		sunColor = sunNight;
		ambientColor = ambientNight;
		skyColor = skyNight;
	} else if (timeOfDay >= 300 && timeOfDay < 390) {
		// night to dawn, 05:00 to 06:30
		float scale = float(timeOfDay - 300) / 90.0f;

		float sun_red = sun_night_red + (sun_dawn_red - sun_night_red) * scale;
		float sun_green = sun_night_green + (sun_dawn_green - sun_night_green) * scale;
		float sun_blue = sun_night_blue + (sun_dawn_blue - sun_night_blue) * scale;
		sunColor = osg::Vec4(sun_red, sun_green, sun_blue, noTransparency);

		float ambient_red = ambient_night_red + (ambient_dawn_red - ambient_night_red) * scale;
		float ambient_green = ambient_night_green + (ambient_dawn_green - ambient_night_green) * scale;
		float ambient_blue = ambient_night_blue + (ambient_dawn_blue - ambient_night_blue) * scale;
		ambientColor = osg::Vec4(ambient_red, ambient_green, ambient_blue, noTransparency);

		float sky_red = sky_night_red + (sky_dawn_red - sky_night_red) * scale;
		float sky_green = sky_night_green + (sky_dawn_green - sky_night_green) * scale;
		float sky_blue = sky_night_blue + (sky_dawn_blue - sky_night_blue) * scale;
		skyColor = osg::Vec4(sky_red, sky_green, sky_blue, noTransparency);
	} else if (timeOfDay >= 390 && timeOfDay < 480) {
		// dawn to day, 06:30 to 08:00
		float scale = float(timeOfDay - 390) / 90.0f;

		float sun_red = sun_dawn_red + (sun_day_red - sun_dawn_red) * scale;
		float sun_green = sun_dawn_green + (sun_day_green - sun_dawn_green) * scale;
		float sun_blue = sun_dawn_blue + (sun_day_blue - sun_dawn_blue) * scale;
		sunColor = osg::Vec4(sun_red, sun_green, sun_blue, noTransparency);

		float ambient_red = ambient_dawn_red + (ambient_day_red - ambient_dawn_red) * scale;
		float ambient_green = ambient_dawn_green + (ambient_day_green - ambient_dawn_green) * scale;
		float ambient_blue = ambient_dawn_blue + (ambient_day_blue - ambient_dawn_blue) * scale;
		ambientColor = osg::Vec4(ambient_red, ambient_green, ambient_blue, noTransparency);

		float sky_red = sky_dawn_red + (sky_day_red - sky_dawn_red) * scale;
		float sky_green = sky_dawn_green + (sky_day_green - sky_dawn_green) * scale;
		float sky_blue = sky_dawn_blue + (sky_day_blue - sky_dawn_blue) * scale;
		skyColor = osg::Vec4(sky_red, sky_green, sky_blue, noTransparency);
	} else if (timeOfDay >= 480 && timeOfDay < 1140) {
		// day, 08:00 to 19:00
		if (previousUpdateWasPerformed && timeOfDay > 480) {
			return;
		}
		sunColor = sunDay;
		ambientColor = ambientDay;
		skyColor = skyDay;
	} else if (timeOfDay >= 1140 && timeOfDay < 1230) {
		// day to dusk, 19:00 to 20:30
		float scale = float(timeOfDay - 1140) / 90.0f;

		float sun_red = sun_day_red + (sun_dusk_red - sun_day_red) * scale;
		float sun_green = sun_day_green + (sun_dusk_green - sun_day_green) * scale;
		float sun_blue = sun_day_blue + (sun_dusk_blue - sun_day_blue) * scale;
		sunColor = osg::Vec4(sun_red, sun_green, sun_blue, noTransparency);

		float ambient_red = ambient_day_red + (ambient_dusk_red - ambient_day_red) * scale;
		float ambient_green = ambient_day_green + (ambient_dusk_green - ambient_day_green) * scale;
		float ambient_blue = ambient_day_blue + (ambient_dusk_blue - ambient_day_blue) * scale;
		ambientColor = osg::Vec4(ambient_red, ambient_green, ambient_blue, noTransparency);

		float sky_red = sky_day_red + (sky_dusk_red - sky_day_red) * scale;
		float sky_green = sky_day_green + (sky_dusk_green - sky_day_green) * scale;
		float sky_blue = sky_day_blue + (sky_dusk_blue - sky_day_blue) * scale;
		skyColor = osg::Vec4(sky_red, sky_green, sky_blue, noTransparency);
	} else if (timeOfDay >= 1230 && timeOfDay < 1320) {
		// dusk to night, 20:30 to 22:00
		float scale = float(timeOfDay - 1230) / 90.0f;

		float sun_red = sun_dusk_red + (sun_night_red - sun_dusk_red) * scale;
		float sun_green = sun_dusk_green + (sun_night_green - sun_dusk_green) * scale;
		float sun_blue = sun_dusk_blue + (sun_night_blue - sun_dusk_blue) * scale;
		sunColor = osg::Vec4(sun_red, sun_green, sun_blue, noTransparency);

		float ambient_red = ambient_dusk_red + (ambient_night_red - ambient_dusk_red) * scale;
		float ambient_green = ambient_dusk_green + (ambient_night_green - ambient_dusk_green) * scale;
		float ambient_blue = ambient_dusk_blue + (ambient_night_blue - ambient_dusk_blue) * scale;
		ambientColor = osg::Vec4(ambient_red, ambient_green, ambient_blue, noTransparency);

		float sky_red = sky_dusk_red + (sky_night_red - sky_dusk_red) * scale;
		float sky_green = sky_dusk_green + (sky_night_green - sky_dusk_green) * scale;
		float sky_blue = sky_dusk_blue + (sky_night_blue - sky_dusk_blue) * scale;
		skyColor = osg::Vec4(sky_red, sky_green, sky_blue, noTransparency);
	}

	//mViewer->getCamera()->setClearColor(skyColor);
	mSkyDome->setColor(skyColor);
	mSun->getLight()->setAmbient(sunColor);
	mSun->getLight()->setDiffuse(sunColor);
	mSun->getLight()->setSpecular(sunColor);
	mLightModel->setAmbientIntensity(ambientColor);
}

float CltViewer::getTerrainHeight(const osg::Vec3& position) const
{
	osg::LineSegment* raySegment = new osg::LineSegment();
	raySegment->set(osg::Vec3(position.x(), position.y(), 999),
					osg::Vec3(position.x(), position.y(), -999));

	osgUtil::IntersectVisitor terrainElevation;
	terrainElevation.addLineSegment(raySegment);
	mTerrainNode->accept(terrainElevation);

	osgUtil::IntersectVisitor::HitList hits = terrainElevation.getHitList(raySegment);
	if (hits.empty()) {
		LogERR("Couldn't get terrain height");
		return -1000.0f;
	} else {
		osg::Vec3 hitPosition = hits.front().getWorldIntersectPoint();
		return hitPosition.z();
	}
}

void CltViewer::cameraClipping(const osg::Vec3& source, const osg::Vec3& dest, osg::Vec3& finalPos)
{
	osg::LineSegment* raySegment = new osg::LineSegment();
	raySegment->set(source, dest);

	osgUtil::IntersectVisitor cameraInters;
	cameraInters.addLineSegment(raySegment);
	mScene->accept(cameraInters);

	osgUtil::IntersectVisitor::HitList hits = cameraInters.getHitList(raySegment);
	if (hits.empty()) {
		finalPos = dest;
	} else {
		finalPos = hits.front().getWorldIntersectPoint();
		float terrainHeight = getTerrainHeight(finalPos);
		if (finalPos[2] < terrainHeight)
			finalPos[2] = terrainHeight;
		// LogDBG("Clipping camera! at: (%.1f, %.1f, %.1f)", finalPos.x(), finalPos.y(), finalPos.z());
	}
}

void CltViewer::applyCollisionAndTerrainHeight(const std::string& selfName,
						   const osg::Vec3& source,
						   const osg::Vec3& dest,
						   osg::Vec3& finalPos)
{
	// final position if nothing happens (overwritten if collision)
	finalPos = osg::Vec3(dest.x(), dest.y(), getTerrainHeight(dest));

	// uses a vector of given radius to calculate several positions in a
	// cylindric-like fashion, if we collide then the final position is the
	// source -- not moving at all
	osg::Vec3 center(dest.x(), dest.y(), getTerrainHeight(dest)+0.5f);
	osg::Vec3 radiusV(0, 0.33f, 0);
	for (float rotation = 0.0; rotation < PI_NUMBER*2.0f; rotation += PI_NUMBER/4.0f) {
		osg::Matrix destMatrix = osg::Matrix::translate(radiusV)
			* osg::Matrix::rotate(rotation, osg::Vec3(0, 0, 1));

		osg::Vec3 raySource(center);
		osg::Vec3 rayDest(center+destMatrix.getTrans());

		// interset and get hitlist
		osg::LineSegment* raySegment = new osg::LineSegment(raySource, rayDest);
		osgUtil::IntersectVisitor collisionDet;
		collisionDet.addLineSegment(raySegment);
		mScene->accept(collisionDet);
		osgUtil::IntersectVisitor::HitList hits = collisionDet.getHitList(raySegment);

		for (osgUtil::IntersectVisitor::HitList::iterator it = hits.begin(); it != hits.end(); ++it) {
			if (selfName == it->getGeode()->getName()) {
				// LogDBG("Self collision ignored ('%s')", it->getGeode()->getName().c_str());
				continue;
			} else {
				osg::Vec3 collision(it->getWorldIntersectPoint());
				LogDBG("Collision with '%s' at (%.3f, %.3f, %.3f)",
					   it->getGeode()->getName().c_str(),
					   collision.x(), collision.y(), collision.z());
				finalPos = source;
				return;
			}
		}
	}
}

uint32_t CltViewer::pick(float x, float y)
{
	osgUtil::LineSegmentIntersector::Intersections intersections;
	if (mViewer->computeIntersections(x, y, intersections)) {
		/* mafm: don't need to iterate, IMO
		for (osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
		     hitr != intersections.end();
		     ++hitr) {
		*/

		if (!intersections.begin()->nodePath.empty()) {
			// nodePath.back() is the 1st geode being hit
			osg::Node* target = intersections.begin()->nodePath.back();
			if (target->getNumDescriptions() > 0) {
				uint64_t id = StrToUInt64(target->getDescription(0).c_str());
				LogDBG("picking: hit '%s', id=%lu",
				       target->getName().c_str(),
				       id);
				return id;
			}
		}
	}

	return 0;
}

osg::Camera& CltViewer::getCamera()
{
	return *mViewer->getCamera();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
