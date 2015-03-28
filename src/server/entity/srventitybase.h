/*
 * srventitybase.h
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

#ifndef __FEARANN_SERVER_ENTITY_BASE_H__
#define __FEARANN_SERVER_ENTITY_BASE_H__


#include "common/net/msgs.h"
#include "common/patterns/observer.h"

#include <string>
#include <vector>


class SrvEntityPlayer;


/** Represents and operates with the movement of entities.
 *
 * @author mafm
 */
class SrvEntityBaseMovable
{
	friend class SrvEntityBase;

public:
	/** Recalculate position (this should be called periodically from the
	 * server, telling the number of milliseconds since the last update) */
	void recalculatePosition(uint32_t ms);
	/** Get the position */
	void getPosition(Vector3& position) const;
	/** Get the position with offset */
	void getPositionWithRelativeOffset(Vector3& position, const Vector3& offset) const;
	/** Return the distance of this entity to the one passed */
	float getDistanceToEntity(const SrvEntityBaseMovable& other) const;
	/** Get the area where this player is currently in */
	const char* getArea() const;

protected:
	/// Data needed for movement
	MsgEntityMove mMov;

	/** Constructor
	 *
	 * @param mov Movement message, representing all data needed for this
	 * entity to operate
	 */
	SrvEntityBaseMovable(const MsgEntityMove& mov);
	/** Destructor */
	~SrvEntityBaseMovable();


	/** Set movement data (setting position alone is not useful, because we
	 * need to change the direction and so on) */
	void setMovementData(const MsgEntityMove& mov);
};


/** ObserverEvent for ServerEntityBase
 */
class SrvEntityBaseObserverEvent : public ObserverEvent
{
public:
	/** Action Identifier enumerator */
	enum ActionId { ENTITY_CREATE = 1, ENTITY_DESTROY };

	/** Action Identifier */
	const ActionId _actionId;

	/** Payload */
	MsgBase& _msg;

	/** Constructor */
	SrvEntityBaseObserverEvent(ActionId actionId, MsgBase& msg) :
		ObserverEvent("SrvEntityBaseObserverEvent"), _actionId(actionId), _msg(msg)
		{ }
};


/** Base class for a observable entity (objects, creatures, players...).
 *
 * Observers (players, in principle) can be added to the entity classes derived
 * from this, so the entity sends them notifications to create the entity,
 * remove it, and appropriete changes depending on position and state of the
 * entity.  It needs to be derived from Movable, because it's one of the main
 * points of this class: tell subscribers about our movements and actions.
 *
 * @author mafm
 */
class SrvEntityBaseObservable : public SrvEntityBaseMovable, public Observable
{
protected:
	/// Basic data about the entity
	MsgEntityCreate mBasic;

	/** Constructor
	 *
	 * @param basic Entity basic data message, basic data about the entity
	 * to be represented
	 *
	 * @param mov For the base movement class
	 */
	SrvEntityBaseObservable(const MsgEntityCreate& basic,
				const MsgEntityMove& mov);
	/** Destructor */
	virtual ~SrvEntityBaseObservable() { }

	/** @see Observable::onAttachObserver */
	virtual void onAttachObserver(Observer* observer);
	/** @see Observable::onDetachObserver */
	virtual void onDetachObserver(Observer* observer);
};


/** Base entity class in the server, all the rest of the classes (players,
 * creatures, objects, NPCs, ...) must be derived from this one.
 *
 * @author mafm
 */
class SrvEntityBase : public SrvEntityBaseObservable
{
public:
	/** Destructor */
	virtual ~SrvEntityBase() { }

	/** Get the name of the entity */
	const char* getName() const;
	/** Get the ID  of the entity */
	EntityID getID() const;
	/** Get the type (first part in the mesh factory, in example races) */
	const char* getType() const;
	/** Get the subtype (second part of the mesh factory, in example
	 * genders) */
	const char* getSubtype() const;
	/** Get the factory to create the mesh for this entity (combination of
	 * type+subtype) */
	const char* getMeshFactory() const;
	/** Get the entity class */
	const char* getEntityClass() const;

protected:
	/// The mesh factory (generated on the fly, but has to be a variable
	/// because we return a pointer)
	std::string mMeshFactory;


	/** Constructor
	 *
	 * @param basic For the base observable class
	 *
	 * @param mov For the base movement class
	 */
	SrvEntityBase(const MsgEntityCreate& basic,
		      const MsgEntityMove& mov);

	/** Virtual function which can be overriden for especial entities
	 * (players at least), to save the state of the entity to the DB */
	virtual void saveToDB();
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
