/*
 * cltceguidrawable.cpp
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
#include "client/cltconfig.h"

#include <CEGUI/System.h>
#include <CEGUI/Window.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>

#include <osg/Geode>
#include <osg/Projection>
#include <osg/Matrix>
#include <osg/MatrixTransform>

#include "cltceguimgr.h"
#include "cltviewer.h"

#include "cltceguidrawable.h"


//--------------------------- OSG2CEGUIKeys -----------------------------
CEGUI::Key::Scan OSG2CEGUIKeys(int key)
{
	CEGUI::Key::Scan converted = CEGUI::Key::Scan(0);
	switch (key)
	{
	case osgGA::GUIEventAdapter::KEY_Escape:
		converted = CEGUI::Key::Escape; break;
	case osgGA::GUIEventAdapter::KEY_F1:
		converted = CEGUI::Key::F1; break;
	case osgGA::GUIEventAdapter::KEY_F2:
		converted = CEGUI::Key::F2; break;
	case osgGA::GUIEventAdapter::KEY_F3:
		converted = CEGUI::Key::F3; break;
	case osgGA::GUIEventAdapter::KEY_F4:
		converted = CEGUI::Key::F4; break;
	case osgGA::GUIEventAdapter::KEY_F5:
		converted = CEGUI::Key::F5; break;
	case osgGA::GUIEventAdapter::KEY_F6:
		converted = CEGUI::Key::F6; break;
	case osgGA::GUIEventAdapter::KEY_F7:
		converted = CEGUI::Key::F7; break;
	case osgGA::GUIEventAdapter::KEY_F8:
		converted = CEGUI::Key::F8; break;
	case osgGA::GUIEventAdapter::KEY_F9:
		converted = CEGUI::Key::F9; break;
	case osgGA::GUIEventAdapter::KEY_F10:
		converted = CEGUI::Key::F10; break;
	case osgGA::GUIEventAdapter::KEY_F11:
		converted = CEGUI::Key::F11; break;
	case osgGA::GUIEventAdapter::KEY_F12:
		converted = CEGUI::Key::F12; break;
	case '1':
		converted = CEGUI::Key::One; break;
	case '2':
		converted = CEGUI::Key::Two; break;
	case '3':
		converted = CEGUI::Key::Three; break;
	case '4':
		converted = CEGUI::Key::Four; break;
	case '5':
		converted = CEGUI::Key::Five; break;
	case '6':
		converted = CEGUI::Key::Six; break;
	case '7':
		converted = CEGUI::Key::Seven; break;
	case '8':
		converted = CEGUI::Key::Eight; break;
	case '9':
		converted = CEGUI::Key::Nine; break;
	case '0':
		converted = CEGUI::Key::Zero; break;
	case '-':
		converted = CEGUI::Key::Minus; break;
	case '=':
		converted = CEGUI::Key::Equals; break;
	case osgGA::GUIEventAdapter::KEY_BackSpace:
		converted = CEGUI::Key::Backspace; break;
	case osgGA::GUIEventAdapter::KEY_Tab:
		converted = CEGUI::Key::Tab; break;
	case 'A':
		converted = CEGUI::Key::A; break;
	case 'B':
		converted = CEGUI::Key::B; break;
	case 'C':
		converted = CEGUI::Key::C; break;
	case 'D':
		converted = CEGUI::Key::D; break;
	case 'E':
		converted = CEGUI::Key::E; break;
	case 'F':
		converted = CEGUI::Key::F; break;
	case 'G':
		converted = CEGUI::Key::G; break;
	case 'H':
		converted = CEGUI::Key::H; break;
	case 'I':
		converted = CEGUI::Key::I; break;
	case 'J':
		converted = CEGUI::Key::J; break;
	case 'K':
		converted = CEGUI::Key::K; break;
	case 'L':
		converted = CEGUI::Key::L; break;
	case 'M':
		converted = CEGUI::Key::M; break;
	case 'N':
		converted = CEGUI::Key::N; break;
	case 'O':
		converted = CEGUI::Key::O; break;
	case 'P':
		converted = CEGUI::Key::P; break;
	case 'Q':
		converted = CEGUI::Key::Q; break;
	case 'R':
		converted = CEGUI::Key::R; break;
	case 'S':
		converted = CEGUI::Key::S; break;
	case 'T':
		converted = CEGUI::Key::T; break;
	case 'U':
		converted = CEGUI::Key::U; break;
	case 'V':
		converted = CEGUI::Key::V; break;
	case 'W':
		converted = CEGUI::Key::W; break;
	case 'X':
		converted = CEGUI::Key::X; break;
	case 'Y':
		converted = CEGUI::Key::Y; break;
	case 'Z':
		converted = CEGUI::Key::Z; break;
	case '[':
		converted = CEGUI::Key::LeftBracket; break;
	case ']':
		converted = CEGUI::Key::RightBracket; break;
	case '\\':
		converted = CEGUI::Key::Backslash; break;
	case osgGA::GUIEventAdapter::KEY_Caps_Lock:
		converted = CEGUI::Key::Capital; break;
	case ';':
		converted = CEGUI::Key::Semicolon; break;
	case '\'':
		converted = CEGUI::Key::Apostrophe; break;
	case osgGA::GUIEventAdapter::KEY_Return:
		converted = CEGUI::Key::Return; break;
	case osgGA::GUIEventAdapter::KEY_Shift_L:
		converted = CEGUI::Key::LeftShift; break;
	case ',':
		converted = CEGUI::Key::Comma; break;
	case '.':
		converted = CEGUI::Key::Period; break;
	case '/':
		converted = CEGUI::Key::Slash; break;
	case osgGA::GUIEventAdapter::KEY_Shift_R:
		converted = CEGUI::Key::RightShift; break;
	case osgGA::GUIEventAdapter::KEY_Control_L:
		converted = CEGUI::Key::LeftControl; break;
	case osgGA::GUIEventAdapter::KEY_Super_L:
		converted = CEGUI::Key::Scan(0); break;
	case osgGA::GUIEventAdapter::KEY_Space:
		converted = CEGUI::Key::Space; break;
	case osgGA::GUIEventAdapter::KEY_Alt_L:
		converted = CEGUI::Key::LeftAlt; break;
	case osgGA::GUIEventAdapter::KEY_Alt_R:
		converted = CEGUI::Key::RightAlt; break;
	case osgGA::GUIEventAdapter::KEY_Super_R:
		converted = CEGUI::Key::Scan(0); break;
	case osgGA::GUIEventAdapter::KEY_Menu:
		converted = CEGUI::Key::Scan(0); break;
	case osgGA::GUIEventAdapter::KEY_Control_R:
		converted = CEGUI::Key::RightControl; break;
	case osgGA::GUIEventAdapter::KEY_Print:
		converted = CEGUI::Key::SysRq; break;
	case osgGA::GUIEventAdapter::KEY_Scroll_Lock:
		converted = CEGUI::Key::ScrollLock; break;
	case osgGA::GUIEventAdapter::KEY_Pause:
		converted = CEGUI::Key::Pause; break;
	case osgGA::GUIEventAdapter::KEY_Home:
		converted = CEGUI::Key::Home; break;
	case osgGA::GUIEventAdapter::KEY_Page_Up:
		converted = CEGUI::Key::PageUp; break;
	case osgGA::GUIEventAdapter::KEY_End:
		converted = CEGUI::Key::End; break;
	case osgGA::GUIEventAdapter::KEY_Page_Down:
		converted = CEGUI::Key::PageDown; break;
	case osgGA::GUIEventAdapter::KEY_Delete:
		converted = CEGUI::Key::Delete; break;
	case osgGA::GUIEventAdapter::KEY_Insert:
		converted = CEGUI::Key::Insert; break;
	case osgGA::GUIEventAdapter::KEY_Left:
		converted = CEGUI::Key::ArrowLeft; break;
	case osgGA::GUIEventAdapter::KEY_Up:
		converted = CEGUI::Key::ArrowUp; break;
	case osgGA::GUIEventAdapter::KEY_Right:
		converted = CEGUI::Key::ArrowRight; break;
	case osgGA::GUIEventAdapter::KEY_Down:
		converted = CEGUI::Key::ArrowDown; break;
	case osgGA::GUIEventAdapter::KEY_Num_Lock:
		converted = CEGUI::Key::NumLock; break;
	case osgGA::GUIEventAdapter::KEY_KP_Divide:
		converted = CEGUI::Key::Divide; break;
	case osgGA::GUIEventAdapter::KEY_KP_Multiply:
		converted = CEGUI::Key::Multiply; break;
	case osgGA::GUIEventAdapter::KEY_KP_Subtract:
		converted = CEGUI::Key::Subtract; break;
	case osgGA::GUIEventAdapter::KEY_KP_Add:
		converted = CEGUI::Key::Add; break;
	case osgGA::GUIEventAdapter::KEY_KP_Home:
		converted = CEGUI::Key::Numpad7; break;
	case osgGA::GUIEventAdapter::KEY_KP_Up:
		converted = CEGUI::Key::Numpad8; break;
	case osgGA::GUIEventAdapter::KEY_KP_Page_Up:
		converted = CEGUI::Key::Numpad9; break;
	case osgGA::GUIEventAdapter::KEY_KP_Left:
		converted = CEGUI::Key::Numpad4; break;
	case osgGA::GUIEventAdapter::KEY_KP_Begin:
		converted = CEGUI::Key::Scan(0); break;
	case osgGA::GUIEventAdapter::KEY_KP_Right:
		converted = CEGUI::Key::Numpad6; break;
	case osgGA::GUIEventAdapter::KEY_KP_End:
		converted = CEGUI::Key::Numpad1; break;
	case osgGA::GUIEventAdapter::KEY_KP_Down:
		converted = CEGUI::Key::Numpad2; break;
	case osgGA::GUIEventAdapter::KEY_KP_Page_Down:
		converted = CEGUI::Key::Numpad3; break;
	case osgGA::GUIEventAdapter::KEY_KP_Insert:
		converted = CEGUI::Key::Numpad0; break;
	case osgGA::GUIEventAdapter::KEY_KP_Delete:
		converted = CEGUI::Key::Decimal; break;
	case osgGA::GUIEventAdapter::KEY_KP_Enter:
		converted = CEGUI::Key::NumpadEnter; break;
	default:
		converted = CEGUI::Key::Scan(0); break;
	}

	return converted;
}

//--------------------------- CltCEGUIEventHandler ----------------------
CltCEGUIEventHandler::CltCEGUIEventHandler() :
	mClassName("CEGUIEventHandler")
{
}

const char* CltCEGUIEventHandler::className() const
{
	return mClassName.c_str();
}

bool CltCEGUIEventHandler::handle(const osgGA::GUIEventAdapter& ea,
				  osgGA::GUIActionAdapter& aa,
				  osg::Object* /* o */,
				  osg::NodeVisitor* /* nv */)
{
	return handle(ea, aa);
}

