/*
 * cltnetmgr.h
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

#ifndef __FEARANN_CLIENT_NET_MGR_H__
#define __FEARANN_CLIENT_NET_MGR_H__


#include "common/patterns/singleton.h"
#include "common/net/netlayer.h"
#include "common/net/msgbase.h"


/** Network manager for the client, abstracting all the operations.  The rest of
 * the application shouldn't worry about this, except for connecting and sending
 * messages, and very little else.
 *
 * This class is also responsible for calling the appropriate modules on the
 * client, when messages requiring client's attention are received.  The client
 * has to call this manager every frame or with some other frequency, so it can
 * process the incoming data.
 */
class CltNetworkMgr : public Singleton<CltNetworkMgr>
{
public:
	/** Returns whether we are still connected to server. */
	bool isConnected() const;

	/** Add server to ping */
	bool addServerToPing(const char* host, int port);

	/** Connect to server */
	bool connectToServer(const char* host, int port);

	/** Disconnect from server */
	void disconnect();

	/** Send a message to server */
	void sendToServer(MsgBase& msg);

	/** Process incoming messages, called from the main app every frame or
	 * at least with some regularity */
	void processIncomingMsgs();

private:
	/** Singleton friend access */
	friend class Singleton<CltNetworkMgr>;

	/// Connection object (with data about us and the peer, the server)
	Netlink mNetlink;

	/// Network layer
	SocketLayer mSocketLayer;

	/// The message and handler factory
	MsgHdlFactory mMsgHdlFactory;


	/** Default constructor */
	CltNetworkMgr();
	/** Default desstructor */
	~CltNetworkMgr();

	/** Register message handlers, called once to set it up */
	void registerMsgHdls();
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
