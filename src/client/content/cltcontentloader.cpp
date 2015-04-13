/*
 * cltcontentloader.cpp
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
#include "client/cltconfig.h"

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgCal/CoreModel>

#include "common/xmlmgr.h"

#include "cltcontentloader.h"


//------------------------- CltContentLoader ---------------------------
template <> CltContentLoader* Singleton<CltContentLoader>::INSTANCE = 0;

CltContentLoader::CltContentLoader()
{
}

CltContentLoader::~CltContentLoader()
{
        for (map<string, ContentInfoBase*>::iterator it = mBaseInfoList.begin(); it != mBaseInfoList.end(); ++it) {
		delete it->second;
        }
}

bool CltContentLoader::isConfigLoaded()
{
	// quick test, unless we have a very broken setup we can assume that
	// this is safe
	if (mBaseInfoList.size() > 0)
		return true;
	else
		return false;
}

bool CltContentLoader::loadCfgFiles()
{
	if (isConfigLoaded())
		return false;

	bool success = true;
	success &= loadCfgCharacters();

	if (success)
		LogNTC("Configuration files loaded succesfully");
	else
		LogERR("Configuration files not loaded!!");

	// clear the XML pool (loaded files), since we shouldn't need them from
	// now on -- we have the structures loaded for that
	XMLMgr::instance().clear();

	return success;
}

const XMLNode* CltContentLoader::loadDocumentRoot(const char* document)
{
	string docFile = string(CONTENT_DIR) + string("/cfg/")
		+ string(document) + ".xml";

	const XMLNode* rootNode = XMLMgr::instance().loadXMLFile(docFile.c_str());
	if (!rootNode) {
		LogWRN("Game config file '%s' could not be loaded", docFile.c_str());
	}
	return rootNode;
}

bool CltContentLoader::loadCfgCharacters()
{
	const XMLNode* rootXMLNode = loadDocumentRoot("characters");
	if (!rootXMLNode)
		return false;

	const XMLNode* childXMLNode = rootXMLNode->getFirstChild();
        while (childXMLNode) {
		LogDBG("Reading character config: '%s'", childXMLNode->getName());
		ContentInfoCharacter* node = new ContentInfoCharacter(childXMLNode->getName());

		// adding this node to the structure
		mCharacterInfoList[node->name.c_str()] = node;
		mBaseInfoList[node->name.c_str()] = node;

		// attributes (pretty name, description, etc)
		node->prettyName = childXMLNode->getAttrValueAsStr("prettyName");

		// race bonus
		const XMLNode* bonus = childXMLNode->getChildWithName(string("bonus"));
		if (!bonus)
			return false;
		node->bonus_str = bonus->getAttrValueAsInt("str");
		node->bonus_con = bonus->getAttrValueAsInt("con");
		node->bonus_dex = bonus->getAttrValueAsInt("dex");
		node->bonus_int = bonus->getAttrValueAsInt("int");
		node->bonus_wis = bonus->getAttrValueAsInt("wis");
		node->bonus_cha = bonus->getAttrValueAsInt("cha");
		delete bonus;

		const XMLNode* aux = childXMLNode;
		childXMLNode = childXMLNode->getNextSibling();
		delete aux;
	}
	delete rootXMLNode;

	return true;
}

osgCal::CoreModel* CltContentLoader::loadCal3DCoreModel(const string& meshType, const string& meshSubtype)
{
	// directory where models live
	string cal3dDir(StrFmt("%s/models/cal3d/%s_%s", CONTENT_DIR, meshType.c_str(), meshSubtype.c_str()));
	osgDB::FileType fileType = osgDB::fileType(cal3dDir.c_str());
	if (fileType != osgDB::DIRECTORY) {
		LogERR("Character directory '%s' doesn't exist or is not a directory: %d",
		       cal3dDir.c_str(), fileType);
		return 0;
	}

	// load cal3d model
	string modelFile(StrFmt("%s/%s_%s.cfg", cal3dDir.c_str(), meshType.c_str(), meshSubtype.c_str()));
	osgCal::CoreModel* coreModel(new osgCal::CoreModel());
	osgCal::MeshParameters* param(new osgCal::MeshParameters);
	coreModel->load(modelFile, param);
	PERM_ASSERT(coreModel);

	// scale, if needed
	//coreModel->getCalCoreModel()->scale(0.0087f);

	return coreModel;
}

osg::Node* CltContentLoader::loadModel(const string& meshType, const string& meshSubtype)
{
	// directory where models live
	string modelDir(StrFmt("%s/models/objects", CONTENT_DIR));
	osgDB::FileType fileType = osgDB::fileType(modelDir.c_str());
	if (fileType != osgDB::DIRECTORY) {
		LogERR("Character directory '%s' doesn't exist or is not a directory: %d",
		       modelDir.c_str(), fileType);
		return 0;
	}

	// load model
	string modelFile(StrFmt("%s/%s_%s.osg", modelDir.c_str(), meshType.c_str(), meshSubtype.c_str()));
	osg::Node* model = dynamic_cast<osg::Node*>(osgDB::readObjectFile(modelFile.c_str()));
	PERM_ASSERT(model);

	// scale, necessary?

	return model;
}

void CltContentLoader::loadArea(const string& name, osg::Node** terrain, osg::Node** buildings)
{
	// get the directory
	string areaDir(StrFmt("%s/areas/%s/", CONTENT_DIR, name.c_str()));
	osgDB::FileType fileType = osgDB::fileType(areaDir.c_str());
	if (fileType != osgDB::DIRECTORY) {
		LogERR("Area directory '%s' doesn't exist or is not a directory: %d",
		       areaDir.c_str(), fileType);
		return;
	}

	// load the terrain
	string terrainFile(areaDir + "terrain.osg");
	*terrain = osgDB::readNodeFile(terrainFile.c_str());
	if (!terrain) {
		LogERR("Couldn't load terrain '%s'", terrainFile.c_str());
	}

	// load the buildings
	string buildingsFile(areaDir + "buildings.osg");
	*buildings = osgDB::readNodeFile(buildingsFile.c_str());
	if (!buildings) {
		LogERR("Couldn't load buildings '%s'", buildingsFile.c_str());
	}

	LogNTC("Area '%s' loaded successfully", name.c_str());
}

const ContentInfoBase* CltContentLoader::getInfoItem(const string& key)
{
	// sanity check
	if (!isConfigLoaded()) {
		LogERR("Config not loaded, ignoring request to retrieve '%s'", key.c_str());
		return 0;
	}

	// normalize to lowercase
	string fmtKey;
	for (size_t i = 0; i < key.length(); ++i) {
		fmtKey.push_back(tolower(key[i]));
	}

	// find and return the element with given key
	map<string, ContentInfoBase*>::iterator it = mBaseInfoList.find(fmtKey);
	if (it == mBaseInfoList.end()) {
		LogERR("Can't find item '%s'", fmtKey.c_str());
		return 0;
	} else {
		return (*it).second;
	}
}

const ContentInfoCharacter* CltContentLoader::getInfoCharacter(const string& key)
{
	return static_cast<const ContentInfoCharacter*>(getInfoItem(key));
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
