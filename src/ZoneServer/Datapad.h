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

#ifndef ANH_ZONESERVER_TANGIBLE_DATAPAD_H
#define ANH_ZONESERVER_TANGIBLE_DATAPAD_H

#include "TangibleObject.h"
#include "ObjectFactoryCallback.h"


//=============================================================================

class IntangibleObject;
class ManufacturingSchematic;
class MissionObject;
class PlayerObject;
class WaypointObject;

typedef std::list<WaypointObject*>			WaypointList;
typedef std::list<ManufacturingSchematic*>	ManufacturingSchematicList;
typedef std::list<MissionObject*>				MissionList;
typedef std::list<IntangibleObject*>			DataList;

//=============================================================================
enum WaypointStatus
{
    WAYPOINT_INACTIVE		= 0,
    WAYPOINT_ACTIVE			= 1

};
class Datapad : public TangibleObject
{

    friend class DatapadFactory;

public:
    Datapad();
    ~Datapad();

    // inherited callback
    virtual void	handleObjectReady(Object* object,DispatchClient* client);

    // player reference
    void			setOwner(PlayerObject* player) {
        mOwner = player;
    }

    // waypoints
    WaypointList*	getWaypoints() {
        return &mWaypoints;
    }
    WaypointObject*	getWaypointById(uint64 id);
    WaypointObject*	getWaypointByName(BString name);
    bool			addWaypoint(WaypointObject* waypoint);
    bool			removeWaypoint(uint64 id);
    bool			removeWaypoint(WaypointObject* waypoint);
    void			setObjectLoadCounter(uint32 count) {
        mObjectLoadCounter = count;
    }
    void			requestNewWaypoint(BString name, const glm::vec3& coords, uint16 planetId, uint8 wpType);
    void			updateWaypoint(uint64 wpId, BString name, const glm::vec3& coords, uint16 planetId, uint64 owner, uint8 activeStatus);

    //missions
    MissionList*   getMissions() {
        return &mMissions;
    }
    MissionObject* getMissionById(uint64 id);
    bool		   addMission(MissionObject* mission);
    bool		   removeMission(uint64 id);
    bool		   removeMission(MissionObject* mission);
    bool		   hasMission() {
        return( mMissions.size()>0);
    }

    MissionList::iterator removeMission(MissionList::iterator it);

    //data -- aka mounts,pets,vehicles, etc
    DataList*		   getData() {
        return &mData;
    }
    IntangibleObject*  getDataById(uint64 id);
    bool		       addData(IntangibleObject* Data);
    bool		       removeData(uint64 id);
    bool		       removeData(IntangibleObject* Data);
    DataList::iterator removeData(DataList::iterator it);

    // ManufacturingSchematics
    ManufacturingSchematicList*		getManufacturingSchematics() {
        return &mManufacturingSchematics;
    }
    ManufacturingSchematic*			getManufacturingSchematicById(uint64 id);


    bool							addManufacturingSchematic(ManufacturingSchematic* ms);
    bool							removeManufacturingSchematic(uint64 id);
    bool							removeManufacturingSchematic(ManufacturingSchematic* ms);


    // capacity
    uint8			getCapacity() {
        return mCapacity;
    }
    uint8			getMissionCapacity() {
        return mMissionCapacity;
    }

    uint32			mWaypointUpdateCounter;
    uint32			mManSUpdateCounter;
    uint32			mSchematicUpdateCounter;
    uint32			mMissionUpdateCounter;

private:

    uint8						mCapacity;
    uint8						mWayPointCapacity;
    uint8						mMissionCapacity;
    WaypointList				mWaypoints;
    ManufacturingSchematicList	mManufacturingSchematics;
    MissionList					mMissions;
    DataList					mData;
    PlayerObject*				mOwner;
    uint32						mObjectLoadCounter;
};

//=============================================================================

#endif

