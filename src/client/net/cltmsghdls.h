/*
 * cltmsghdls.h
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

#ifndef __FEARANN_CLIENT_NET_MSGHDLS_H__
#define __FEARANN_CLIENT_NET_MSGHDLS_H__

#include "common/net/msgbase.h"


//---------------------------------------------------------------
// Test
//---------------------------------------------------------------

class CltMsgHdlTestDataTypes : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

//---------------------------------------------------------------
// Connections
//---------------------------------------------------------------

class CltMsgHdlConnectReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlLoginReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlJoinReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlNewUserReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlNewCharReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlDelCharReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Content
//---------------------------------------------------------------

class CltMsgHdlContentFilePart : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlContentDeleteList : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlContentUpdateList : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Console
//---------------------------------------------------------------

class CltMsgHdlChat : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Dialog
//---------------------------------------------------------------

class CltMsgHdlNPCDialog : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Contact list
//---------------------------------------------------------------

class CltMsgHdlContactStatus : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Entities
//---------------------------------------------------------------

class CltMsgHdlEntityCreate : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlEntityMove : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlEntityDestroy : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Inventory
//---------------------------------------------------------------

class CltMsgHdlInventoryListing : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlInventoryAdd : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlInventoryDel : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Player actions
//---------------------------------------------------------------

class CltMsgHdlGetItemReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};

class CltMsgHdlDropItemReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Player data
//---------------------------------------------------------------

class CltMsgHdlPlayerData : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Time
//---------------------------------------------------------------

class CltMsgHdlTimeMinute : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Trade
//---------------------------------------------------------------

class CltMsgHdlTrade : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


//---------------------------------------------------------------
// Combat
//---------------------------------------------------------------

class CltMsgHdlCombat : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink = 0);
};


class CltMsgHdlCombatResult : public MsgHdlBase
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
