/*
 * cltcontentloader.h
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

#ifndef __FEARANN_CLIENT_CONTENT_LOADER_H__
#define __FEARANN_CLIENT_CONTENT_LOADER_H__


#include "common/patterns/singleton.h"

#include <map>


namespace osg {
	class Node;
}
namespace osgCal {
	class CoreModel;
}
class XMLNode;


/** Class to hold information about classes of entities (characters, creatures,
 * tools, ...)
 */
class ContentInfoBase
{
	friend class CltContentLoader;

protected:
	string name;
	string prettyName;
	string description;
	string entClass;
	bool equipable;

public:
	ContentInfoBase(const char* n, const char* c) :
		name(n), entClass(c), equipable(false)
	{ }

	const char* getName() const { return name.c_str(); }
	const char* getPrettyName() const { return prettyName.c_str(); }
	const char* getDescription() const { return description.c_str(); }
	const char* getEntityClass() const { return entClass.c_str(); }
	bool isEquipable() const { return equipable; }
};


/** Class to hold game-related information about characters (race traits, etc).
 */
class ContentInfoCharacter : public ContentInfoBase
{
	friend class CltContentLoader;

public:
	ContentInfoCharacter(const char* _name) :
		ContentInfoBase(_name, "Character"),
		bonus_str(-1), bonus_con(-1), bonus_dex(-1), bonus_int(-1), bonus_wis(-1), bonus_cha(-1)
	{ }

	int getBonusStr() const { return bonus_str; }
	int getBonusCon() const { return bonus_con; }
	int getBonusDex() const { return bonus_dex; }
	int getBonusInt() const { return bonus_int; }
	int getBonusWis() const { return bonus_wis; }
	int getBonusCha() const { return bonus_cha; }

protected:
	int bonus_str, bonus_con, bonus_dex, bonus_int, bonus_wis, bonus_cha;
};


/** Class to hold information about equipable items, to be loaded and perform
 * some global operations.
 */
class ContentInfoEquipable : public ContentInfoBase
{
	friend class CltContentLoader;

public:
	ContentInfoEquipable(const char* _name, const char* _class) : ContentInfoBase(_name, _class)
	{
		load = -1.0f;
		price = -1;
		handed = -1;

		// special for equipable objects
		equipable = true;
	}

	float getLoad() const { return load; }
	int getPrice() const { return price; }
	int getHanded() const { return handed; }

protected:
	float load;
	int price, handed;
};


/** This class abstracts the functions related with loading content into the
 * engine.
 */
class CltContentLoader : public Singleton<CltContentLoader>
{
public:
	/** Check whether the config files are already loaded */
	bool isConfigLoaded();
	/** Load the config files of everything */
	bool loadCfgFiles();

	/** Get the structure representing a generic item */
	const ContentInfoBase* getInfoItem(const std::string& key);
	/** Get the structure representing a character */
	const ContentInfoCharacter* getInfoCharacter(const std::string& key);

	/** Load cal3d model according with the entity type */
	osgCal::CoreModel* loadCal3DCoreModel(const std::string& meshType, const std::string& meshSubtype);
	/** Load model according with the entity type */
	osg::Node* loadModel(const std::string& meshType, const std::string& meshSubtype);
	/** Load the given area */
	void loadArea(const std::string& name, osg::Node** terrain, osg::Node** buildings);

private:
	/** Singleton friend access */
	friend class Singleton<CltContentLoader>;

	/// Structure to hold the elements read in game configuration files,
	/// all-items list
	map<std::string, ContentInfoBase*> mBaseInfoList;
	/// Structure to hold the elements read in game configuration files,
	/// characters
	map<std::string, ContentInfoCharacter*> mCharacterInfoList;

	/** Default constructor */
	CltContentLoader();
	/** Default destructor */
	~CltContentLoader();


	/** Load an XML file containing configurations of given types
	 * (characters, areas, tools, ...) and return the root node (0 if there
	 * was an error) */
	const XMLNode* loadDocumentRoot(const char* document);
	/** Load the config file */
	bool loadCfgCharacters();
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
