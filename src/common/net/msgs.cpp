/*
 * msgs.cpp
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

#include "config.h"

#include "msgs.h"

#include <cstdlib>


const char* MsgUtils::Errors::getDescription(int code)
{
	switch (code) {
	case SUCCESS:
		return "Success";
	case EBADLOGIN:
		return "Wrong login/password";
	case EALREADYLOGGED:
		return "Already logged";
	case EDATABASE:
		return "Database error (Internal server error)";
	case ECHARCORRUPT:
		return "Characters corrupted (Internal server error)";
	case EUSERALREADYEXIST:
		return "Username already exists";
	case EMAXCHARS:
		return "Max number of characters per account reached";
	case ENEWCHARBADDATA:
		return "Bad data for new character (race, gender, class, points...)";
	case ECREATEFAILED:
		return "Couldn't create player in the world (Internal server error)";
	case EALREADYPLAYING:
		return "Already playing";
	default:
		return "Unknown error code";
	}
}

//------------------------------------------------------------
// TestDataTypes
//------------------------------------------------------------
MsgType MsgTestDataTypes::mType("Test");

MsgBase* MsgTestDataTypes::createInstance()
{
	return new MsgTestDataTypes;
}

void MsgTestDataTypes::serializeData()
{
	str1 = "String 1"; write(str1);
	uint64_1 = 0xffffffffffffffffLL; write(uint64_1);
	uint64_2 = 0; write(uint64_2);
	uint64_3 = +1234567890123456789LL; write(uint64_3);
	uint32_1 = 0xffffffff; write(uint32_1);
	uint32_2 = 0; write(uint32_2);
	uint32_3 = +1234567890; write(uint32_3);
	uint16_1 = 0xffff; write(uint16_1);
	uint16_2 = 0; write(uint16_2);
	uint16_3 = +12345; write(uint16_3);
	uint8_1 = 0xff; write(uint8_1);
	uint8_2 = 0; write(uint8_2);
	uint8_3 = +123; write(uint8_3);
	str2 = "String 2"; write(str2);
	int32_1 = -123456789; write(int32_1);
	int32_2 = 0; write(int32_2);
	int32_3 = +123456789; write(int32_3);
	int16_1 = -12345; write(int16_1);
	int16_2 = 0; write(int16_2);
	int16_3 = +12345; write(int16_3);
	int8_1 = -123; write(int8_1);
	int8_2 = 0; write(int8_2);
	int8_3 = +123; write(int8_3);
	str3 = "String 3"; write(str3);
	c = 'c'; write(c);
	b = true; write(b);
	f1 = -123456.78f; write(f1);
	f2 = 0.0f; write(f2);
	f3 = +123456.78f; write(f3);
	str4 = "String 4"; write(str4);
}

void MsgTestDataTypes::deserializeData()
{
	read(str1); LogDBG("String 1: '%s'", str1.c_str()); 
	read(uint64_1); LogDBG("0xffffffffffffffff: '%lx'", uint64_1);
	read(uint64_2); LogDBG("0: '%lu'", uint64_2);
	read(uint64_3); LogDBG("+1234567890123456789: '%lu'", uint64_3);
	read(uint32_1); LogDBG("0xffffffff: '%x'", uint32_1); 
	read(uint32_2); LogDBG("0: '%u'", uint32_2); 
	read(uint32_3); LogDBG("+1234567890: '%u'", uint32_3); 
	read(uint16_1); LogDBG("0xffff: '%x'", uint16_1); 
	read(uint16_2); LogDBG("0: '%u'", uint16_2); 
	read(uint16_3); LogDBG("+12345: '%u'", uint16_3); 
	read(uint8_1); LogDBG("0xff: '%x'", uint8_1); 
	read(uint8_2); LogDBG("0: '%u'", uint8_2); 
	read(uint8_3); LogDBG("+123: '%u'", uint8_3); 
	read(str2); LogDBG("String 2: '%s'", str2.c_str()); 
	read(int32_1); LogDBG("-123456789: '%d'", int32_1); 
	read(int32_2); LogDBG("0: '%d'", int32_2); 
	read(int32_3); LogDBG("+123456789: '%d'", int32_3); 
	read(int16_1); LogDBG("-12345: '%d'", int16_1); 
	read(int16_2); LogDBG("0: '%d'", int16_2); 
	read(int16_3); LogDBG("+12345: '%d'", int16_3); 
	read(int8_1); LogDBG("-123: '%d'", int8_1); 
	read(int8_2); LogDBG("0: '%d'", int8_2); 
	read(int8_3); LogDBG("+123: '%d'", int8_3); 
	read(str3); LogDBG("String 3: '%s'", str3.c_str()); 
	read(c); LogDBG("c: '%c'", c); 
	read(b); LogDBG("1 (bool): '%d'", b); 
	read(f1); LogDBG("-123456.78f: '%8.02f'", f1); 
	read(f2); LogDBG("0: '%g'", f2); 
	read(f3); LogDBG("+123456.78f: '%8.02f'", f3); 
	read(str4); LogDBG("String 4: '%s'", str4.c_str()); 
}



//------------------------------------------------------------
// Connections
//------------------------------------------------------------
MsgType MsgConnect::mType("Conn");

MsgBase* MsgConnect::createInstance()
{
	return new MsgConnect;
}

void MsgConnect::serializeData()
{
}

void MsgConnect::deserializeData()
{
}


MsgType MsgConnectReply::mType("ConR");

MsgBase* MsgConnectReply::createInstance()
{
	return new MsgConnectReply;
}

void MsgConnectReply::serializeData()
{
	write(resultCode);
	write(protocolVersion);
	write(uptime);
	write(totalUsers);
	write(totalChars);
	write(currentPlayers);
}

void MsgConnectReply::deserializeData()
{
	read(resultCode);
	read(protocolVersion);
	read(uptime);
	read(totalUsers);
	read(totalChars);
	read(currentPlayers);
}


MsgType MsgLogin::mType("Logi");

MsgBase* MsgLogin::createInstance()
{
	return new MsgLogin;
}

void MsgLogin::serializeData()
{
	write(username);
	write(pw_md5sum);
}

void MsgLogin::deserializeData()
{
	read(username);
	read(pw_md5sum);
}


MsgType MsgLoginReply::mType("LogR");

MsgBase* MsgLoginReply::createInstance()
{
	return new MsgLoginReply;
}

void MsgLoginReply::serializeData()
{
	charNumber = charList.size();
	write(resultCode);
	write(charNumber);
	for (size_t i = 0; i < charNumber; ++i) {
		write(charList[i].name);
		write(charList[i].race);
		write(charList[i].gender);
		write(charList[i].playerClass);
		write(charList[i].area);
	}
}

void MsgLoginReply::deserializeData()
{
	read(resultCode);
	read(charNumber);
	for (size_t i = 0; i < charNumber; ++i) {
		string name = "<none>";
		string race = "<none>";
		string gender = "<none>";
		string playerClass = "<none>";
		string area = "<none>";
		read(name);
		read(race);
		read(gender);
		read(playerClass);
		read(area);
		addCharacter(name, race, gender, playerClass, area);
	}
}

void MsgLoginReply::addCharacter(string& name, string& race,
				   string& gender, string& playerClass, string& area)
{
	CharacterListEntry character;
	character.name = name;
	character.race = race;
	character.gender = gender;
	character.playerClass = playerClass;
	character.area = area;
	charList.push_back(character);
}


MsgType MsgNewUser::mType("NUsr");

MsgBase* MsgNewUser::createInstance()
{
	return new MsgNewUser;
}

void MsgNewUser::serializeData()
{
	write(username);
	write(pw_md5sum);
	write(email);
	write(realname);
}

void MsgNewUser::deserializeData()
{
	read(username);
	read(pw_md5sum);
	read(email);
	read(realname);
}


MsgType MsgNewUserReply::mType("NUsR");

MsgBase* MsgNewUserReply::createInstance()
{
	return new MsgNewUserReply;
}

void MsgNewUserReply::serializeData()
{
	write(resultCode);
}

void MsgNewUserReply::deserializeData()
{
	read(resultCode);
}


MsgType MsgNewChar::mType("NCha");

MsgBase* MsgNewChar::createInstance()
{
	return new MsgNewChar;
}

void MsgNewChar::serializeData()
{
	write(charname);
	write(race);
	write(gender);
	write(playerClass);
	write(ab_choice_con);
	write(ab_choice_str);
	write(ab_choice_dex);
	write(ab_choice_int);
	write(ab_choice_wis);
	write(ab_choice_cha);
}

void MsgNewChar::deserializeData()
{
	read(charname);
	read(race);
	read(gender);
	read(playerClass);
	read(ab_choice_con);
	read(ab_choice_str);
	read(ab_choice_dex);
	read(ab_choice_int);
	read(ab_choice_wis);
	read(ab_choice_cha);
}


MsgType MsgNewCharReply::mType("NChR");

MsgBase* MsgNewCharReply::createInstance()
{
	return new MsgNewCharReply;
}

void MsgNewCharReply::serializeData()
{
	write(resultCode);
	write(charname);
	write(race);
	write(gender);
	write(playerClass);
	write(area);
}

void MsgNewCharReply::deserializeData()
{
	read(resultCode);
	read(charname);
	read(race);
	read(gender);
	read(playerClass);
	read(area);
}


MsgType MsgDelChar::mType("DCha");

MsgBase* MsgDelChar::createInstance()
{
	return new MsgDelChar;
}

void MsgDelChar::serializeData()
{
	write(charname);
}

void MsgDelChar::deserializeData()
{
	read(charname);
}


MsgType MsgDelCharReply::mType("DChR");

MsgBase* MsgDelCharReply::createInstance()
{
	return new MsgDelCharReply;
}

void MsgDelCharReply::serializeData()
{
	write(resultCode);
	write(charname);
}

void MsgDelCharReply::deserializeData()
{
	read(resultCode);
	read(charname);
}


MsgType MsgJoin::mType("Join");

MsgBase* MsgJoin::createInstance()
{
	return new MsgJoin;
}

void MsgJoin::serializeData()
{
	write(charname);
}

void MsgJoin::deserializeData()
{
	read(charname);
}


MsgType MsgJoinReply::mType("JoiR");

MsgBase* MsgJoinReply::createInstance()
{
	return new MsgJoinReply;
}

void MsgJoinReply::serializeData()
{
	write(resultCode);
}

void MsgJoinReply::deserializeData()
{
	read(resultCode);
}


//------------------------------------------------------------
// Console
//------------------------------------------------------------
MsgType MsgChat::mType("Chat");

MsgBase* MsgChat::createInstance()
{
	return new MsgChat;
}

void MsgChat::serializeData()
{
	write(origin);
	write(target);
	write(text);
	write(type);
}

void MsgChat::deserializeData()
{
	read(origin);
	read(target);
	read(text);
	read(type);
}


MsgType MsgCommand::mType("Cmmd");

MsgBase* MsgCommand::createInstance()
{
	return new MsgCommand;
}

void MsgCommand::serializeData()
{
	write(command);
}

void MsgCommand::deserializeData()
{
	read(command);
}


//------------------------------------------------------------
// Contacts
//------------------------------------------------------------
MsgType MsgContactStatus::mType("CtSt");

MsgBase* MsgContactStatus::createInstance()
{
	return new MsgContactStatus;
}

void MsgContactStatus::serializeData()
{
	write(charname);
	write(type);
	write(status);
	write(lastLogin);
	write(comment);
}

void MsgContactStatus::deserializeData()
{
	read(charname);
	read(type);
	read(status);
	read(lastLogin);
	read(comment);
}


MsgType MsgContactAdd::mType("CtAd");

MsgBase* MsgContactAdd::createInstance()
{
	return new MsgContactAdd;
}

void MsgContactAdd::serializeData()
{
	write(charname);
	write(type);
	write(comment);
}

void MsgContactAdd::deserializeData()
{
	read(charname);
	read(type);
	read(comment);
}


MsgType MsgContactDel::mType("CtDl");

MsgBase* MsgContactDel::createInstance()
{
	return new MsgContactDel;
}

void MsgContactDel::serializeData()
{
	write(charname);
}

void MsgContactDel::deserializeData()
{
	read(charname);
}


//------------------------------------------------------------
// Entities
//------------------------------------------------------------
MsgType MsgEntityCreate::mType("EnCr");

MsgBase* MsgEntityCreate::createInstance()
{
	return new MsgEntityCreate;
}

void MsgEntityCreate::serializeData()
{
	write(entityID);
	write(entityName);
	write(entityClass);
	write(meshType);
	write(meshSubtype);
	write(area);
	write(position);
	write(rot);
}

void MsgEntityCreate::deserializeData()
{
	read(entityID);
	read(entityName);
	read(entityClass);
	read(meshType);
	read(meshSubtype);
	read(area);
	read(position);
	read(rot);
}


MsgType MsgEntityMove::mType("EnMv");

MsgBase* MsgEntityMove::createInstance()
{
	return new MsgEntityMove;
}

void MsgEntityMove::serializeData()
{
	write(entityID);
	write(area);
	write(position);
	write(direction);
	write(directionSpeed);
	write(rot);
	write(rotSpeed);
	write(mov_fwd);
	write(mov_bwd);
	write(run);
	write(rot_left);
	write(rot_right);
}

void MsgEntityMove::deserializeData()
{
	read(entityID);
	read(area);
	read(position);
	read(direction);
	read(directionSpeed);
	read(rot);
	read(rotSpeed);
	read(mov_fwd);
	read(mov_bwd);
	read(run);
	read(rot_left);
	read(rot_right);
}


MsgType MsgEntityDestroy::mType("EnDt");

MsgBase* MsgEntityDestroy::createInstance()
{
	return new MsgEntityDestroy;
}

void MsgEntityDestroy::serializeData()
{
	write(entityID);
}

void MsgEntityDestroy::deserializeData()
{
	read(entityID);
}


//------------------------------------------------------------
// Inventory
//------------------------------------------------------------
MsgType MsgInventoryListing::mType("IvLt");

MsgBase* MsgInventoryListing::createInstance()
{
	return new MsgInventoryListing;
}

void MsgInventoryListing::serializeData()
{
	string type,subtype;
	listSize = invListing.size();
	write(listSize);
	for (size_t i = 0; i < listSize; ++i) {
		InventoryItem item = invListing[i];
		type = item.getType();
		subtype = item.getSubtype();
		write(atoi(item.getItemID()));
		write(type);
		write(subtype);
		write(item.getLoad());
	}
}

void MsgInventoryListing::deserializeData()
{
	read(listSize);
	for (size_t i = 0; i < listSize; ++i) {
		uint32_t itemID;
		string meshType, meshSubtype;
		float load;
		read(itemID);
		read(meshType);
		read(meshSubtype);
		read(load);
		invListing.push_back(InventoryItem(itemID, meshType, meshSubtype, load));
	}
}


void MsgInventoryListing::addItem(InventoryItem* item)
{
	invListing.push_back(*item);
}


MsgType MsgInventoryGet::mType("IvGt");

MsgBase* MsgInventoryGet::createInstance()
{
	return new MsgInventoryGet;
}

void MsgInventoryGet::serializeData()
{
	write(itemID);
}

void MsgInventoryGet::deserializeData()
{
	read(itemID);
}


MsgType MsgInventoryAdd::mType("IvAd");

MsgBase* MsgInventoryAdd::createInstance()
{
	return new MsgInventoryAdd;
}

void MsgInventoryAdd::serializeData()
{
	string type(item.getType());
	string subtype(item.getSubtype());
	write(atoi(item.getItemID()));
	write(type);
	write(subtype);
	write(item.getLoad());
}

void MsgInventoryAdd::deserializeData()
{
	uint32_t itemID;
	string meshType, meshSubtype;
	float load;
	read(itemID);
	read(meshType);
	read(meshSubtype);
	read(load);
	item.setItemID(itemID);
	item.setType(meshType);
	item.setSubtype(meshSubtype);
	item.setLoad(load);
}


MsgType MsgInventoryDrop::mType("IvDp");

MsgBase* MsgInventoryDrop::createInstance()
{
	return new MsgInventoryDrop;
}

void MsgInventoryDrop::serializeData()
{
	write(itemID);
}

void MsgInventoryDrop::deserializeData()
{
	read(itemID);
}


MsgType MsgInventoryDel::mType("IvDl");

MsgBase* MsgInventoryDel::createInstance()
{
	return new MsgInventoryDel;
}

void MsgInventoryDel::serializeData()
{
	write(itemID);
}

void MsgInventoryDel::deserializeData()
{
	read(itemID);
}


//------------------------------------------------------------
// Player data
//------------------------------------------------------------
MsgType MsgPlayerData::mType("PlDa");

MsgBase* MsgPlayerData::createInstance()
{
	return new MsgPlayerData;
}

void MsgPlayerData::serializeData()
{
	write(health_max);
	write(health_cur);
	write(magic_max);
	write(magic_cur);
	write(load_max);
	write(load_cur);
	write(stamina);
	write(gold);
	write(level);
	write(ab_con);
	//write(abe_con);
	write(ab_str);
	//write(abe_str);
	write(ab_dex);
	//write(abe_agi);
	write(ab_int);
	//write(abe_int);
	write(ab_wis);
	//write(abe_wis);
	write(ab_cha);
	//write(abe_cha);
}

void MsgPlayerData::deserializeData()
{
	read(health_max);
	read(health_cur);
	read(magic_max);
	read(magic_cur);
	read(load_max);
	read(load_cur);
	read(stamina);
	read(gold);
	read(level);
	read(ab_con);
	//read(abe_con);
	read(ab_str);
	//read(abe_str);
	read(ab_dex);
	//read(abe_agi);
	read(ab_int);
	//read(abe_int);
	read(ab_wis);
	//read(abe_wis);
	read(ab_cha);
	//read(abe_cha);
}


//------------------------------------------------------------
// Time
//------------------------------------------------------------
MsgType MsgTimeMinute::mType("TmMn");

MsgBase* MsgTimeMinute::createInstance()
{
	return new MsgTimeMinute;
}

void MsgTimeMinute::serializeData()
{
	write(gametime);
}

void MsgTimeMinute::deserializeData()
{
	read(gametime);
}


//------------------------------------------------------------
// Content
//------------------------------------------------------------
MsgType MsgContentQueryUpdate::mType("CQUp");

MsgBase* MsgContentQueryUpdate::createInstance()
{
	return new MsgContentQueryUpdate;
}

void MsgContentQueryUpdate::serializeData()
{
	totalFiles = filepairs.size();
	write(totalFiles);
	for (size_t i = 0; i < totalFiles; ++i) {
		write(filepairs[i].name);
		write(filepairs[i].value);
	}
}

void MsgContentQueryUpdate::deserializeData()
{
	read(totalFiles);
	for (size_t i = 0; i < totalFiles; ++i) {
		string filename, updatekey;
		read(filename);
		read(updatekey);
		addFile(filename, updatekey);
	}
}

void MsgContentQueryUpdate::addFile(const string& filename,
				    const string& updatekey)
{
	filepairs.push_back(NameValuePair(filename, updatekey));
}


MsgType MsgContentDeleteList::mType("CDel");

MsgBase* MsgContentDeleteList::createInstance()
{
	return new MsgContentDeleteList;
}

void MsgContentDeleteList::serializeData()
{
	filesToDelete = deleteList.size();
	write(filesToDelete);
	for (size_t i = 0; i < filesToDelete; ++i) 
	{
		write(deleteList[i]);
	}
}

void MsgContentDeleteList::deserializeData()
{
	read(filesToDelete);
	for (size_t i = 0; i < filesToDelete; ++i) 
	{
		string filename;
		read(filename);
		addFile(filename);
	}
}

void MsgContentDeleteList::addFile(string& filename)
{
	deleteList.push_back(filename);
}


MsgType MsgContentUpdateList::mType("CUpd");

MsgBase* MsgContentUpdateList::createInstance()
{
	return new MsgContentUpdateList;
}

void MsgContentUpdateList::serializeData()
{
	filesToUpdate = updateList.size();
	write(filesToUpdate);
	for (size_t i = 0; i < filesToUpdate; ++i) {
		write(updateList[i].transferID);
		write(updateList[i].numParts);
		write(updateList[i].filename);
		write(updateList[i].updatekey);
		write(updateList[i].size);
	}
}

void MsgContentUpdateList::deserializeData()
{
	read(filesToUpdate);
	for (size_t i = 0; i < filesToUpdate; ++i) 
	{
		ContentUpdateFileInfo file;
		read(file.transferID);
		read(file.numParts);
		read(file.filename);
		read(file.updatekey);
		read(file.size);
		updateList.push_back(file);
	}
}

void MsgContentUpdateList::addFile(const char* filename,
				   const char* updatekey,
				   uint32_t transfer_id,
				   uint32_t num_parts,
				   uint32_t size)
{
	ContentUpdateFileInfo file;
	file.transferID = transfer_id;
	file.numParts = num_parts;
	file.filename = filename;
	file.updatekey = updatekey;
	file.size = size;
	updateList.push_back(file);
}


MsgType MsgContentFilePart::mType("CFPa");

MsgBase* MsgContentFilePart::createInstance()
{
	return new MsgContentFilePart;
}

void MsgContentFilePart::serializeData()
{
	size = buffer.size();
	write(transferID);
	write(partNum);
	write(size);
	write(&buffer[0], size);
}

void MsgContentFilePart::deserializeData()
{
	read(transferID);
	read(partNum);
	read(size);
	char* buffaux = new char[size];
	read(buffaux, size);
	for (size_t i = 0; i < size; ++i) {
		buffer.push_back(buffaux[i]);
	}
	delete[] buffaux;
}

//--------------------------------------------
// Trading
//--------------------------------------------

MsgType MsgTrade::mType("Trde");

MsgBase* MsgTrade::createInstance()
{
	return new MsgTrade;
}

void MsgTrade::serializeData()
{
	string meshType,meshSubtype;
	listSize = itemList.size();
  	plListSize = playerSelectedList.size();
	tgListSize = targetSelectedList.size();
/*
	LogDBG("serial: list: %d, plList: %d, tgList: %d", itemList.size(), 
												playerSelectedList.size(), 
												targetSelectedList.size() );
*/
	write(type);
	write(target);
	write(listSize);
	write(plListSize);
	write(tgListSize);
	for (size_t i = 0; i < listSize; ++i) {
		InventoryItem item = itemList[i];
		meshType = item.getType();
		meshSubtype = item.getSubtype();
		write(atoi(item.getItemID()));
		write(meshType);
		write(meshSubtype);
		write(item.getLoad());
	}
	int item;
	for (size_t i = 0; i < plListSize; ++i) {
		item = playerSelectedList[i];
		write(item);
	}
	for (size_t i = 0; i < tgListSize; ++i) {
		item = targetSelectedList[i];
		write(item);
	}
}