bool CltCEGUIEventHandler::handle(const osgGA::GUIEventAdapter& ea,
				  osgGA::GUIActionAdapter& /* aa */)
{
	bool catched = false;

	switch(ea.getEventType())
        {
	case(osgGA::GUIEventAdapter::DRAG):
	case(osgGA::GUIEventAdapter::MOVE):
	{
		// coordinates conversion
		int x = static_cast<int>((1.0f + ea.getX()) * CltViewer::instance().getWindowWidth()/2);
		int y = static_cast<int>((1.0f - ea.getY()) * CltViewer::instance().getWindowHeight()/2);
		catched = CEGUI::System::getSingleton().getDefaultGUIContext().injectMousePosition(x, y);

		return catched;
	}
	case(osgGA::GUIEventAdapter::PUSH):
	{
                if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			catched = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::LeftButton);
                else if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
			catched = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::MiddleButton);
                else if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
			catched = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::RightButton);

		if (!catched) {
			LogDBG("CEGUI -- focus disabled");
			CltCEGUIMgr::instance().disableFocus();
		}

		return catched;
	}
	case(osgGA::GUIEventAdapter::RELEASE):
	{
                if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			catched = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(CEGUI::LeftButton);
                else if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
			catched = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(CEGUI::MiddleButton);
                else if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
			catched = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(CEGUI::RightButton);

		return catched;
	}
	case(osgGA::GUIEventAdapter::DOUBLECLICK):
	{
                if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			catched = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::LeftButton);
                else if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
			catched = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::MiddleButton);
                else if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
			catched = CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::RightButton);

		return catched;
	}
	case(osgGA::GUIEventAdapter::KEYDOWN):
	{
		if (CEGUI::System::getSingleton().getDefaultGUIContext().injectChar(static_cast<CEGUI::utf32>(ea.getKey()))) {
			return true;
		} else if (CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyDown(OSG2CEGUIKeys(ea.getKey()))) {
			return true;
		} else {
			return false;
		}
	}
	case(osgGA::GUIEventAdapter::KEYUP):
	{
		return CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp(OSG2CEGUIKeys(ea.getKey()));
	}
	case(osgGA::GUIEventAdapter::FRAME):
	{
		static double lastTime = 0.0;
		CEGUI::System::getSingletonPtr()->injectTimePulse(ea.time() - lastTime);
		lastTime = ea.time();
		return false;
	}
	default:
		return false;
                break;
        }

	return false;
}


