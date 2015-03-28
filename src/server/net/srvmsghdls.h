/*
 * srvmsghdls.h
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

/// @ingroup srvnet
/// @{

/** \file srvmsghdls
 *
 * This file contains all the server message handlers, which decode the data in
 * the message and relay the message to the corresponding manager.
 */

#ifndef __FEARANN_SERVER_NET_MSGHDLS_H__
#define __FEARANN_SERVER_NET_MSGHDLS_H__


#include "common/net/msgbase.h"


//---------------------------------------------------------------
// Connections
//---------------------------------------------------------------

class MsgHdlConnect : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

class MsgHdlLogin : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

class MsgHdlJoin : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

class MsgHdlNewUser : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

class MsgHdlNewChar : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

class MsgHdlDelChar : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};


//---------------------------------------------------------------
// Content
//---------------------------------------------------------------

class MsgHdlContentQueryUpdate : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};


//---------------------------------------------------------------
// Console
//---------------------------------------------------------------

class MsgHdlChat : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

class MsgHdlCommand : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

//---------------------------------------------------------------
// Dialog
//---------------------------------------------------------------

class MsgHdlNPCDialog : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};


class MsgHdlNPCDialogReply : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};


//---------------------------------------------------------------
// Contact list
//---------------------------------------------------------------

class MsgHdlContactAdd : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

class MsgHdlContactDel : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};


//---------------------------------------------------------------
// Entities
//---------------------------------------------------------------

class MsgHdlEntityMove : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};


//---------------------------------------------------------------
// Inventory
//---------------------------------------------------------------

class MsgHdlInventoryGet : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

class MsgHdlInventoryDrop : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

//---------------------------------------------------------------
// Trade
//---------------------------------------------------------------

class MsgHdlTrade : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

//---------------------------------------------------------------
// Combat
//---------------------------------------------------------------

class MsgHdlCombat : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};

class MsgHdlCombatAction : public MsgHdlBase
{
public:
	virtual MsgType getMsgType() const;
	virtual void handleMsg(MsgBase& msg, Netlink* netlink);
};


#endif

/// @}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
