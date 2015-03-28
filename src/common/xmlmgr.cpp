/*
 * xmlmgr.cpp
 * Copyright (C) 2006-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *			      Bryan Duff <duff0097@umn.edu>
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

#include "xmlmgr.h"

#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#if (XERCES_VERSION_MAJOR > 2)
#include <xercesc/dom/DOM.hpp>
#else
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#endif

#include <cstring>
#include <cstdlib>
#include <fstream>


XERCES_CPP_NAMESPACE_USE

//----------------------- utf16 -> ascii conversion function ---------------
const char* convertUTF16(const XMLCh* str)
{
	static string stringHolder;
	char* tmp = XMLString::transcode(str);
	if (!tmp) {
		LogWRN("Couldn't convert incoming XML string to char*");
		return 0;
	} else {
		stringHolder.clear();
		stringHolder.append(tmp);
		XMLString::release(&tmp);
		return stringHolder.c_str();
	}
}

#if (XERCES_VERSION_MAJOR <= 2)
//----------------------- DOMDummyErrorHandler ----------------------------
class DOMDummyErrorHandler : public DOMErrorHandler
{
	virtual bool handleError(const DOMError&) {
		return false;
	}
};
#endif

//----------------------- XMLNode ----------------------------
XMLNode::XMLNode(DOMNode* node) :
	mDOMNode(node)
{
	PERM_ASSERT(mDOMNode);
}

const char* XMLNode::getName() const
{
	return convertUTF16(mDOMNode->getNodeName());
}

const XMLNode* XMLNode::getFirstChild() const
{
	// skip #text nodes
	for (size_t i = 0; i < mDOMNode->getChildNodes()->getLength(); ++i) {
		string childName = convertUTF16(mDOMNode->getChildNodes()->item(i)->getNodeName());
		if (childName != "#text") {
			// LogDBG("Returning child: '%s'", childName.c_str());
			return new XMLNode(mDOMNode->getChildNodes()->item(i));
		}
	}

	LogDBG("No children found for node '%s' (all were #text nodes, blank space)",
	       getName());
	return 0;
}

const XMLNode* XMLNode::getChildWithName(const string& name) const
{
	// skip #text nodes
	for (size_t i = 0; i < mDOMNode->getChildNodes()->getLength(); ++i) {
		string childName = convertUTF16(mDOMNode->getChildNodes()->item(i)->getNodeName());
		if (childName == name) {
			// LogDBG("Returning child: '%s'", childName.c_str());
			return new XMLNode(mDOMNode->getChildNodes()->item(i));
		}
	}

	LogDBG("No children named '%s' found for node '%s'", name.c_str(), getName());
	return 0;
}

const XMLNode* XMLNode::getNextSibling() const
{
	DOMNode* child = mDOMNode->getNextSibling();
	while (child) {
		if (string("#text") == convertUTF16(child->getNodeName())) {
			// skip #text nodes
			child = child->getNextSibling();
			continue;
		} else {
			return new XMLNode(child);
		}
	}

	return 0;
}

const XMLNode* XMLNode::getChildAt(size_t index) const
{
	DOMNodeList* list = mDOMNode->getChildNodes();
	if (!list || list->item(index)->getNodeType() != DOMNode::ELEMENT_NODE) {
		return 0;
	} else {
		return new XMLNode(list->item(index));
	}
}

int XMLNode::getChildListSize() const
{
	DOMNodeList* list = mDOMNode->getChildNodes();
	return list->getLength();
}

string XMLNode::getAttrNameAt(uint32_t index) const
{
	if (mDOMNode->hasAttributes()) {
		// get all the attributes of the node
		DOMNamedNodeMap* attributes = mDOMNode->getAttributes();
		for (size_t i = 0; i < attributes->getLength(); ++i) {
			DOMAttr* attributeNode = static_cast<DOMAttr*>(attributes->item(i));
			if ( index == i ) {
				string name = convertUTF16(attributeNode->getName());
				//string value = convertUTF16(attributeNode->getValue());
				//LogDBG("Returning '%s'='%s'", name.c_str(), value.c_str() );
				return name;
			}
		}
	} else {
		LogDBG("Element '%s' doesn't have attributes, can't get attribute: %u",
		       convertUTF16(mDOMNode->getNodeName()), index);
		return string("");
	}

	LogDBG("Element '%s' has attributes, but couldn't get attribute: %u",
	       convertUTF16(mDOMNode->getNodeName()), index);
	return string("");
}

uint32_t XMLNode::getAttributesLength() const
{
	if (mDOMNode->hasAttributes()) {
		// get all the attributes of the node
		DOMNamedNodeMap* attributes = mDOMNode->getAttributes();
		return attributes->getLength();
	}
	return 0;
}

string XMLNode::getAttrValueAsStr(const char* attrName) const
{
	if (mDOMNode->hasAttributes()) {
		// get all the attributes of the node
		DOMNamedNodeMap* attributes = mDOMNode->getAttributes();
		for (size_t i = 0; i < attributes->getLength(); ++i) {
			DOMAttr* attributeNode = static_cast<DOMAttr*>(attributes->item(i));
			string name = convertUTF16(attributeNode->getName());
			//LogDBG("getAttributeAsStr '%s' =? '%s'", attrName, name.c_str() );
			if (name == attrName) {
				string value = convertUTF16(attributeNode->getValue());
				//LogDBG("Returning '%s'='%s'", name.c_str(), value.c_str() );
				return value;
			}
		}
	} else {
		LogWRN("Element '%s' doesn't have attributes, can't get attribute: %s",
		       convertUTF16(mDOMNode->getNodeName()), attrName);
		return string("");
	}

	LogWRN("Element '%s' has attributes, but couldn't get attribute: %s",
	       convertUTF16(mDOMNode->getNodeName()), attrName);
	return string("");
}

int XMLNode::getAttrValueAsInt(const char* attrName) const
{
	string value = getAttrValueAsStr(attrName);
	if (!value.empty()) {
		return atoi(value.c_str());
	} else {
		return 0;
	}
}

float XMLNode::getAttrValueAsFloat(const char* attrName) const
{
	string value = getAttrValueAsStr(attrName);
	if (!value.empty()) {
		return atof(value.c_str());
	} else {
		return 0.0f;
	}
}


//----------------------- XMLMgr ----------------------------
template <> XMLMgr* Singleton<XMLMgr>::INSTANCE = 0;

XMLMgr::XMLMgr()
{
	try {
		XMLPlatformUtils::Initialize();
	} catch (const XMLException& toCatch) {
		LogERR("XMLMgr: Error during initialization: '%s'",
		       convertUTF16(toCatch.getMessage()));
	}

	// Instantiate the DOM parser
	static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
	DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(gLS);
#if (XERCES_VERSION_MAJOR > 2)
	mParser = impl->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, NULL);
#else
	mParser = impl->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

	mParser->setFeature(XMLUni::fgDOMNamespaces, false);
	mParser->setFeature(XMLUni::fgXercesSchema, false);
	mParser->setFeature(XMLUni::fgXercesSchemaFullChecking, false);
	mParser->setFeature(XMLUni::fgDOMValidateIfSchema, true);

	// enable datatype normalization - default is off
	mParser->setFeature(XMLUni::fgDOMDatatypeNormalization, true);

	// And create our error handler and install it
	mErrorHandler = new DOMDummyErrorHandler();
	mParser->setErrorHandler(mErrorHandler);
#endif
}

XMLMgr::~XMLMgr()
{
	//LogDBG("Deleting XML manager");
	mParser->release();
#if (XERCES_VERSION_MAJOR <= 2)
	delete mErrorHandler;
#endif

	// call the termination method
	XMLPlatformUtils::Terminate();
}

const XMLNode* XMLMgr::loadXMLFile(const char* file)
{
	try {
		DOMDocument* doc = mParser->parseURI(file);
		if (!doc) {
			LogERR("XMLMgr::loadXMLFile: Document '%s' not loaded for unknown reason", file);
			// For some reason this can be (nil) with no exceptions
			// thrown
			return 0;
		}
		DOMNode* node = reinterpret_cast<DOMNode*>(doc->getDocumentElement());
		if (!node) {
			LogERR("XMLMgr::loadXMLFile: DOMNode for '%s' not created for unknown reason", file);
			return 0;
		}
		return new XMLNode(node);
	} catch (const XMLException& e) {
		LogERR("XMLMgr::loadXMLfile: Exception parsing '%s': %s", file, convertUTF16(e.getMessage()));
	} catch (const DOMException& e) {
		LogERR("XMLMgr::loadXMLfile: DOM Exception parsing '%s': code %d", file, e.code);
	} catch (...)  {
		LogERR("XMLMgr::loadXMLfile: Unexpected exception parsing '%s'", file);
	}
	return 0;
}

void XMLMgr::clear()
{
	mParser->resetDocumentPool();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
