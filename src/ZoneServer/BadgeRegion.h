/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef ZONE_SERVER_BADGE_REGION_H_
#define ZONE_SERVER_BADGE_REGION_H_

#include <boost/noncopyable.hpp>

#include "RegionObject.h"

/** Badge regions represent sections of the world map that players can earn
* badges by exploring.
*/
class BadgeRegion : public RegionObject , boost::noncopyable {
public:
    /** Constructs a badge region with a specific id.
    *
    * \param badge_id The badge id to construct the badge region with.
    */
    explicit BadgeRegion(uint32_t badge_id);    

    // Default destructor.
    ~BadgeRegion();

    /// Returns the badge id for this region.
    uint32_t badge_id() const;
    
    /// Set's the badge id for this region.
    void badge_id(uint32_t badge_id);
    
private:
    // Disable the default constructor, badge region should never be created
    // without an id.
    BadgeRegion();

    // Override onObjectEnter handler to provide custom functionality
	void onObjectEnter(Object* object);

	uint32_t badge_id_;
};

#endif  // ZONE_SERVER_BADGE_REGION_H_
