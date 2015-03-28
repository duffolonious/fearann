/*
 * srvworldmgr.h
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

#ifndef __FEARANN_SERVER_WORLD_MGR_H__
#define __FEARANN_SERVER_WORLD_MGR_H__


#include "common/patterns/singleton.h"
#include "common/datatypes.h"

#include <vector>


class LoginData;
class SrvNetworkMgr;
class SrvEntityBase;
class SrvEntityObject;
class SrvEntityCreature;
class SrvEntityPlayer;


/** Class governing the world events.
 *
 * This class governs the world and manages the things composing it (entities
 * which make part of the world), and their relationships (subscribing players
 * to entities), and so on.
 *
 * @author mafm
 */
class SrvWorldMgr : public Singleton<SrvWorldMgr>
{
public:
	/** Finalize, do whatever cleanup needed when the server shuts down. */
	void finalize();

	/** Returns whether the area is loaded or not */
	bool isAreaLoaded(const std::string& name) const;
	/** Load a new area, returns whether the operation is succesful or
	 * not. */
	bool loadArea(const std::string& name);

	/** Load objects from the DB (should be done typically when starting the
	 * game, after loading areas), returns whether the operation is
	 * succesful or not. */
	bool loadObjectsFromDB(const std::string& area);

	/** Load creatures from the DB done after loading areas.
	 */
	bool loadCreaturesFromDB(const std::string& area);

	/** Add a new player to the world */
	void addPlayer(LoginData* loginData);
	/** Remove a player */
	void removePlayer(LoginData* loginData);
	/** Returns the list of players within a certain radius from a given
	 * player (useful in example to distribute chat messages) */
	void getNearbyPlayers(const LoginData* player,
			      float radius,
			      std::vector<LoginData*>& nearbyPlayers) const;

	/** Player wants to get an entity from the world to the inventory */
	void playerGetItem(SrvEntityPlayer* player, EntityID entityID);
	/** Player wants to drop an entity from the inventory */
	void playerDropItem(SrvEntityPlayer* player, EntityID entityID);
	/** Change the owner of the object (useful for trading, in example) */
	bool changeObjectOwner(EntityID entityID, const std::string& charname);

private:
	/** Singleton friend access */
	friend class Singleton<SrvWorldMgr>;


	/// List of areas
	std::vector<std::string> mAreaList;
	/// List of players connected
	std::vector<SrvEntityPlayer*> mPlayerList;
	/// List of creatures
	std::vector<SrvEntityCreature*> mCreatureList;
	/// List of objects
	std::vector<SrvEntityObject*> mObjectList;


	/** Default constructor */
	SrvWorldMgr();

	/** Find player by login data
	 *
	 * \returns 0 if not found
	 */
	SrvEntityPlayer* findPlayer(const LoginData* loginData) const;
	/** Find a creature by id
	 *
	 * \returns 0 if not found
	 */
	SrvEntityCreature* findCreature(EntityID entityID) const;
	/** Find an object
	 *
	 * \returns 0 if not found
	 */
	SrvEntityObject* findObject(EntityID entityID) const;

	/** Add an entity to the game (other than players). */
	void addEntity(SrvEntityBase* entity);
	/** Remove an entity */
	void removeEntity(SrvEntityBase* entity);

	/** Get an object to an inventory
	 *
	 * \returns whether the operation was successful
	 */
	bool getObjectToInventory(EntityID entityID, const std::string& charname);
	/** Drop an object from an inventory
	 *
	 * \returns whether the operation was successful
	 */
	bool dropObjectFromInventory(EntityID entityID,
				     const::string& area,
				     const Vector3& position);
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