void MsgTrade::deserializeData()
{
	read(type);
	read(target);
	read(listSize);
	read(plListSize);
	read(tgListSize);
/*
	LogDBG("deserial: list: %d, plList: %d, tgList: %d", listSize, 
												plListSize, 
												tgListSize );
*/
	for (size_t i = 0; i < listSize; ++i) {
		uint32_t itemID;
		string meshType, meshSubtype;
		float load;
		read(itemID);
		read(meshType);
		read(meshSubtype);
		read(load);
		itemList.push_back(InventoryItem(itemID, meshType, meshSubtype, load));
	}
	int item;
	for (size_t i = 0; i < plListSize; ++i) {
		read(item);
		playerSelectedList.push_back(item);
	}
	for (size_t i = 0; i < tgListSize; ++i) {
		read(item);
		targetSelectedList.push_back(item);
	}
}


void MsgTrade::addItem(InventoryItem* item)
{
	itemList.push_back(*item);
}


void MsgTrade::addPlayerSelectedItem( int itemid )
{
	playerSelectedList.push_back(itemid);
}


void MsgTrade::addTargetSelectedItem( int itemid )
{
	targetSelectedList.push_back(itemid);
}


MsgTrade::MESSAGE_TYPE MsgTrade::getState( void )
{
	return (MsgTrade::MESSAGE_TYPE)type;
}


