/*
 * botcombat.cpp
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

#include "config.h"

#include <cmath>
#include <ctime>
#include <cstdlib>

#include <vector>

#include "common/threads.h"

#include "bot/net/botnetmgr.h"
#include "bot/bot.h"
#include "botmove.h"

//----------------------- MoveMgr ----------------------------
template <> BotMoveMgr* Singleton<BotMoveMgr>::INSTANCE = 0;

BotMoveMgr::~BotMoveMgr()
{
}


void *halt(void *arg)
{
  struct timespec *req = (struct timespec *)arg;
  struct timespec rem;
  //FIXME: need to convert elapsedseconds to sleepable amount.
  //req->tv_sec = req->tv_nsec/999999999;
  //req->tv_nsec -= req->tv_sec * 999999999;
  LogDBG("lookAt: sec: %lu, nsec: %lu", req->tv_sec, req->tv_nsec);
  nanosleep(req, &rem);
  free(req);
  //after time elapsed, stop movement.
  MsgEntityMove move = BotMoveMgr::instance().getMove();
	LogNTC("halt: rot: %f, id: %lu",
		move.rot,
		move.entityID);

  move.rotSpeed = 0.0f;
  move.rot_left = false;
  move.rot_right = false;

  Bot->getNetworkMgr()->sendToServer(move);
  return (void *)NULL;
}

MsgEntity BotMoveMgr::findEntity(uint64_t otherId)
{
  std::vector<MsgEntity>::iterator it;
  for(it = entities.begin(); it != entities.end(); it++) {
    if((*it).create.entityID == otherId) {
      return (*it);
    }
  }
  return (MsgEntity());
}

void BotMoveMgr::lookAt(Vector3 position)
{
  float angle; //absolute angle
  float rel_angle; //angle relative to previous angle
  float tmp_rot;
  //dot product of unit vectors (normalize vectors first).
  Vector3 dir(0, -1, 0); //always use the 0 rotation unit vector
  Vector3 tmp_position = position - move.position;
  tmp_position.z = 0.0f; //only on the xy plane
  tmp_position.Normalize(); //convert to unit vector
  angle = acos(tmp_position * dir); //angle in radians
  tmp_rot = (tmp_position.x >= 0? angle:-angle);
  //need to know current direction to know whether rotation left or right.
  rel_angle = tmp_rot - move.rot;
  if(fabs(rel_angle) > PI_NUMBER) {
    rel_angle = -rel_angle;
  }
  if(rel_angle > 0) {
    move.rot_left = true;
    move.rot_right = false;
  } else if(rel_angle < 0) {
    move.rot_left = false;
    move.rot_right = true;
  }
  move.rot = tmp_rot;
  move.rotSpeed = (2*PI_NUMBER)/4.0f;
  MsgEntityMove tmp = move;
  Bot->getNetworkMgr()->sendToServer(tmp);

  /// create a timer (thread) to perform move action after time elapsed?
  /// perhaps a timer listener that sends a time update to all listening 
  /// classes.  The listening classes would act accordingly.
  double elapsedseconds = fabs(rel_angle)/move.rotSpeed;
  //void * tmp = (void *)elapsedseconds;
  struct timespec *req;
  if(!(req = (struct timespec *)malloc(sizeof(struct timespec)))) {
    throw "req malloc failed.";
    return;
  }
  req->tv_sec = (int)elapsedseconds;
  req->tv_nsec = (long int)((elapsedseconds - (float)req->tv_sec) * 1000000000);
#if 0
  LogDBG("lookAt: elapsedsecs: %f", elapsedseconds);
  LogDBG("lookAt: req.tv_sec: %d", req.tv_sec);
  LogDBG("lookAt: req.tv_nsec: %lu", req.tv_nsec);
#endif
  Thread callback((void*)&halt, (void *)req);
  callback.Detach();
}

bool BotMoveMgr::handleCreateMsg(MsgEntityCreate* msg)
{
  const char *name;
	LogNTC("entityName: %s, meshtype: %s\n"
			"pos -> x: %f, y: %f, z: %f, rot: %f, id: %lu",
			msg->entityName.c_str(), msg->meshType.c_str(),
			msg->position.x, msg->position.y, msg->position.z,
			msg->rot, msg->entityID);

  name = Bot->getName();
  if(!msg->entityName.compare(name)) {
    LogDBG("bot set: %s", msg->entityName.c_str());
    create = *msg;
    move.position.x = msg->position.x;
    move.position.y = msg->position.y;
    move.position.z = msg->position.z;
    move.rot = msg->rot;
  } else {
    //put into entity vector
    MsgEntity entity;
    entity.create = *msg;
    entities.push_back(entity);
  }

  return true;
}


bool BotMoveMgr::handleMoveMsg(MsgEntityMove* msg)
{
	LogNTC("pos -> x: %f, y: %f, z: %f, dir -> x: %f, y: %f, z: %f, rot: %f, id: %lu",
			msg->position.x, msg->position.y, msg->position.z,
			msg->direction.x, msg->direction.y, msg->direction.z,
      msg->rot,
			msg->entityID);

  if(msg->entityID == create.entityID) {
    move = *msg;
  } else {
    //set correct entity.
    findEntity(msg->entityID).move = *msg;
  }

  return true;
}


void BotMoveMgr::listMoveInfo()
{
  LogNTC("Move info:");
	LogNTC("pos -> x: %f, y: %f, z: %f, dir -> x: %f, y: %f, z: %f, rot: %f, id: %lu",
			move.position.x, move.position.y, move.position.z,
			move.direction.x, move.direction.y, move.direction.z,
			move.rot,
			move.entityID);

	return;
}

/**
 * This is a triangular boundary for the bot (3 Vec3's).
 *
 * Right now we don't care about the vertical (the Z axis).
 *
 * Also requires other checks (outZone - true if move point outside zone).
 */
void BotMoveMgr::setZone(Vector3 point1, Vector3 point2, Vector3 point3)
{
	//In the future this may change to multiple triangles (zones).
	zone.point[0] = point1;
	zone.point[1] = point2;
	zone.point[2] = point3;

  for(int i=0; i < 3; i++) {
    LogNTC("point%d: x: %3.2f, y: %3.2f, z: %3.2f", i, zone.point[i].x, zone.point[i].y, zone.point[i].z);
  }
  LogNTC("point in zone: %s", isInZone(move.position)? "True" : "False");
}

/**
 */
int BotMoveMgr::isInZone(Vector3 point1)
{
  Vector3 v0, v1, v2;
  Vector3 A, B, C, P;
  float dot00, dot01, dot02, dot11, dot12;
  float invDenom;
  float u, v;

  A = zone.point[0];
  B = zone.point[1];
  C = zone.point[2];
  P = point1;

  // Compute vectors        
  v0 = C - A;
  v1 = B - A;
  v2 = P - A;

  // Compute dot products
  dot00 = v0 * v0;
  dot01 = v0 * v1;
  dot02 = v0 * v2;
  dot11 = v1 * v1;
  dot12 = v1 * v2;

  // Compute barycentric coordinates
  invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
  u = (dot11 * dot02 - dot01 * dot12) * invDenom;
  v = (dot00 * dot12 - dot01 * dot02) * invDenom;

  // Check if point is in triangle
  LogDBG("u: %f, v: %f", u, v);
  return (u > 0) && (v > 0) && (u + v < 1);
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
