/*
 * botmsghdls.h
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

#ifndef __FEARANN_BOT_NET_MSGHDLS_H__
#define __FEARANN_BOT_NET_MSGHDLS_H__

#include "common/net/msgbase.h"


//---------------------------------------------------------------
// Test
//---------------------------------------------------------------

class MsgHdlTestDataTypes : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

//---------------------------------------------------------------
// Connections
//---------------------------------------------------------------

class MsgHdlConnectReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlLoginReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlJoinReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlNewUserReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlNewCharReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlDelCharReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Content
//---------------------------------------------------------------

class MsgHdlContentFilePart : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlContentDeleteList : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlContentUpdateList : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Console
//---------------------------------------------------------------

class MsgHdlChat : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Dialog
//---------------------------------------------------------------

class MsgHdlNPCDialogReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Contact list
//---------------------------------------------------------------

class MsgHdlContactStatus : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Entities
//---------------------------------------------------------------

class MsgHdlEntityCreate : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlEntityMove : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlEntityDestroy : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Inventory
//---------------------------------------------------------------

class MsgHdlInventoryListing : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlInventoryAdd : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlInventoryDel : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Player actions
//---------------------------------------------------------------

class MsgHdlGetItemReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlDropItemReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Player data
//---------------------------------------------------------------

class MsgHdlPlayerData : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


// Time
//---------------------------------------------------------------

class MsgHdlTimeMinute : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

// Trade
//---------------------------------------------------------------

class MsgHdlTrade : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

// Combat
//---------------------------------------------------------------

class MsgHdlCombat : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlCombatAction : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class MsgHdlCombatResult : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
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