MsgType MsgCombat::mType("Cmbt");

MsgBase* MsgCombat::createInstance()
{
	return new MsgCombat;
}

void MsgCombat::serializeData()
{
	write(target);
	write(type);
	write(state);
}

void MsgCombat::deserializeData()
{
	read(target);
	read(type);
	read(state);
}

MsgCombat::BATTLE_STATE MsgCombat::getState( void )
{
    return (MsgCombat::BATTLE_STATE)state;
}

MsgCombat::BATTLE_TYPE MsgCombat::getBattleType( void )
{
    return (MsgCombat::BATTLE_TYPE)type;
}


MsgType MsgCombatAction::mType("CmAc");

MsgBase* MsgCombatAction::createInstance()
{
	return new MsgCombatAction;
}

void MsgCombatAction::serializeData()
{
	write(target);
	write(action);
	write(sp_action_type);
	write(sp_action);
}

void MsgCombatAction::deserializeData()
{
	read(target);
	read(action);
	write(sp_action_type);
	write(sp_action);
}

MsgCombatAction::BATTLE_ACTION MsgCombatAction::getAction( void )
{
    return (MsgCombatAction::BATTLE_ACTION)action;
}

MsgCombatAction::SPECIAL_ACTION MsgCombatAction::getSpecialAction( void )
{
    return (MsgCombatAction::SPECIAL_ACTION)sp_action_type;
}


