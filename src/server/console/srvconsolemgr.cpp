/*
 * srvconsolemgr.cpp
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

#include "config.h"

#include "srvconsolemgr.h"

#include "common/net/msgs.h"
#include "common/configmgr.h"

#include "server/login/srvloginmgr.h"
#include "server/net/srvnetworkmgr.h"
#include "server/world/srvworldmgr.h"
#include "server/console/srvcommand.h"

#include <cstdlib>


/*******************************************************************************
 * SrvConsoleMgr
 ******************************************************************************/
template <> SrvConsoleMgr* Singleton<SrvConsoleMgr>::INSTANCE = 0;

SrvConsoleMgr::SrvConsoleMgr()
{
	mRadiusSay = atof(ConfigMgr::instance().getConfigVar("Server.Chat.SayRadius", "0"));
	if (mRadiusSay == 0.0f) {
		LogERR("Couldn't get ChatSayRadius from the config file");
	}
}

void SrvConsoleMgr::processChat(const MsgChat* msg, const LoginData* player)
{
	// mafm: disabled the chat censor, some people (as me :P) don't like it
	// so maybe won't be even used, but anyway it's useless now

	LogDBG("Console message: '<%s> %s' [to '%s']",
	       player->getPlayerName(),
	       msg->text.c_str(),
	       msg->target.c_str());

	// Find out who the message came from (don't trust the client to tell us
	// the right name)
	MsgChat repmsg;
	repmsg.origin = player->getPlayerName();
	repmsg.text = msg->text;
	if (msg->target.empty()) {
		// target empty, treat as /say command
		vector<LoginData*> nearbyPlayers;
		SrvWorldMgr::instance().getNearbyPlayers(player,
							 mRadiusSay,
							 nearbyPlayers);
		repmsg.type = MsgChat::CHAT;
		SrvNetworkMgr::instance().sendToPlayerList(repmsg,
							   nearbyPlayers);
	} else {
		// target especified, treat as private message
		LoginData* target = SrvLoginMgr::instance().findPlayer(msg->target.c_str());
		repmsg.type = MsgChat::PM;
		if (target && target->isPlaying()) {
			repmsg.target = msg->target;
			SrvNetworkMgr::instance().sendToPlayer(repmsg,
							       target);
		} else {
			// can't find player, bouncing
			repmsg.target = repmsg.origin;
			repmsg.text += " [player '" + msg->target + "' not found]";
			SrvNetworkMgr::instance().sendToPlayer(repmsg, player);
		}
	}
}

void SrvConsoleMgr::processCommand(const MsgCommand* msg, const LoginData* player)
{
	LogNTC("Command: '<%s %d> %s'",
	       player->getPlayerName(),
	       player->getPermissionLevel(),
	       msg->command.c_str());

	// execute
	CommandOutput out;
	SrvCommandMgr::instance().execute(msg->command.c_str(),
					  player->getPermissionLevel(),
					  out);
	// send reply message with result
	MsgChat replymsg;
	replymsg.origin = "Server";
	replymsg.type = MsgChat::SYSTEM;
	replymsg.text = out.getOutput();
	SrvNetworkMgr::instance().sendToPlayer(replymsg, player);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
