/*
 * cltentitybase.cpp
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

#include <cstdlib>

#include <osg/BoundingBox>
#include <osg/MatrixTransform>
#include <osg/Node>

#include "cltentitybase.h"


//----------------------- CltEntityBase ----------------------------
CltEntityBase::CltEntityBase(const MsgEntityCreate* entityBasicData, const char* _className) :
	mClassName(_className), mEntityBasicData(*entityBasicData),
	mNode(new osg::Node()), mTransform(new osg::MatrixTransform()), mBoundingBox(new osg::BoundingBox())
{
	mTransform->addChild(mNode);
}

CltEntityBase::~CltEntityBase()
{
}

const char* CltEntityBase::className() const
{
	return mClassName.c_str();
}

const char* CltEntityBase::getName() const
{
	return mEntityBasicData.entityName.c_str();
}

uint64_t CltEntityBase::getID() const
{
	return mEntityBasicData.entityID;
}

osg::MatrixTransform* CltEntityBase::getTransform() const
{
	return mTransform;
}

const osg::BoundingBox* CltEntityBase::getBoundingBox() const
{
	return mBoundingBox;
}

void CltEntityBase::setMovementProperties(const MsgEntityMove* msg)
{
	mEntityMovData = *msg;
}

const MsgEntityMove* CltEntityBase::getMovementProperties() const
{
	return &mEntityMovData;
}

void CltEntityBase::updateTransform(double elapsedSeconds)
{
	/** \todo mafm: we shouldn't need this, fix it. */
	abort();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