MsgType MsgCombatResult::mType("CmRs");

MsgBase* MsgCombatResult::createInstance()
{
	return new MsgCombatResult;
}

void MsgCombatResult::serializeData()
{
	write(target);
	write(damage);
	write(result);
}

void MsgCombatResult::deserializeData()
{
	read(target);
	read(damage);
	read(result);
}

MsgCombatResult::BATTLE_RESULT MsgCombatResult::getResult( void )
{
    return (MsgCombatResult::BATTLE_RESULT)result;
}


//------------------------------------------------------------
// NPC Dialog
//------------------------------------------------------------
MsgType MsgNPCDialog::mType("NPCD");

MsgBase* MsgNPCDialog::createInstance()
{
	return new MsgNPCDialog;
}

void MsgNPCDialog::serializeData()
{
	string optText;
	uint32_t optID;
	uint32_t listSize = options.size();
	write(text);
	write(target);
	write(done);
	write(listSize);
	for (size_t i = 0; i < listSize; ++i) {
		NPCDialogOption option = options[i];
		optID = option.getID();
		optText = option.getText();
		write(optID);
		write(optText);
	}
}

void MsgNPCDialog::deserializeData()
{
	uint32_t listSize;
	read(text);
	read(target);
	read(done);
	read(listSize);
	for (size_t i = 0; i < listSize; ++i) {
		uint32_t optID;
		string optText;
		read(optID);
		read(optText);
		options.push_back(NPCDialogOption(optID, optText));
	}
}


void MsgNPCDialog::addOption(NPCDialogOption* option)
{
	options.push_back(*option);
}


MsgType MsgNPCDialogReply::mType("NPDR");

MsgBase* MsgNPCDialogReply::createInstance()
{
	return new MsgNPCDialogReply;
}

void MsgNPCDialogReply::serializeData()
{
	write(origin);
	write(target);
	write(done);
	write(option);
}

void MsgNPCDialogReply::deserializeData()
{
	read(origin);
	read(target);
	read(done);
	read(option);
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
