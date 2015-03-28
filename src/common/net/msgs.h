/*
 * msgs.h
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

/** \file msgs
 *
 * Here are all the messages that will be sent between client and server.
 */

#ifndef __FEARANN_COMMON_NET_MSGS_H__
#define __FEARANN_COMMON_NET_MSGS_H__


#include "msgbase.h"

#include "common/datatypes.h"

#include <vector>
#include <string>
#include <map>


/** Namespace to hold functions related with messages. */
namespace MsgUtils {

	namespace Errors {
		/** Errors for messages in the initial screens
		 */
		enum Codes {
			SUCCESS = 0,
			EBADLOGIN,
			EALREADYLOGGED,
			EDATABASE,
			ECHARCORRUPT,
			EUSERALREADYEXIST,
			ECHARALREADYEXIST,
			EMAXCHARS,
			ENEWCHARBADDATA,
			ENOSUCHCHAR,
			ECREATEFAILED,
			EALREADYPLAYING,
		};

		/** Human-readable descriptions of errors for messages in the
		 * initial screens.  Using 'int' instead of 'Codes' type for
		 * simplicity, to avoid casts in code: the function returns
		 * "Unknown error"-like message if something goes wrong anyway,
		 * no harm. */
		const char* getDescription(int code);
	}
}



/** Test data types, message to quickly plugin tests and see if the data
 * received has the format that we expect
 */
class MsgTestDataTypes : public MsgBase
{
public:
	std::string str1;
	uint64_t uint64_1;
	uint64_t uint64_2;
	uint64_t uint64_3;
	uint32_t uint32_1;
	uint32_t uint32_2;
	uint32_t uint32_3;
	uint16_t uint16_1;
	uint16_t uint16_2;
	uint16_t uint16_3;
	uint8_t uint8_1;
	uint8_t uint8_2;
	uint8_t uint8_3;
	std::string str2;
	int32_t int32_1;
	int32_t int32_2;
	int32_t int32_3;
	int16_t int16_1;
	int16_t int16_2;
	int16_t int16_3;
	int8_t int8_1;
	int8_t int8_2;
	int8_t int8_3;
	std::string str3;
	char c;
	bool b;
	float f1;
	float f2;
	float f3;
	std::string str4;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A connect message
 */
class MsgConnect : public MsgBase
{
public:
	// Empty message, just a trigger for the server

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A connect reply message
 */
class MsgConnectReply : public MsgBase
{
public:
	/// Result code (0 success, otherwise error codes)
	uint32_t resultCode;
	/// Protocol version (to know what clients can connect)
	std::string protocolVersion;
	/// Uptime
	std::string uptime;
	/// Total number of user accounts
	uint32_t totalUsers;
	/// Total number of characters
	uint32_t totalChars;
	/// Current number of players on-line
	uint32_t currentPlayers;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A login message
 */
class MsgLogin : public MsgBase
{
public:
	/// Username
	std::string username;
	/// Password as md5 checksum
	std::string pw_md5sum;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A login reply message
 */
class MsgLoginReply : public MsgBase
{
public:
	/// Compount type to send info about characters
	class CharacterListEntry
	{
	public:
		std::string name, race, gender, playerClass, area;
	};

	/// Result code (0 success, otherwise error codes)
	uint32_t resultCode;
	/// Number of characters returned
	uint32_t charNumber;
	/// character name, race and gender
	std::vector<CharacterListEntry> charList;

