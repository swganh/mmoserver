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
#include "WaypointObject.h"
#include "anh\byte_buffer.h"
#include "anh\crc.h"


//=============================================================================

WaypointObject::WaypointObject() : Object()
{
    mType = ObjType_Waypoint;
    mCoords = glm::vec3();
    mActive = false;
	std::string n("");
	mName = std::u16string(n.begin(), n.end());

    mWPType = Waypoint_blue;
    mPlanetCRC = 0;
}

//=============================================================================

WaypointObject::~WaypointObject()
{
}

//=============================================================================


void PlayerWaypointSerializer::SerializeBaseline(swganh::ByteBuffer& data, const PlayerWaypointSerializer& t)
{
    data.write<uint8_t>(0);
	data.write<uint64_t>(t.waypoint->getId());
    data.write<uint32_t>(0);
    //auto coordinates_ = t.waypoint->GetCoordinates();
	data.write<float>(t.waypoint->getCoords().x);
    data.write<float>(t.waypoint->getCoords().y);
    data.write<float>(t.waypoint->getCoords().z);
    data.write<uint64_t>(0);
    data.write<uint32_t>(t.waypoint->getPlanetCRC());
    data.write<std::u16string>(t.waypoint->getName());
    data.write<uint64_t>(t.waypoint->getId());
    data.write<uint8_t>(t.waypoint->getWPType());
    data.write<uint8_t>(t.waypoint->getActive() ? 1 : 0);
}

void PlayerWaypointSerializer::SerializeDelta(swganh::ByteBuffer& data, const PlayerWaypointSerializer& t)
{
    data.write<uint64_t>(t.waypoint->getId());
    data.write<uint32_t>(0);
    data.write<float>(t.waypoint->getCoords().x);
    data.write<float>(t.waypoint->getCoords().y);
    data.write<float>(t.waypoint->getCoords().z);
    data.write<uint64_t>(0);
    data.write<uint32_t>(t.waypoint->getPlanetCRC());
    data.write<std::u16string>(t.waypoint->getName());
    data.write<uint64_t>(t.waypoint->getId());
    data.write<uint8_t>(t.waypoint->getWPType());
    data.write<uint8_t>(t.waypoint->getActive() ? 1 : 0);
}

bool PlayerWaypointSerializer::operator==(const PlayerWaypointSerializer& other)
{
    return waypoint->getId() == other.waypoint->getId();
}

void	WaypointObject::setPlanet(std::string planet) {
        planet_	= planet;
		mPlanetCRC = swganh::memcrc(planet_);
    }