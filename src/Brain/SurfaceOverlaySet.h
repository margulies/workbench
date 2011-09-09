#ifndef __SURFACE_OVERLAY_SET__H_
#define __SURFACE_OVERLAY_SET__H_

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

#include "BrainConstants.h"
#include "CaretObject.h"
#include "SurfaceOverlay.h"

namespace caret {

    
    /// Contains a set of overlay assignments
    class SurfaceOverlaySet : public CaretObject {
        
    public:
        SurfaceOverlaySet();
        
        virtual ~SurfaceOverlaySet();
        
    private:
        SurfaceOverlaySet(const SurfaceOverlaySet&);

        SurfaceOverlaySet& operator=(const SurfaceOverlaySet&);
        
        SurfaceOverlay* getPrimaryOverlay();
        
        SurfaceOverlay* getUnderlay();
        
        SurfaceOverlay* getOverlay(const int32_t overlayNumber);
        
    public:
        virtual AString toString() const;
        
    private:
        SurfaceOverlay overlays[BrainConstants::MAXIMUM_NUMBER_OF_SURFACE_OVERLAYS];
    };
    
#ifdef __SURFACE_OVERLAY_SET_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __SURFACE_OVERLAY_SET_DECLARE__

} // namespace
#endif  //__SURFACE_OVERLAY_SET__H_