	/** Add a character (convenience function to hide details) */
	void addCharacter(std::string& name, std::string& race,
			  std::string& gender, std::string& playerClass, std::string& area);

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A new user request message
 */
class MsgNewUser : public MsgBase
{
public:
	/// Username 
	std::string username;
	/// Password as md5 checksum
	std::string pw_md5sum;
	/// Email address 
	std::string email;
	/// Real name
	std::string realname;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each
	 * message of course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A new user reply message
 */
class MsgNewUserReply : public MsgBase
{
public:
	/// Result code (0 success, otherwise error codes)
	uint32_t resultCode;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A new character request message
 */
class MsgNewChar : public MsgBase
{
public:
	/// Character name
	std::string charname;
	/// Race
	std::string race;
	/// Gender
	std::string gender;
	/// Class (fighter, sorcerer, ... )
	std::string playerClass;
	/// Character attribute choices
	uint8_t ab_choice_str;
	/// Character attribute choices
	uint8_t ab_choice_con;
	/// Character attribute choices
	uint8_t ab_choice_dex;
	/// Character attribute choices
	uint8_t ab_choice_int;
	/// Character attribute choices
	uint8_t ab_choice_wis;
	/// Character attribute choices
	uint8_t ab_choice_cha;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the
	 * same order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A new character reply message
 */
class MsgNewCharReply : public MsgBase
{
public:
	/// Result code (0 success, otherwise error codes)
	uint32_t resultCode;
	/// Character name
	std::string charname;
	/// Race
	std::string race;
	/// Gender
	std::string gender;
	/// Class
	std::string playerClass;
	/// Area
	std::string area;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A delete character request message
 */
class MsgDelChar : public MsgBase
{
public:
	/// Character name
	std::string charname;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A del character reply message
 */
class MsgDelCharReply : public MsgBase
{
public:
	/// Result code (0 success, otherwise error codes)
	uint32_t resultCode;
	/// Character name
	std::string charname;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A join game request message
 */
class MsgJoin : public MsgBase
{
public:
	/// Charname
	std::string charname;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A join game reply message
 */
class MsgJoinReply : public MsgBase
{
public:
	/// Result code (0 success, otherwise error codes)
	uint32_t resultCode;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A chat message
 */
class MsgChat : public MsgBase
{
public:
	/// Enumeration of the message types
	enum MESSAGE_TYPE
		{
			SYSTEM = 1,
			ACTION,
			CHAT,
			PM
		};
	/// Origin: the sender's player name
	std::string origin;
	/// Target: empty if a public message, player name for PM
	std::string target;
	/// Text: the message itself
	std::string text;
	/// Type of the message (chat, server->client notification, etc)
	uint32_t type;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A command message
 */
class MsgCommand : public MsgBase
{
public:
	/// The command itself
	std::string command;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A message to fill/update the contact list with status info about a contact
 */
class MsgContactStatus : public MsgBase
{
public:
	/// Character name
	std::string charname;
	/// Type (friend, enemy, other)
	char type;
	/// Status
	char status;
	/// Last login
	std::string lastLogin;
	/// Comment
	std::string comment;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A Contact Add message -- to add somebody to the contact list
 */
class MsgContactAdd : public MsgBase
{
public:
	/// Character name
	std::string charname;
	/// Type (friend, enemy, other)
	char type;
	/// Comment
	std::string comment;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A Contact Delete message -- to remove somebody to the contact list
 */
class MsgContactDel : public MsgBase
{
public:
	/// Character name
	std::string charname;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A message to create an entity
 */
class MsgEntityCreate : public MsgBase
{
public:
	/// Entity id, unique and the same in both client and server
	uint64_t entityID;
	/// Entity name
	std::string entityName;
	/// Entity class
	std::string entityClass;
	/// Name of the type (race, type of weapon, ...)
	std::string meshType;
	/// Name of the subtype (gender, concrete model, ...)
	std::string meshSubtype;
	/// Area
	std::string area;
	/// Position
	Vector3 position;
	/// Rotation (up-down axis)
	float rot;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A message to notify of changes in an entity due to movement
 */
class MsgEntityMove : public MsgBase
{
public:
	/// Entity id, unique and the same in both client and server
	uint64_t entityID;
	/// Data for linear movement
	std::string area;
	/// Data for linear movement
	Vector3 position;
	/// Data for linear movement
	Vector3 direction;
	/// Data for linear movement
	float directionSpeed;
	/// Data for linear movement
	float rot;
	/// Data for linear movement
	float rotSpeed;
	/// Data for the action (in meshes in which makes sense)
	bool mov_fwd;
	/// Data for linear movement
	bool mov_bwd;
	/// Data for linear movement
	bool run;
	/// Data for linear movement
	bool rot_left;
	/// Data for linear movement
	bool rot_right;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** A message to destroy an entity
 */
class MsgEntityDestroy : public MsgBase
{
public:
	/// Entity id, unique and the same in both client and server
	uint64_t entityID;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** Inventory listing. It's a convenience message, althought it could be done
 * with multiple InventoryAdd messages.
 */
class MsgInventoryListing : public MsgBase
{
public:
	/// The listing size (necessary to deserialize)
	uint32_t listSize;
	/// The listing
	std::vector<InventoryItem> invListing;

