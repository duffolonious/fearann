/*
 * xmlmgr.h
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

#ifndef __FEARANN_COMMON_XML_MGR_H__
#define __FEARANN_COMMON_XML_MGR_H__


#include "common/patterns/singleton.h"

#include <xercesc/util/XercesDefs.hpp>
#if (XERCES_VERSION_MAJOR > 2)
#include <xercesc/dom/DOMLSParser.hpp>
#endif

#include <vector>


namespace XERCES_CPP_NAMESPACE {
	class DOMBuilder;
	class DOMErrorHandler;
	class DOMNode;
}


/** Wrapper around the node of the library, so our application doesn't need to
 * include the library headers when reading files.
 */
class XMLNode
{
public:
	/** Constructor with the given node as parameter */
	XMLNode(XERCES_CPP_NAMESPACE::DOMNode* node);

	/** Get the name of this node */
	const char* getName() const;

	/** Get the first child node (0 if no children).
	 *
	 * \remark Note that the first child according with W3C DOM definition
	 * is to get as child node the blanks between them.  Don't know what
	 * they've been smoking when writing this, but we just skip it until we
	 * find a proper child node. */
	const XMLNode* getFirstChild() const;
	/** Get child node with the givven name, returns 0 if not found. */
	const XMLNode* getChildWithName(const std::string& name) const;
	/** Get the next sibling node (0 if no more) */
	const XMLNode* getNextSibling() const;
	/** Get child node at index */
	const XMLNode* getChildAt(size_t index) const;
	/** returns size of child node list */
	int getChildListSize() const;

	/** Get node attribute at index (0 through length-1) */
	std::string getAttrNameAt(uint32_t index) const;
	/** Get node attribute length */
	uint32_t getAttributesLength() const;

	/** Return value of attribute named attrName */
	std::string getAttrValueAsStr(const char* attrName) const;
	/** Return value of attribute named attrName */
	int getAttrValueAsInt(const char* attrName) const;
	/** Return value of attribute named attrName */
	float getAttrValueAsFloat(const char* attrName) const;

private:
	/// The XERCES node
	XERCES_CPP_NAMESPACE::DOMNode* mDOMNode;
};


/** This class reads (doesn't write!) xml files
 */
class XMLMgr : public Singleton<XMLMgr>
{
public:
	/** Load the given file, return root node.  The file remains loaded and
	 * active until the clear() method is called. */
	const XMLNode* loadXMLFile(const char* file);
	/** Unload all files loaded */
	void clear();

private:
	/** Singleton friend access */
	friend class Singleton<XMLMgr>;

	/// Link to the parser
#if (XERCES_VERSION_MAJOR > 2)
	XERCES_CPP_NAMESPACE::DOMLSParser* mParser;
#else
	XERCES_CPP_NAMESPACE::DOMBuilder* mParser;
	/// Error handler
	XERCES_CPP_NAMESPACE::DOMErrorHandler* mErrorHandler;
#endif

	/** Default constructor */
	XMLMgr();
	/** Destructor */
	~XMLMgr();
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
