/*
 * cltceguidrawable.h
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

#ifndef __FEARANN_CLIENT_CEGUI_DRAWABLE_H__
#define __FEARANN_CLIENT_CEGUI_DRAWABLE_H__

#include <osg/CopyOp>
#include <osg/Drawable>
#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/Object>
#include <osg/Projection>
#include <osgGA/GUIEventHandler>


/** Event handler to capture events and inject them into CEGUI
 */
class CltCEGUIEventHandler : public osgGA::GUIEventHandler 
{
public:
	/** Default constructor */
	CltCEGUIEventHandler();

	/** Class name of this handler */
	virtual const char* className() const;
	/** Handle the events */
	virtual bool handle(const osgGA::GUIEventAdapter& ea,
			    osgGA::GUIActionAdapter& aa,
			    osg::Object* o,
			    osg::NodeVisitor* nv);
	/** Handle the events */
	virtual bool handle(const osgGA::GUIEventAdapter& ea,
			    osgGA::GUIActionAdapter& aa);

private:
	/// Name that will be used as identifier
	std::string mClassName;
};


/** Class to encapsulate CEGUI inside OSG, receiving the input and rendering
 * itself in every frame.  This has to be added to the scene, as any other node.
 *
 * \remark This class must be instantiated *after* the application has created a
 * valid OpenGL context and the window -- Viewer.realize() does this.
 */
class CltCEGUIDrawable : public osg::Drawable
{
public:
	/** Default constructor, with the window dimensions as parameters. */
	CltCEGUIDrawable(unsigned int windowWidth, unsigned int windowHeight);
	/** Default destructor */
	virtual ~CltCEGUIDrawable();

	/** osg::Drawable interface */
	CltCEGUIDrawable(const CltCEGUIDrawable& drawable, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) :
		osg::Drawable(drawable, copyop)
	{ }
	/** osg::Drawable interface */
	virtual osg::Object* cloneType() const
	{ return new CltCEGUIDrawable(mWindowWidth, mWindowHeight); }
	/** osg::Drawable interface */
	virtual osg::Object* clone(const osg::CopyOp& copyop) const
	{ return new CltCEGUIDrawable(*this, copyop); }
	/** osg::Drawable interface */
	virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

	/** Get node (to be added to the scene) */
	osg::Node* getNode() const;

private:
	/// Projection in the viewport
	osg::Projection* mProjection;
	/// Matrix transformation for the drawable
	osg::MatrixTransform* mTransform;

	/// Window dimension/resolution
	mutable unsigned int mWindowWidth;
	/// Window dimension/resolution
	mutable unsigned int mWindowHeight;
	/// GL context for CEGUI
	mutable unsigned int mActiveContextID;
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