	/// Add an item to the list
	void addItem(InventoryItem* item);

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** Message to request to put an entity inside the inventory (sent by the client
 * when the player tries to pick up an object)
 */
class MsgInventoryGet : public MsgBase
{
public:
	/// The target item
	uint64_t itemID;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** Message to add an item to the inventory (sent by the server, either by
 * client request or by other causes like NPCs giving something)
 */
class MsgInventoryAdd : public MsgBase
{
public:
	/// The item
	InventoryItem item;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** Message to request to drop an entity from the inventory (sent by the client
 * when the player asks to do so)
 */
class MsgInventoryDrop : public MsgBase
{
public:
	/// The target item
	uint64_t itemID;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** Message to delete an item from the inventory (sent by the server, either by
 * client request or by other causes like NPCs requesting something)
 */
class MsgInventoryDel : public MsgBase
{
public:
	/// The item
	uint64_t itemID;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** All the data concerning one player, in the form of message ready to be send
 * to the client to update own data
 */
class MsgPlayerData : public MsgBase
{
public:
	/// The player max health
	int16_t health_max;
	/// The player current health
	int16_t health_cur;
	/// The player max magic energy
	int16_t magic_max;
	/// The player current magic energy
	int16_t magic_cur;
	/// The max amount of weight/volume that players can carry
	int16_t load_max;
	/// The current load
	int16_t load_cur;
	/// The player stamina, range 1-100 (kind of tiredness/fatigue that
	/// affects a lot of actions and self-recuperation)
	int16_t stamina;
	/// The amount of gold (coins, units)
	int32_t gold;
	/// level of the player
	int32_t level;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t ab_con;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t abe_con;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t ab_str;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t abe_str;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t ab_dex;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t abe_dex;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t ab_int;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t abe_int;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t ab_wis;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t abe_wis;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t ab_cha;
	/// Basic abilities, 'e' after 'ab' means effective/current
	int16_t abe_cha;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** This message is sent every minute in the game, (which are 5 seconds of real
 * time at the time of writing this).
 */
class MsgTimeMinute : public MsgBase
{
public:
	/// The game time
	uint32_t gametime;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** This list is sent from client to server to ask for files
 */
class MsgContentQueryUpdate : public MsgBase
{
public:
	/// Total number of files sent from the client content tree
	uint32_t totalFiles;
	/// List of pairs filename:updatekey (content tree in the client)
	std::vector<NameValuePair> filepairs;

	/** Add a file+updatekey to this message. Client should send all the
	 * files in the local content dir, so server can then check for
	 * outdated, missing or unneded files; and give proper instructions to
	 * keep the client synced. */
	void addFile(const std::string& filename, const std::string& updatekey);

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** This message is sent from server to client. It is a list of files that the
 * client should delete, as well as the number of files to update and total size
 * of the download needed (this part is to display progress when downloading).
 */
class MsgContentDeleteList : public MsgBase
{
public:
	/// Number of files to delete (length of the following list)
	uint32_t filesToDelete;
	/// List of files to delete
	std::vector<std::string> deleteList;

	/** Add a file to delete, we must provide just the filename and the
	 * counter is updated when serialized. We need the filename here because
	 * the client can delete the files as soon as it receives this
	 * message */
	void addFile(std::string& filename);

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** This file list is sent from server to client. Client will receive file parts
 * immediately after this message
 */
class MsgContentUpdateList : public MsgBase
{
public:
	/// Class to represent the basic info of a file to be transferred
	class ContentUpdateFileInfo
	{
	public:
		std::string filename;
		std::string updatekey;
		uint32_t transferID;
		uint32_t numParts;
		uint32_t size;
	};

	/// Number of files to update (length of the following list)
	uint32_t filesToUpdate;
	/// Files to update
	std::vector<ContentUpdateFileInfo> updateList;

