/*
 * cltentityobject.cpp
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

#include "config.h"
#include "client/cltconfig.h"

#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include "common/net/msgs.h"

#include "client/cltcamera.h"
#include "client/cegui/cltceguiinventory.h"
#include "client/content/cltcontentloader.h"
#include "client/net/cltnetmgr.h"
#include "client/cltviewer.h"

#include "cltentityobject.h"


//----------------------- CltEntityObject ----------------------------
CltEntityObject::CltEntityObject(const MsgEntityCreate* entityBasicData) :
	CltEntityBase(entityBasicData, "Object"), mModel(0)
{
	// bounding box for players (maybe we need to tweak it for every race,
	// but it should be OK at least for testing)
	mBoundingBox->set(-0.33f, -0.33f, 0.0f,
			  0.33f, 0.33f, 1.75f);
}

CltEntityObject::~CltEntityObject()
{
}

osg::Node* CltEntityObject::loadModel()
{
	// load model with the help of the content loader
	mModel = CltContentLoader::instance().loadModel(mEntityBasicData.meshType,
										       mEntityBasicData.meshSubtype);
	if (!mModel) {
		LogERR("Loading osg model: %s", "error");
		return 0;
	}

	// set the node name as player name
	mModel->setName(mEntityBasicData.entityName);

	return mModel;
}

void CltEntityObject::setMovementProperties(const MsgEntityMove* entityMovData)
{
	mEntityMovData = *entityMovData;

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
	if ( mTransform->getNumDescriptions() > 0 )
	{
		LogDBG("Transform tag: %s", mTransform->getDescription( 0 ).c_str() );
	}

	/*
	LogNTC("Position of object '%s' changed to: (%.1f, %.1f, %.1f), rot=%.1f",
	       getName(), p.x(), p.y(), p.z(), mEntityMovData.rot);
	*/
}


void CltEntityObject::updateTransform(double elapsedSeconds)
{
/*
	osg::Vec3 p = mTransform->getMatrix().getTrans();
	LogDBG("Object '%s' position internal update: (%.1f, %.1f, %.1f)",
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
