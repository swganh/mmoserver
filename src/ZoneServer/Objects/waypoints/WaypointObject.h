/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#ifndef ANH_ZONESERVER_WAYPOINT_OBJECT_H
#define ANH_ZONESERVER_WAYPOINT_OBJECT_H

#include "ZoneServer/Objects/Object.h"


namespace swganh	{
	class ByteBuffer;
}

//=============================================================================

enum waypoint_types
{
    Waypoint_blue	= 1,
    Waypoint_green	= 2,
    Waypoint_orange	= 3,
    Waypoint_yellow	= 4,
    Waypoint_red	= 5,
    Waypoint_white	= 6,
    Waypoint_JTL	= 7
};


//=============================================================================

class WaypointObject : public Object
{
    friend class WaypointFactory;

public:

    WaypointObject();
    ~WaypointObject();

    bool				getActive() {
        return mActive;
    }
    void				setActive(bool active) {
        mActive = active;
    }
    void				toggleActive() {
        mActive = !mActive;
    };
	std::u16string		getName() {
        return mName;
    }
    void				setName(const std::u16string name) {
        mName = name;
    }
    uint8				getWPType() {
        return mWPType;
    }
    void				setWPType(uint8 type) {
        mWPType = type;
    }

    void				setCoords(const glm::vec3& coords) {
        mCoords = coords;
    }
    const glm::vec3&	getCoords() {
        return mCoords;
    }

    uint32				getPlanetCRC() {
        return mPlanetCRC;
    }
    void				setPlanetCRC(uint32 planet_crc) {
        mPlanetCRC = planet_crc;
    }

	uint32				getPlanetId() {
        return planet_id_;
    }
    void				setPlanetId(uint16 planet_id) {
        planet_id_	= planet_id;
    }

	std::string			getPlanet() {
        return planet_;
    }
	void				setPlanet(std::string planet);
    

private:

    glm::vec3			mCoords;
    bool				mActive;
    std::u16string		mName;
	std::string			planet_;
    uint8				mWPType;
	uint16				planet_id_;
    uint32				mPlanetCRC;
};


struct PlayerWaypointSerializer
{
	
    PlayerWaypointSerializer()
        : waypoint(nullptr) {}
    PlayerWaypointSerializer(std::shared_ptr<WaypointObject> waypoint_)
        : waypoint(waypoint_) {}

    static void SerializeBaseline(swganh::ByteBuffer& data, const PlayerWaypointSerializer& t);
    static void SerializeDelta(swganh::ByteBuffer& data, const PlayerWaypointSerializer& t);

    bool operator==(const PlayerWaypointSerializer& other);

    std::shared_ptr<WaypointObject> waypoint;
	
};


//=============================================================================

#endif