	/** Add a file to update, we must provide just the filename and other
	 * params and the counter is updated when serialized. */
	void addFile(const char* filename, const char* updatekey,
		     uint32_t transfer_id, uint32_t num_parts, uint32_t size);

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** This class represents the part of a file, sent from server to client when
 * updating content. */
class MsgContentFilePart : public MsgBase
{
public:
	/** This ID is used by client and server to specify a particular file
	 *  during the transfer. */
	uint32_t transferID;
	/// Part index
	uint32_t partNum;
	/// Size of the data
	uint32_t size;
	/** The data itself. */
	std::vector<char> buffer;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};

/** This class represents a trade session
 */
class MsgTrade : public MsgBase
{
public:
	/// Enumeration of the message types
	enum MESSAGE_TYPE
		{
			START = 1, //start trading session
			END, //end the trading session
			ACCEPT, //target will accept or deny the trade 'start'
			UPDATE_LIST,
			COMMIT,
			COMMIT_ACCEPT,
			COMMIT_REJECT
		};
	/// name of character trading - only used on msg handling side.
	std::string player;
	/// name of character to trade with.
	std::string target;
	/// Message type (start, end, accept, deny)
	uint32_t type;
	/// The listing size (necessary to deserialize)
	uint32_t listSize;
	/// The selected player (necessary to deserialize)
	uint32_t plListSize;
	/// The selected target (necessary to deserialize)
	uint32_t tgListSize;
	/// The listing
	std::vector<InventoryItem> itemList;
	/// Selected items - player
	std::vector<int> playerSelectedList;
	/// Selected items - target
	std::vector<int> targetSelectedList;

	/// Add an item to the list
	void addItem( InventoryItem* item );
	void addPlayerSelectedItem( int itemid);
	void addTargetSelectedItem( int itemid);
	MESSAGE_TYPE getState();

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** This class represents a trade session
 */
class MsgCombat : public MsgBase
{
public:
	enum BATTLE_STATE { START=1, ACCEPTED, END };

	/// normal battles are automatically accepted, duel's aren't
	enum BATTLE_TYPE { NORMAL=1, DUEL };

	/// used again as a place holder...
	uint64_t player;
	/// name of character to trade with.
	uint64_t target;
	/// Message state (start, end, accept)
	uint32_t state;
	/// Message type (normal, duel)
	uint32_t type;

	BATTLE_STATE getState();
	BATTLE_TYPE getBattleType();

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** This class represents a combat action
 */
class MsgCombatAction : public MsgBase
{
public:
	enum BATTLE_ACTION { ATTACK = 1, DEFEND, NON_COMBAT };
	enum SPECIAL_ACTION { SPELL = 1, SPECIAL };

	/// used again as a place holder...
	std::string player;
	/// name of character to trade with.
	std::string target;
	/// Action (attack, defend, more to come...)
	uint32_t action;
	/// type of special action.
	uint32_t sp_action_type;
	/// name of special attack or spell ("ray of frost").
	std::string sp_action;

	BATTLE_ACTION getAction();
	SPECIAL_ACTION getSpecialAction();

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** This class represents a combat hit/miss (physical or otherwise)
 */
class MsgCombatResult : public MsgBase
{
public:
	enum BATTLE_RESULT { HIT = 1, MISS, OTHER };

	/// target - needed because this will be sent to both the defender and
	/// attacker
	uint64_t target;
	/// Damage amount
	uint32_t damage;
	/// Result of attack
	uint32_t result;

	BATTLE_RESULT getResult();

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** NPC Dialog
 */
class MsgNPCDialog : public MsgBase
{
public:
	/// Origin: the sender's player name
	std::string origin;
	/// Target: empty if a public message, player name for PM
	std::string target;
	/// Text: the message itself
	std::string text;
	/// options for the player to choose - to continue the dialog.
	std::vector<NPCDialogOption> options;
	/// Dialog ended.
	bool done;

	void addOption(NPCDialogOption* option);

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
};


/** NPC Dialog - player's reply to NPC.
 */
class MsgNPCDialogReply : public MsgBase
{
public:
	/// Origin: the sender's player name
	std::string origin;
	/// Target: empty if a public message, player name for PM
	uint64_t target;
	/// Text: the message itself
	uint32_t option;
	/// Dialog ended.
	bool done;

public:
	/* Common abstract part that the it has to be defined */

	static MsgType mType;

	/** Returns the message type, it has to be different for each message of
	 * course. */
	MsgType getType() const { return mType; }
	/** Returns an instance of the message */
	virtual MsgBase* createInstance();
	/** Performs the serialization of the data defined in this derived
	 * class, the real content of the message. */
	virtual void serializeData();
	/** Reverse action of serialization, it must be performed in the same
	 * order to obtain an exact copy of the message in the peer */
	virtual void deserializeData();
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
