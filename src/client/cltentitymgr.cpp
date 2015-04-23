/*
 * cltentitymgr.cpp
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

#include <osg/Vec3>
#include <osg/MatrixTransform>
#include <osgCal/Model>

#include "common/net/msgs.h"

#include "client/entity/cltentitybase.h"
#include "client/entity/cltentitymainplayer.h"
#include "client/entity/cltentityplayer.h"
#include "client/entity/cltentitycreature.h"
#include "client/entity/cltentityobject.h"
#include "client/content/cltcontentloader.h"
#include "cltcamera.h"
#include "cltinput.h"
#include "cltviewer.h"

#include "cltentitymgr.h"


//-------------------------- CltEntityMgr -------------------------
template <> CltEntityMgr* Singleton<CltEntityMgr>::INSTANCE = 0;

CltEntityMgr::CltEntityMgr()
{
}

CltEntityMgr::~CltEntityMgr()
{
}

void CltEntityMgr::entityCreate(const MsgEntityCreate* msg)
{
	LogNTC("New entity: id: %lu, name: %s, class %s, mesh: %s %s, pos: %.1f %.1f %.1f",
	       msg->entityID, msg->entityName.c_str(),
	       msg->entityClass.c_str(),
	       msg->meshType.c_str(), msg->meshSubtype.c_str(),
	       msg->position.x, msg->position.y, msg->position.z);

	// creating behaviour depending on the entity type
	if (msg->entityClass == "MainPlayer") {
		// initialize the main player class before calling any method
		// through the singleton
		CltEntityBase* entity = CltEntityMainPlayer::createEntity(msg);

		// load the area where the main player wanders in, and add the
		// player transform too
		CltViewer::instance().loadScene(msg->area);

		// set position in the class controlling the player movement
		osg::Vec3 position(msg->position.x, msg->position.y, msg->position.z);
		CltMainPlayerManipulator::instance().setPosition(position, msg->rot);

		// set the camera having as target the main player transform
		osg::MatrixTransform* mainPlayerTransform = CltMainPlayerManipulator::instance().getMatrixTransform();
		CltCameraMgr::instance().setTargetTransform(mainPlayerTransform);

		// add to the list
		PERM_ASSERT(mEntityList.find(msg->entityID) == mEntityList.end());
		mEntityList[msg->entityID] = entity;

	} else if (msg->entityClass == "Player") {
		// create entity
		CltEntityPlayer* entity = new CltEntityPlayer(msg);

		// load model and add to scene
		osg::Node* playerNode = entity->loadModel();
		playerNode->getOrCreateStateSet()->setMode(GL_LIGHTING,
							   osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		entity->getTransform()->addChild(playerNode);
		playerNode->addDescription(StrFmt("%lu", msg->entityID));
		//LogDBG("%s desc: '%s'", entity->getName(), entity->getTransform()->getDescription( 0 ).c_str() );
		CltViewer::instance().addToScene(entity->getTransform());

		///\todo: duffolonious: pump the player so it stays in place.
		MsgEntityMove movemsg;
		movemsg.position = msg->position;
		movemsg.run = movemsg.mov_fwd = movemsg.mov_bwd = false;
		movemsg.rot = msg->rot; //0.0f;
		movemsg.rot_left = movemsg.rot_right = false;
		entity->setMovementProperties(&movemsg);

		// add to the list
		PERM_ASSERT(mEntityList.find(msg->entityID) == mEntityList.end());
		mEntityList[msg->entityID] = entity;

		LogNTC("Player '%s' added successfully", entity->getName());

	} else if (msg->entityClass == "Creature") {
		// create entity
		CltEntityCreature* entity = new CltEntityCreature(msg);

		// load model and add to scene
		osg::Node* creatureNode = entity->loadModel();
		creatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING,
							   osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		entity->getTransform()->addChild(creatureNode);
		creatureNode->addDescription(StrFmt("%lu", msg->entityID));
		LogDBG("%s desc: '%s'", entity->getName(),
		       entity->getTransform()->getDescription(0).c_str());
		CltViewer::instance().addToScene(entity->getTransform());

		// add to the list
		///\todo: duffolonious: fix collisions with other entities with
		///the same id.
		PERM_ASSERT(mEntityList.find(msg->entityID) == mEntityList.end());
		mEntityList[msg->entityID] = entity;

		///\todo: duffolonious: pump the creature so it stays in place.
		MsgEntityMove movemsg;
		movemsg.position = msg->position;
		movemsg.run = movemsg.mov_fwd = movemsg.mov_bwd = false;
		movemsg.rot = msg->rot; //0.0f;
		movemsg.rot_left = movemsg.rot_right = false;
		entity->setMovementProperties(&movemsg);

		LogNTC("Creature '%s' added successfully", entity->getName());
	} else if (msg->entityClass == "Object") {
		// create entity
		CltEntityObject* entity = new CltEntityObject(msg);

		// load model and add to scene
		osg::Node* node = entity->loadModel();
		entity->getTransform()->addChild(node);
		node->addDescription(StrFmt("%lu", msg->entityID));
		LogDBG("%s desc: '%s' - type '%s'",
		       entity->getName(),
		       entity->getTransform()->getDescription(0).c_str(),
		       entity->getTransform()->className());
		CltViewer::instance().addToScene(entity->getTransform());

		// add to the list
		///\todo: duffolonious: fix collisions with other entities with
		///the same id.
		PERM_ASSERT(mEntityList.find(msg->entityID) == mEntityList.end());
		mEntityList[msg->entityID] = entity;

		///\todo: duffolonious: pump the creature so it stays in place.
		MsgEntityMove movemsg;
		movemsg.position = msg->position;
		//movemsg.rot = 0.0f;
		entity->setMovementProperties(&movemsg);

		LogNTC("Object '%s' added successfully", entity->getName());
	} else {
		LogERR("Entity class not recognized: '%s'",
		       msg->entityClass.c_str());
	}
}

void CltEntityMgr::entityMove(const MsgEntityMove* msg)
{
	LogDBG("Received EntityMove msg for entity (id: %lu)", msg->entityID);

	// checking autenticity
	if (mEntityList.find(msg->entityID) == mEntityList.end()) {
		LogERR("Received entity move notification but entity unknown! (id: %lu)",
		       msg->entityID);
	} else {
		LogDBG("Moving entity id '%lu'", msg->entityID);
		CltEntityBase* entity = mEntityList.find(msg->entityID)->second;

		if (string("MainPlayer") == entity->className()) {
			LogWRN("Got a message requesting to change the position of the main player "
			       "(it should happen only when resetting the position for some reason)");
			osg::Vec3 position(msg->position.x, msg->position.y, msg->position.z);
			CltMainPlayerManipulator::instance().setPosition(position, msg->rot);
		} else {
			entity->setMovementProperties(msg);
		}
	}
}

void CltEntityMgr::entityDestroy(const MsgEntityDestroy* msg)
{
	map<uint64_t, CltEntityBase*>::iterator it = mEntityList.find(msg->entityID);

	if (it == mEntityList.end()) {
		LogERR("Received entity destruction notification but entity unknown! (id: %lu)",
		       msg->entityID);
	} else {
		LogDBG("Destroying entity id '%lu'", msg->entityID);

		// destroying entity entity
		CltEntityBase* entity = it->second;
		if (string("MainPlayer") == entity->className()) {
			LogWRN("Refusing to destroy main player entity!!!");
		} else {
			if (string("Player") == entity->className()) {
				delete dynamic_cast<CltEntityPlayer*>(entity);
			}
			if (string("Creature") == entity->className()) {
				delete dynamic_cast<CltEntityCreature*>(entity);
			}
		}

		// removing model from to scene
		CltViewer::instance().removeFromScene(entity->getTransform());

		// removing from list
		mEntityList.erase(it);

		//LogNTC("%s '%s' removed successfully", entity->className(), entity->getName());
	}
}

void CltEntityMgr::updateTransforms(double elapsedSeconds)
{
	for (map<uint64_t, CltEntityBase*>::iterator it = mEntityList.begin(); it != mEntityList.end(); ++it) {
		CltEntityBase* entity = it->second;
		if (string("Player") == entity->className()) {
			dynamic_cast<CltEntityPlayer*>(entity)->updateTransform(elapsedSeconds);
		}
		if (string("Creature") == entity->className()) {
			dynamic_cast<CltEntityCreature*>(entity)->updateTransform(elapsedSeconds);
		}
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
