
/*LICENSE_START*/
/* 
 *  Copyright 1995-2011 Washington University School of Medicine 
 * 
 *  http://brainmap.wustl.edu 
 * 
 *  This file is part of CARET. 
 * 
 *  CARET is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation; either version 2 of the License, or 
 *  (at your option) any later version. 
 * 
 *  CARET is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 * 
 *  You should have received a copy of the GNU General Public License 
 *  along with CARET; if not, write to the Free Software 
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 * 
 */ 

#define __OVERLAY_SET_DECLARE__
#include "OverlaySet.h"
#undef __OVERLAY_SET_DECLARE__

#include "CaretAssert.h"
#include "OverlaySet.h"

using namespace caret;

/**
 * \class OverlaySet
 * \brief Contains a set of overlay assignments
 *
 * The maximum number of overlays is fixed.  The number
 * of overlays presented to the user varies and is
 * controlled using the ToolBox in a Browser Window.
 * 
 * The primary overlay is always the overlay at index zero.
 * The underlay is the overlay at (numberOfDisplayedOverlays - 1).
 * When models are colored, the overlays are assigned 
 * starting with the underlay and concluding with the primary
 * overlay.
 */

/**
 * Constructor.
 */
OverlaySet::OverlaySet()
: CaretObject()
{
    this->numberOfDisplayedOverlays = BrainConstants::MINIMUM_NUMBER_OF_OVERLAYS;
}

/**
 * Destructor.
 */
OverlaySet::~OverlaySet()
{
    
}

/**
 * @return Returns the primary overlay.
 */
Overlay* 
OverlaySet::getPrimaryOverlay()
{
    return &this->overlays[0];
}

/**
 * @return Returns the underlay which is the lowest
 * displayed overlay.
 */
Overlay* 
OverlaySet::getUnderlay()
{
    return &this->overlays[this->getNumberOfDisplayedOverlays() - 1];
}

/**
 * Get the overlay at the specified index.
 * @param overlayNumber
 *   Index of the overlay.
 * @return Overlay at the given index.
 */
const Overlay* 
OverlaySet::getOverlay(const int32_t overlayNumber) const
{
    CaretAssertArrayIndex(this->overlays, 
                          BrainConstants::MAXIMUM_NUMBER_OF_OVERLAYS, 
                          overlayNumber);
    return &this->overlays[overlayNumber];    
}

/**
 * Get the overlay at the specified index.
 * @param overlayNumber
 *   Index of the overlay.
 * @return Overlay at the given index.
 */
Overlay* 
OverlaySet::getOverlay(const int32_t overlayNumber)
{
    CaretAssertArrayIndex(this->overlays, 
                          BrainConstants::MAXIMUM_NUMBER_OF_OVERLAYS, 
                          overlayNumber);
    return &this->overlays[overlayNumber];    
}

/**
 * Get a description of this object's content.
 * @return String describing this object's content.
 */
AString 
OverlaySet::toString() const
{
    return "OverlaySet";
}

/**
 * Add a displayed overlay.  If the maximum
 * number of surface overlays is reached,
 * this method has no effect.
 */
void 
OverlaySet::addDisplayedOverlay()
{
    this->numberOfDisplayedOverlays++;
    if (this->numberOfDisplayedOverlays > BrainConstants::MAXIMUM_NUMBER_OF_OVERLAYS) {
        this->numberOfDisplayedOverlays = BrainConstants::MAXIMUM_NUMBER_OF_OVERLAYS;
    }
}

/**
 * @return Returns the number of displayed overlays. 
 */
int32_t 
OverlaySet::getNumberOfDisplayedOverlays() const
{
    return this->numberOfDisplayedOverlays;
}

/**
 * Remove a displayed overlay.  This method will have
 * no effect if the minimum number of overlays are
 * displayed
 *
 * @param overlayIndex 
 *    Index of overlay for removal from display.
 */
void 
OverlaySet::removeDisplayedOverlay(const int32_t overlayIndex)
{
    if (this->numberOfDisplayedOverlays > BrainConstants::MINIMUM_NUMBER_OF_OVERLAYS) {
        this->numberOfDisplayedOverlays--;
        for (int32_t i = overlayIndex; i < this->numberOfDisplayedOverlays; i++) {
            this->overlays[i].copyData(&this->overlays[i+1]);
        }
    }
}

/**
 * Move the overlay at the given index up one level
 * (swap it with overlayIndex - 1).  This method will
 * have no effect if the overlay is the top-most overlay.
 *
 * @param overlayIndex 
 *    Index of overlay that is to be moved up.
 */
void 
OverlaySet::moveDisplayedOverlayUp(const int32_t overlayIndex)
{
    if (overlayIndex > 0) {
        this->overlays[overlayIndex].swapData(&this->overlays[overlayIndex - 1]);
    }
}

/**
 * Move the overlay at the given index down one level
 * (swap it with overlayIndex + 1).  This method will
 * have no effect if the overlay is the bottom-most overlay.
 *
 * @param overlayIndex 
 *    Index of overlay that is to be moved down.
 */
void 
OverlaySet::moveDisplayedOverlayDown(const int32_t overlayIndex)
{
    const int32_t nextOverlayIndex = overlayIndex + 1;
    if (nextOverlayIndex < this->numberOfDisplayedOverlays) {
        this->overlays[overlayIndex].swapData(&this->overlays[nextOverlayIndex]);
    }
}



