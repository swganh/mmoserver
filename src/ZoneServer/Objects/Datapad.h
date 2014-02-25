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

#ifndef ANH_ZONESERVER_TANGIBLE_DATAPAD_H
#define ANH_ZONESERVER_TANGIBLE_DATAPAD_H

#include "ZoneServer/Objects/Tangible Object/TangibleObject.h"
#include "ZoneServer/Objects/Object/ObjectFactoryCallback.h"
#include <MessageLib\messages\containers\network_map.h>
#include <MessageLib\messages\containers\network_vector.h>


//=============================================================================

class IntangibleObject;
class ManufacturingSchematic;
class MissionObject;
class PlayerObject;
class WaypointObject;

struct PlayerWaypointSerializer;

namespace messages	{
	struct BaseSwgMessage;
}


typedef std::list<ManufacturingSchematic*>						ManufacturingSchematicList;
typedef std::list<MissionObject*>								MissionList;
typedef std::list<IntangibleObject*>							DataList;


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
	virtual void	handleObjectReady(std::shared_ptr<Object> object);

    // player reference
    void			setOwner(CreatureObject* creature) {
        mOwner = creature;
    }

	/**
		 * @return The waypoints currently held by the player.
		 */
		std::vector<std::shared_ptr<WaypointObject>> GetWaypoints() ;
		std::vector<std::shared_ptr<WaypointObject>> GetWaypoints(boost::unique_lock<boost::mutex>& lock) ;

		/**
		 * Adds a waypoint to the player.
		 *
		 * @param waypoint The waypoint to add to the player.
		 */
		void AddWaypoint(const std::shared_ptr<WaypointObject>& waypoint);
		void AddWaypoint(const std::shared_ptr<WaypointObject>& waypoint, boost::unique_lock<boost::mutex>& lock);

		/**
		 * Adds a waypoint to the player.
		 * WITHOUT creating a delta (when initializing the player)
		 * @param waypoint The waypoint to add to the player.
		 */
		void AddWaypointInit(const std::shared_ptr<WaypointObject>& waypoint);
		void AddWaypointInit(const std::shared_ptr<WaypointObject>& waypoint, boost::unique_lock<boost::mutex>& lock);

		/**
		 * Removes a waypoint from the player.
		 *
		 * @param waypoint_id The id of the waypoint to remove.
		 */
		void RemoveWaypoint(uint64_t waypoint_id);
		void RemoveWaypoint(uint64_t waypoint_id, boost::unique_lock<boost::mutex>& lock);

		/**
		 * Modifies an existing waypoint.
		 *
		 * @param waypoint The new waypoint data.
		 */
		void ModifyWaypoint(uint64_t way_object_id);
		void ModifyWaypoint(uint64_t way_object_id, boost::unique_lock<boost::mutex>& lock);

		bool SerializeWaypoints(swganh::messages::BaseSwgMessage* message);
		bool SerializeWaypoints(swganh::messages::BaseSwgMessage* message, boost::unique_lock<boost::mutex>& lock);


    
    std::shared_ptr<WaypointObject> getWaypointById(uint64 id);
    
	std::shared_ptr<WaypointObject> getWaypointByName(std::u16string name);
    
	
    
	
	void			setObjectLoadCounter(uint32 count) {
        mObjectLoadCounter = count;
    }
    void			requestNewWaypoint(std::u16string name, const glm::vec3& coords, uint16 planetId, uint8 wpType);
    
	/*	@brief updates a waypointobjects members, persists them to db and then sends the relevant delta to the client
	*/
	void			updateWaypoint(uint64 wpId, std::u16string name, const glm::vec3& coords, uint16 planetId, uint64 owner, uint8 activeStatus);

	void			updateWaypoint(std::shared_ptr<WaypointObject> waypoint);

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
	swganh::containers::NetworkMap<uint64_t, PlayerWaypointSerializer, PlayerWaypointSerializer> waypoints_;

    uint8						mCapacity;
    uint8						mWayPointCapacity;
    uint8						mMissionCapacity;
    //WaypointList				mWaypoints;

    ManufacturingSchematicList	mManufacturingSchematics;
    MissionList					mMissions;
    DataList					mData;
    CreatureObject*				mOwner;
    uint32						mObjectLoadCounter;
};

//=============================================================================

#endif

