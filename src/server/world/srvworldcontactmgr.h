/*
 * srvworldcontactmgr.h
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

#ifndef __FEARANN_SERVER_WORLD_CONTACT_MGR_H__
#define __FEARANN_SERVER_WORLD_CONTACT_MGR_H__


class LoginData;


/** Manages everything about contact lists.
 *
 * Contact lists are kind of list of "friends" in instant messaging tools.  Note
 * that the methods have to be declared as static to avoid to create an object
 * for them.
 *
 * @author mafm
 */
class SrvWorldContactMgr
{
public:
	/** This function does all needed when the player status changes (logs
	 * in, logs out):
	 *
	 * 1) Send the newly connected player the status info about contact list
	 * (who in the contact list is connected); and
	 *
	 * 2) and notify the status to other players on-line interested in this
	 * player
	 */
	static void playerStatusChange(const LoginData* player,
				       bool connected);

	/** Add a contact by request of the player */
	static void addContact(const LoginData* player, 
			       std::string& otherCharname,
			       char otherTypeChar,
			       std::string& otherComment);

	/** Remove a contact by request of the player */
	static void removeContact(const LoginData* player, 
				  std::string& otherCharname);

private:

	/** Helper method to check whether a player is connected */
	static bool isContactPlaying(const std::string& name);

	/** Helper method to send messages via console */
	static void sendConsoleReply(const LoginData* loginData,
				     const std::string& msg);

	/** Helper method to notify the player being connected about one
	 * specific contact */
	static void notifyPlayerOfOneContact(const LoginData* player,
					     const std::string& contactName,
					     char contactType,
					     const std::string& contactComment,
					     const std::string& contactLastLogin,
					     bool connected);

	/** Helper method to notify the player being connected about his/her
	 * contacts */
	static void notifyPlayerOfContacts(const LoginData* player);

	/** Helper method to notify the other players about the player being
	 * connected */
	static void notifyOtherPlayers(const LoginData* player,
				       bool connected);
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
