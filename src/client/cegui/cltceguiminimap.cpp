/*
 * cltceguiminimap.cpp
 * Copyright (C) 2005-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#include <osg/Drawable>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Projection>

#include <CEGUISystem.h>
#include <CEGUIWindowManager.h>

#include "client/cltviewer.h"
#include "cltceguimgr.h"

#include "cltceguiminimap.h"



/** Class to draw the minimap.
 */
class CltMinimapDrawable : public osg::Drawable
{
public:
	/** Default constructor */
	CltMinimapDrawable() {
		// basic setup of this drawable
		setSupportsDisplayList(false);
		setUseDisplayList(false);

		// projection: occupying the full screen
		mTransform = new osg::MatrixTransform(osg::Matrix::identity());
		mTransform->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		mProjection = new osg::Projection(osg::Matrix::ortho2D(650, 800, 0, 150));
		mProjection->addChild(mTransform);

		// setting the rendering properties of the node
		osg::Geode* geode = new osg::Geode();
		mTransform->addChild(geode);
		geode->addDrawable(this);
		osg::StateSet* stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
		stateset->setRenderBinDetails(11, "RenderBin");

		/** \todo mafm: disabling it ATM, it's WIP
		osg::Image* image = osgDB::readImageFile("Images/land_shallow_topo_2048.jpg");
		if (image) {
			osg::Texture2D* texture = new osg::Texture2D;
			texture->setImage(image);
			texture->setMaxAnisotropy(8);
			stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
		}

		osg::Material* material = new osg::Material;
		stateset->setAttribute(material);
		*/

		// setup a callback, to render frame
		// setEventCallback(new CltCEGUIEventHandler());
	}

	/** osg::Drawable interface */
	virtual void drawImplementation(osg::RenderInfo& renderInfo) const {
		// tell the UI to update and to render

		/** \todo mafm: complete
		renderInfo.getState()->setActiveTextureUnit(0);
		CEGUI::System::getSingleton().renderGUI();
		*/
	}
	/** osg::Drawable interface */
	CltMinimapDrawable(const CltMinimapDrawable& drawable, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) :
		osg::Drawable(drawable, copyop)
		{ }
	/** osg::Drawable interface */
	virtual osg::Object* cloneType() const {
		return new CltMinimapDrawable();
	}
	/** osg::Drawable interface */
	virtual osg::Object* clone(const osg::CopyOp& copyop) const {
		return new CltMinimapDrawable(*this, copyop);
	}
	/** Get node (to be added to the scene) */
	osg::Node* getNode() const {
		return mProjection;
	}

private:
	/// Projection in the viewport
	osg::Projection* mProjection;
	/// Matrix transformation for the drawable
	osg::MatrixTransform* mTransform;
};


//----------------------- CltCEGUIMinimap ----------------------------
template <> CltCEGUIMinimap* Singleton<CltCEGUIMinimap>::INSTANCE = 0;

CltCEGUIMinimap::CltCEGUIMinimap()
{
	mZoomLevel = 2;
	mLastX = -1;
	mLastY = -1;
	mMinimapDrawable = new CltMinimapDrawable();
	CltViewer::instance().addToScene(mMinimapDrawable->getNode());
}

CltCEGUIMinimap::~CltCEGUIMinimap()
{
}

void CltCEGUIMinimap::setMapImage(const char* areaName)
{
/** \todo mafm: adapt

	string imageName = string(areaName) + "_minimap";
	// load the pixmap of the minimap to be drawn
	iTextureWrapper* minimapTex =
		Client->GetEngine()->GetTextureList()->FindByName(imageName.c_str());
	if (!minimapTex) {
		LogERR("The engine hasn't loaded the image for the map: '%s'",
		       imageName.c_str());
		return;
	}
	minimapTex->SetFlags(CS_TEXTURE_2D);
	iTextureHandle* minimapHTex = minimapTex->GetTextureHandle();
	PERM_ASSERT (minimapHTex);
	minimap = Client->GetMinimap();
	minimap->SetTextureHandle(minimapHTex);
*/
}

void CltCEGUIMinimap::setZoomLevel(int level)
{
	if (level != 1 || level != 2 || level != 4 || level != 8) {
		// LogWRN("Zoom level is not 1, 2, 4 or 8; refusing to change it");
	} else {
		mZoomLevel = level;
		updateMinimap(mLastX, mLastY);
	}
}

void CltCEGUIMinimap::increaseZoomLevel()
{
	if (mZoomLevel > 7) {
		// LogWRN("zoom level > 7, refusing to increase");
	} else {
		mZoomLevel *= 2;
		updateMinimap(mLastX, mLastY);
	}
}

void CltCEGUIMinimap::decreaseZoomLevel()
{
	if (mZoomLevel < 2) {
		// LogWRN("zoom level < 2, refusing to decrease");
	} else {
		mZoomLevel /= 2;
		updateMinimap(mLastX, mLastY);
	}
}

void CltCEGUIMinimap::updateMinimap(int x, int z)
{
/** \todo mafm: adapt

	mLastX = x;

	// Y of the image is CS' Z coordinate, but upside down
	mLastY = z;
	int y = 1024 - z;

	// zone to use, depending on the zoom
	int sideLength = 512 / mZoomLevel;
	int topLeftCornerX = x - sideLength/2;
	int topLeftCornerY = y - sideLength/2;
	minimap->SetTextureRectangle(topLeftCornerX, topLeftCornerY,
				     sideLength, sideLength);

	// put over the decoration again
	CEGUI::StaticImage* decorImg = static_cast<CEGUI::StaticImage*>
		(CEGUI::WindowManager::getSingleton().getWindow("InGame/CompassDecoration"));
	PERM_ASSERT(decorImg);
	decorImg->setImage("InterfaceDecorations", "MapCompassCase");

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
