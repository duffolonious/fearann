/*
 * cltceguiminimap.h
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

#ifndef __FEARANN_CLIENT_CEGUI_MINIMAP_H__
#define __FEARANN_CLIENT_CEGUI_MINIMAP_H__


#include "common/patterns/singleton.h"


namespace CEGUI {
	class ImageSetManager;
	class WindowManager;
}
class CltMinimapDrawable;


/** Class to manage the minimap image in the corner of the screen.
 */
class CltCEGUIMinimap : public Singleton<CltCEGUIMinimap>
{
public:
	/** Set the image to use for the map */
	void setMapImage(const char* areaName);
	/** Update the minimap with the given coordinates */
	void updateMinimap(int x, int z);

	/** Change the zoom level (absolute value) */
	void setZoomLevel(int level);
	/** Increase the zoom level (won't do anything when max level
	 * reached) */
	void increaseZoomLevel();
	/** Decrease the zoom level (won't do anything when max level
	 * reached) */
	void decreaseZoomLevel();

private:
	/** Singleton friend access */
	friend class Singleton<CltCEGUIMinimap>;

	/// Factor value to use different zooms
	int mZoomLevel;
	/// Saving these values, needed at least to update when changin zoom
	int mLastX, mLastY;
	/// Drawable node to be added to the scene
	CltMinimapDrawable* mMinimapDrawable;

	/** Default constructor */
	CltCEGUIMinimap();
	/** Destructor */
	~CltCEGUIMinimap();
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
