/*
 * srventitybase.cpp
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

#include "srventitybase.h"

#include "common/net/msgs.h"

#include "server/db/srvdbmgr.h"
#include "server/net/srvnetworkmgr.h"
#include "srventityplayer.h"

#include <osg/Matrix>
#include <osg/Vec3>


/*******************************************************************************
 * SrvEntityBaseMovable
 ******************************************************************************/
SrvEntityBaseMovable::SrvEntityBaseMovable(const MsgEntityMove& mov) :
	mMov(mov)
{
}

SrvEntityBaseMovable::~SrvEntityBaseMovable()
{
}

void SrvEntityBaseMovable::recalculatePosition(uint32_t ms)
{
	/** \todo mafm: check that matches previous behaviour

	Vector3 current;
	current = mMov.position + (mMov.direction / static_cast<float>(ms));
	LogDBG("Position recalculation (%ums):\n"
	       "Original:\t (%.1f, %.1f, %.1f)\n"
	       "Current:\t (%.1f, %.1f, %.1f)",
	       ms,
	       mMov.position.x, mMov.position.y, mMov.position.z,
	       current.x, current.y, current.z);
	mMov.position = current;
	*/

	Vector3 current;
	current = mMov.position + (mMov.direction / static_cast<float>(ms));
	LogDBG("Position recalculation (%ums):\n"
	       "  - Original:\t (%.1f, %.1f, %.1f)\n"
	       "  - Current:\t (%.1f, %.1f, %.1f)",
	       ms,
	       mMov.position.x, mMov.position.y, mMov.position.z,
	       current.x, current.y, current.z);
	mMov.position = current;
}

void SrvEntityBaseMovable::getPosition(Vector3& position) const
{
	position = mMov.position;
}

void SrvEntityBaseMovable::getPositionWithRelativeOffset(Vector3& position,
							 const Vector3& offset) const
{
	/** \todo mafm: check that matches previous behaviour

	csReversibleTransform transf;

	// set the origin, which is the position of the movable entity
	transf.SetOrigin(mMov.position);
	
	// rotate around the "up" vector, so the offset is oriented to
	// where the entity looks at
	Vector3 up(0.0f, 1.0f, 0.0f);
	transf.RotateThis(up, mMov.yrot);

	// now really calculate the offset
	position = transf.This2Other(offset);

	*/

	osg::Matrix transf;
	transf.setTrans(mMov.position.x, mMov.position.y, mMov.position.z);
	transf.makeRotate(mMov.rot, osg::Z_AXIS);
	transf.makeTranslate(offset.x, offset.y, offset.z);

	osg::Vec3 pos = transf.getTrans();
	position.x = pos.x();
	position.y = pos.y();
	position.z = pos.z();

	LogDBG("PosWithOffset: Pos (%.1f, %.1f, %.1f),"
	       " offset (%.1f, %.1f, %.1f), "
	       " result = (%.1f, %.1f, %.1f)",
	       mMov.position.x, mMov.position.y, mMov.position.z,
	       offset.x, offset.y, offset.z,
	       position.x, position.y, position.z);
}

float SrvEntityBaseMovable::getDistanceToEntity(const SrvEntityBaseMovable& other) const
{
	Vector3 otherPos;
	other.getPosition(otherPos);

	float distance = mMov.position.distance(otherPos);

	LogDBG("Distance (%.1f, %.1f, %.1f) to (%.1f, %.1f, %.1f) = %0.1f",
	       mMov.position.x, mMov.position.y, mMov.position.z,
	       otherPos.x, otherPos.y, otherPos.z,
	       distance);

	return distance;
}

const char* SrvEntityBaseMovable::getArea() const
{
	return mMov.area.c_str();
}

void SrvEntityBaseMovable::setMovementData(const MsgEntityMove& mov)
{
	mMov = mov;
}


/*******************************************************************************
 * SrvEntityBaseObservable
 ******************************************************************************/
SrvEntityBaseObservable::SrvEntityBaseObservable(const MsgEntityCreate& basic,
						 const MsgEntityMove& mov) :
	SrvEntityBaseMovable(mov), mBasic(basic)
{
}

void SrvEntityBaseObservable::onAttachObserver(Observer* observer)
{
	LogDBG("Subscribing observer '%s' to entity: name='%s', id='%llu',"
	       " pos(%.1f, %.1f, %.1f)",
	       observer->_name.c_str(), mBasic.entityName.c_str(), mBasic.entityID,
	       mMov.position.x, mMov.position.y, mMov.position.z);

	mBasic.position = mMov.position;
	mBasic.area = mMov.area;

	MsgEntityCreate msg = mBasic;
	SrvEntityBaseObserverEvent event(SrvEntityBaseObserverEvent::ENTITY_CREATE, msg);
	observer->updateFromObservable(event);
}

void SrvEntityBaseObservable::onDetachObserver(Observer* observer)
{
	LogDBG("Unsubscribing observer '%s' from entity: name='%s', id='%llu'",
	       observer->_name.c_str(), mBasic.entityName.c_str(), mBasic.entityID);

	MsgEntityDestroy msg;
	msg.entityID= mBasic.entityID;
	SrvEntityBaseObserverEvent event(SrvEntityBaseObserverEvent::ENTITY_DESTROY, msg);
	observer->updateFromObservable(event);
}


/*******************************************************************************
 * SrvEntityBase
 ******************************************************************************/
SrvEntityBase::SrvEntityBase(const MsgEntityCreate& basic,
			     const MsgEntityMove& mov) :
	SrvEntityBaseObservable(basic, mov)
{
	mMeshFactory = mBasic.meshType + "_" + mBasic.meshSubtype;
}

void SrvEntityBase::saveToDB()
{
	LogDBG("SrvEntityBase::saveToDB()");

	// mafm: sometimes works bad with the "exact" data, +=0.5 for pos3=z
	// (height)
	string id = StrFmt("%llu", mBasic.entityID);
	string area = mMov.area;
	string pos1 = StrFmt("%.1f", mMov.position.x);
	string pos2 = StrFmt("%.1f", mMov.position.y);
	string pos3 = StrFmt("%.1f", mMov.position.z + 0.5f);
	string rot = StrFmt("%.3f", mMov.rot);

	SrvDBQuery query;
	query.setTables("entities");
	query.setCondition("id='" + id + "'");
	query.addColumnWithValue("area", area);
	query.addColumnWithValue("pos1", pos1);
	query.addColumnWithValue("pos2", pos2);
	query.addColumnWithValue("pos3", pos3);
	query.addColumnWithValue("rot", rot);
	bool success = SrvDBMgr::instance().queryUpdate(&query);
	if (!success) {
		LogERR("Error while saving position for entity '%s'",
		       id.c_str());
		return;
	}

	LogDBG("Saving position for entity '%s' in DB: '%s' (%s, %s, %s) rot=%s",
	       id.c_str(), mMov.area.c_str(),
	       pos1.c_str(), pos2.c_str(), pos3.c_str(), rot.c_str());
}

const char* SrvEntityBase::getName() const
{
	return mBasic.entityName.c_str();
}

EntityID SrvEntityBase::getID() const
{
	return mBasic.entityID;
}

const char* SrvEntityBase::getType() const
{
	return mBasic.meshType.c_str();
}

const char* SrvEntityBase::getSubtype() const
{
	return mBasic.meshSubtype.c_str();
}

const char* SrvEntityBase::getMeshFactory() const
{
	return mMeshFactory.c_str();
}

const char* SrvEntityBase::getEntityClass() const
{
	return mBasic.entityClass.c_str();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