//------------------------------- CltCEGUIDrawable ---------------------
CltCEGUIDrawable::CltCEGUIDrawable(unsigned int windowWidth, unsigned int windowHeight) :
	mWindowWidth(windowWidth), mWindowHeight(windowHeight)
{
	// basic setup of this drawable
	setSupportsDisplayList(false);
	setUseDisplayList(false);

	// projection: occupying the full screen
	mTransform = new osg::MatrixTransform(osg::Matrix::identity());
	mTransform->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	mProjection = new osg::Projection(osg::Matrix::ortho2D(0, mWindowWidth, 0, mWindowHeight));
	mProjection->addChild(mTransform);

	// setting the rendering properties of the node
	osg::Geode* geode = new osg::Geode();
	mTransform->addChild(geode);
	geode->addDrawable(this);
	osg::StateSet* stateset = geode->getOrCreateStateSet();
	stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
	stateset->setRenderBinDetails(11, "RenderBin");

	// setup CEGUI (load data files, etc)
	CltCEGUIMgr::instance().setup(mWindowWidth, mWindowHeight);

	// setup the event handler, to render the GUI every frame and capture
	// other events
	//
	// mafm: note that this cannot be a callback as it would normally be,
	// because OSG doesn't seem to treat it as an event handler when being a
	// callback, so even when the event catches an event ("return true"),
	// OSG ignores it and propagates the event to other parts of the engine,
	// so the keys for player movement continue as wel... So we handle this
	// externally.
}

CltCEGUIDrawable::~CltCEGUIDrawable()
{
}

osg::Node* CltCEGUIDrawable::getNode() const
{
	return mProjection;
}

void CltCEGUIDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
/* mafm: apparently not needed
	unsigned int oldClientActiveTextureUnit = renderInfo.getState()->getClientActiveTextureUnit();
        renderInfo.getState()->setClientActiveTextureUnit(0);
	glDisable(GL_TEXTURE_2D);
	renderInfo.getState()->setActiveTextureUnit(0);
	glEnable(GL_TEXTURE_2D);
*/

	// tell the UI to update and to render
	CEGUI::System::getSingleton().renderAllGUIContexts();
	renderInfo.getState()->checkGLErrors("CEGUIDrawable::drawImplementation");
/* mafm: apparently not needed
        renderInfo.getState()->setClientActiveTextureUnit(oldClientActiveTextureUnit);
        renderInfo.getState()->setActiveTextureUnit(oldClientActiveTextureUnit);
*/
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
