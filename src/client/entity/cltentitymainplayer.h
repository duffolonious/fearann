/*
 * cltentitymainplayer.h
 * Copyright (C) 2006 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#ifndef __FEARANN_CLIENT_ENTITY_MAINPLAYER_H__
#define __FEARANN_CLIENT_ENTITY_MAINPLAYER_H__


#include "cltentityplayer.h"

#include <vector>

class InventoryItem;


/** Controls the main player entity, which handles most of the game logic.  Note
 * that it's convenient to have a Singleton for many reasons, among other to
 * have only one instance, and not having to rely on external modules to hold a
 * pointer to it (so any class can access the main player with ::instance()).
 *
 * \remark This is a problem when initializing the object, because it's
 * convenient for the rest of entities to include the EntityCreate msg to
 * initialize the base class; but we can't do that with the Singleton.  Thus,
 * the solution is force to call a createEntity() before accessing instance(),
 * which is only a minor inconvenience and a minor breakage of the pattern
 * access.
 *
 * \remark mafm This doesn't use the Singleton template of the rest of the
 * classes because of the aforementioned issues.
 *
 * \todo mafm Shouln't be using Singleton after all?  Or at least differentiate
 * better with different accessors or something?
 */
class CltEntityMainPlayer : public CltEntityPlayer
{
public: 
	/** Singleton instance. */
	static CltEntityMainPlayer& instance();
	/** Checking if initialized. */
	static bool isInitialized();

	/** Create entity.  It has to be called initially before any
	 * ::instance() call, read the comment in the class description. */
	static CltEntityMainPlayer* createEntity(const MsgEntityCreate* entityBasicData);

	/** Set the player statistics */
	void setPlayerData(MsgPlayerData* playerData);

	/** Ask the server to pickup an item from the world to the */
	void pickup(uint32_t itemID);
	/** Ask the server to drop an item from the inventory */
	void drop(uint32_t itemID);
	/** Add an object to the inventory (after server confirmation) */
	void addToInventory(InventoryItem* item);
	/** Remove an object from the inventory (after server confirmation) */
	void removeFromInventory(uint32_t itemID);

	/** get distance to entity */
	float getDistanceToEntity( uint32_t id ) const;

private:
	/// Singleton instance
	static CltEntityMainPlayer* INSTANCE;

	/// The player data (statistics, abilities and so on)
	MsgPlayerData mPlayerData;


	/** Default constructor */
	CltEntityMainPlayer(const MsgEntityCreate* entityBasicData);
	/** Destructor */
	~CltEntityMainPlayer();
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
