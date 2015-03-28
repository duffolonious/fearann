/*
 * srvquestmgr.h
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

/** Quest manager - basic concept.
 * The purpose of the quest manager is to handle messages from players about 
 * quest completion.
 * NOTE: this may never actually be implemented.
 * 
 * Simple examples:
 * 1. Deliver an item to an NPC.
 * 2. Kill a certain creature.
 *
 * How this will probably work is to have quests on both the client and the
 * server.  So they each have a list of quests.  Then when the client completes
 * a quest it sends a message to the server saying quest # completed.  The
 * server then checks if the request has been completed, and if so, gives the
 * character experience points or a reward.
 *
 * Where to start, probably simply with movement.  I say movement, because there
 * can be multiple stages to the quest to help people.  The quest can first
 * direct a player to a general area, a town, a pond, and then when they get
 * there, tell them that there quest target is nearby.
 */


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
