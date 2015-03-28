/*
 * botcombat.h
 * Copyright (C) 2005-2006 by Bryan Duff <duff0097@umn.edu>
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

#ifndef __FEARANN_BOT_ACTION_MOVE_H__
#define __FEARANN_BOT_ACTION_MOVE_H__


#include "common/patterns/singleton.h"
#include "common/net/msgs.h"

#include <string>


class Triangle
{
public:
	Vector3 point[3];
};


class MsgEntity
{
public:
	MsgEntityCreate create;
	MsgEntityMove move;
};


/** Class contains and manages objects
 */
class BotMoveMgr : public Singleton<BotMoveMgr>
{
public:
	MsgEntityMove getMove() const { return move; };

	void lookAt(Vector3 position);

	/// ... move action messages
	bool handleCreateMsg(MsgEntityCreate* msg); //first one after joining
	bool handleMoveMsg(MsgEntityMove* msg);

	/// List position, rotation, objects within a certain distance?
	void listMoveInfo();
	void setZone(Vector3 point1, Vector3 point2, Vector3 point3);
	int isInZone(Vector3 point1);

	uint64_t getBotID(void) { return id; };
	void setBotID(uint64_t _id) { id = _id; };

private:
	/** Singleton friend access */
	friend class Singleton<BotMoveMgr>;


	/// This bot's id.
	uint64_t id;
	/// Other's id's.

	Triangle zone;

	MsgEntityCreate create;
	MsgEntityMove move;

	//switch to map
	std::vector<MsgEntity> entities;

	MsgEntity findEntity(uint64_t id);

	//bool facingDirection;


	~BotMoveMgr();
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
