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

#include "Zoneserver/Objects/Datapad.h"
#include "ZoneServer/Objects/VehicleController.h"
#include "Zoneserver/objects/IntangibleObject.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/ManufacturingSchematic.h"
#include "ZoneServer/GameSystemManagers/Mission Manager/MissionObject.h"
#include "ZoneServer/Objects/ObjectFactory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/Objects/Player Object/player_message_builder.h"
#include "ZoneServer/Objects/Player Object/PlayerObjectFactory.h"
#include "Zoneserver/Objects/waypoints/WaypointObject.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"

#include "MessageLib/MessageLib.h"

#include <MessageLib\messages\containers\network_map.h>

//=============================================================================

Datapad::Datapad()
    : TangibleObject()
    , mManufacturingSchematics(0)
    , mMissions(0)
    , mData(0)
    , mOwner(NULL)
    , mObjectLoadCounter(0) {
    mTanGroup					= TanGroup_Datapad;
    mTanType					= TanType_CharacterDatapad;
    mSchematicUpdateCounter		= 0;

    mCapacity = static_cast<uint8>(gWorldConfig->getConfiguration<uint32_t>("Player_Datapad_Capacity", static_cast<uint32_t>(100)));
    mWayPointCapacity = static_cast<uint8>(gWorldConfig->getConfiguration<uint32_t>("Player_DataPad_WayPointCapacity", static_cast<uint32_t>(100)));
    mMissionCapacity = static_cast<uint8>(gWorldConfig->getConfiguration<uint32_t>("Player_Datapad_MissionCapacity", static_cast<uint32_t>(100)));

    if(mMissionCapacity	> 6) {
        mMissionCapacity = 6;
    }
}


//=============================================================================

Datapad::~Datapad()
{
	LOG (info) << "Datapad::~Datapad()";

    //--------------------------------------------
    auto it = waypoints_.begin();
    while(it != waypoints_.end())    {
		gWorldManager->eraseObject(it->first);
		it++;
    }
    
	
	//--------------------------------------------
    MissionList::iterator ite = mMissions.begin();
    while(ite != mMissions.end())
    {
        ite = mMissions.erase(ite);
    }
    //--------------------------------------------

    
    DataList::iterator iter = mData.begin();
    while(iter != mData.end())
    {
        //check to see whether its a vehicle controller
        // PlayerObject* player = dynamic_cast<PlayerObject*>(creatureObject);
        //Object* vehicle = dynamic_cast<Object*>(*iter);
        //VehicleController* vehicle = dynamic_cast<VehicleController*>(*iter);

        //if(vehicle)
        //{
        uint64 id = (*iter)->getId();
        iter = mData.erase(iter);
        gWorldManager->eraseObject(id);
        //}
        //else
        //iter = mData.erase(iter);


    }
    //--------------------------------------------

}

//=============================================================================

std::shared_ptr<WaypointObject>   Datapad::getWaypointById(uint64 id)
{
    auto it = waypoints_.begin();
    while(it != waypoints_.end())
    {
		if((*it).first == id)	{
			std::shared_ptr<WaypointObject> waypoint = std::static_pointer_cast<WaypointObject>(gWorldManager->getSharedObjectById((*it).first));
            return(waypoint);
		}

        ++it;
    }
	
    return(nullptr);
}

//=============================================================================


//=============================================================================

std::shared_ptr<WaypointObject>  Datapad::getWaypointByName(std::u16string name)
{
    auto it = waypoints_.begin();

    while(it != waypoints_.end())
    {
		std::shared_ptr<WaypointObject> waypoint = std::static_pointer_cast<WaypointObject>(gWorldManager->getSharedObjectById((*it).first));
        
		if(name.compare(waypoint->getName()) == 0)
            return(waypoint);
        ++it;
    }

    return(nullptr);
}

//=============================================================================

ManufacturingSchematic* Datapad::getManufacturingSchematicById(uint64 id)
{
    ManufacturingSchematicList::iterator it = mManufacturingSchematics.begin();

    while(it != mManufacturingSchematics.end())
    {
        if((*it)->getId() == id)
            return(*it);
        ++it;
    }

    return(NULL);
}

//=============================================================================

bool Datapad::removeManufacturingSchematic(uint64 id)
{
    ManufacturingSchematicList::iterator it = mManufacturingSchematics.begin();

    while(it != mManufacturingSchematics.end())
    {
        if((*it)->getId() == id)
        {
            delete((*it)->getItem());
            mManufacturingSchematics.erase(it);
            mCapacity++;
            return(true);
        }

        ++it;
    }

    return(false);
}

//=============================================================================

bool Datapad::removeManufacturingSchematic(ManufacturingSchematic* ms)
{
    ManufacturingSchematicList::iterator it = mManufacturingSchematics.begin();

    while(it != mManufacturingSchematics.end())
    {
        if((*it) == ms)
        {
            delete(ms->getItem());
            mManufacturingSchematics.erase(it);
            mCapacity++;
            return(true);
        }

        ++it;
    }

    return(false);
}

//=============================================================================


bool Datapad::addData(IntangibleObject* Data)
{
    if(mCapacity)
    {
        mData.push_back(Data);
        //PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
        mCapacity--;

        // the datapads (or inventories) client volume counters are clientside updated
        // items being contained by the inventory (or datapad) do count as made out
        // by the objects volume count (in the tano 3)
        // please make sure that the containment isnt send by the factory before the item is even created

        return true;
    }
    else
    {
        PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
        if (player) {
            gMessageLib->SendSystemMessage(::common::OutOfBand("base_player","too_many_waypoints"), player);
        }
        return false;

    }
}

void Datapad::updateWaypoint(std::shared_ptr<WaypointObject> waypoint)
{
	if (!waypoint)    {
		LOG (error) << "Datapad::updateWaypoint : could not update waypoint : " << waypoint->getId();
		return;
	}

	ModifyWaypoint(waypoint->getId());

	gObjectFactory->requestUpdatedWaypoint(this, waypoint->getId(), waypoint->getName(), waypoint->getCoords(), waypoint->getPlanetId(), waypoint->getParentId(), waypoint->getActive());
}

void Datapad::updateWaypoint(uint64 wpId, std::u16string name, const glm::vec3& coords, uint16 planetId, uint64 owner, uint8 activeStatus)
{
	std::shared_ptr<WaypointObject> waypoint = getWaypointById(wpId);
	
	if (!waypoint)    {
		LOG (error) << "Datapad::updateWaypoint : could not update waypoint : " << wpId;
		return;
	}
		
	waypoint->setName(name);
	waypoint->setCoords(coords);
	std::string planet = gWorldManager->getPlanetNameById(planetId);
	
	waypoint->setPlanetCRC(swganh::memcrc(planet)); ;
	waypoint->setParentId(owner);
	waypoint->setActive((activeStatus != 0) ? true : false);

	ModifyWaypoint(wpId);

	gObjectFactory->requestUpdatedWaypoint(this, wpId, name, coords, planetId, owner, activeStatus);
    

}
//=============================================================================

void Datapad::handleObjectReady(std::shared_ptr<Object> object)
{
	std::shared_ptr<WaypointObject> waypoint;
	waypoint = std::dynamic_pointer_cast<WaypointObject>(object);

	if(waypoint)    {

		if(gPlayerObjectFactory->PlayerLoading(this->getParentId()))	{
			return;
		}

		AddWaypoint(waypoint);           
    }

}

void Datapad::handleObjectReady(Object* object,DispatchClient* client)
{
    

    if(ManufacturingSchematic* ms = dynamic_cast<ManufacturingSchematic*>(object))
    {
        if(addManufacturingSchematic(ms))
            gMessageLib->sendCreateManufacturingSchematic(ms, mOwner, false);
        else
        {
			gWorldManager->eraseObject(ms->getId());
        }
    }
}

//=============================================================================

MissionObject* Datapad::getMissionById(uint64 id)
{
    MissionList::iterator it = mMissions.begin();

    while(it != mMissions.end())
    {
        if((*it)->getId() == id) return(*it);
        ++it;
    }

    return NULL;
}

//=============================================================================

bool Datapad::removeMission(MissionObject* mission)
{
    MissionList::iterator it = mMissions.begin();
    while(it != mMissions.end())
    {
        if((*it) == mission)
        {
            removeMission(it);
            return true;
        }
        ++it;
    }
    return false;
}

//=============================================================================

bool Datapad::removeMission(uint64 id)
{
    MissionList::iterator it = mMissions.begin();
    while(it != mMissions.end())
    {
        if((*it)->getId() == id)
        {
            removeMission(it);
            return true;
        }
        ++it;
    }
    return false;
}

//=============================================================================

MissionList::iterator Datapad::removeMission(MissionList::iterator it)
{
    it = mMissions.erase(it);
    mCapacity++;
    mMissionCapacity++;
    return it;
}

//=============================================================================

IntangibleObject* Datapad::getDataById(uint64 id)
{
    DataList::iterator it = mData.begin();

    while(it != mData.end())
    {
        if((*it)->getId() == id) return(*it);
        ++it;
    }

    return NULL;
}

//=============================================================================

bool Datapad::removeData(IntangibleObject* Data)
{
    DataList::iterator it = mData.begin();
    while(it != mData.end())
    {
        if((*it) == Data)
        {
            removeData(it);
            return true;
        }
        ++it;
    }
    return false;
}

//=============================================================================

bool Datapad::removeData(uint64 id)
{
    DataList::iterator it = mData.begin();
    while(it != mData.end())
    {
        if((*it)->getId() == id)
        {
            removeData(it);
            return true;
        }
        ++it;
    }
    return false;
}

//=============================================================================

DataList::iterator Datapad::removeData(DataList::iterator it)
{
    it = mData.erase(it);
    mCapacity++;
    return it;
}

//=============================================================================


void Datapad::requestNewWaypoint(std::u16string name, const glm::vec3& coords, uint16 planetId, uint8 wpType)
{

    if(!mCapacity)    {
        PlayerObject*	player			= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
        if (player) {
            gMessageLib->SendSystemMessage(::common::OutOfBand("base_player","too_many_waypoints"), player);
        }

        return;
    }

    gObjectFactory->requestNewWaypoint(this,name,coords,planetId, mOwner->getId(),wpType);
}

//=============================================================================

bool Datapad::addManufacturingSchematic(ManufacturingSchematic* ms)
{
    if(mCapacity)
    {
        mManufacturingSchematics.push_back(ms);
        mCapacity--;
        return true;
    }
    else
    {
        PlayerObject*	player			= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
        if (player) {
            gMessageLib->SendSystemMessage(::common::OutOfBand("ui_craft","err_datapad_full_prose"), player);
        }

        return false;
    }
}

//=============================================================================

bool Datapad::addMission(MissionObject* mission)
{
    PlayerObject*	player			= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
    if(!mCapacity)
    {
        gMessageLib->SendSystemMessage(L"Error datapad at max capacity. Couldn't create the mission.", player);
        return false;
    }
    if(!mMissionCapacity)
    {
        gMessageLib->SendSystemMessage(L"Error datapad mission system at max capacity. Couldn't create the mission.", player);
        return false;
    }
    mMissions.push_back(mission);
    mCapacity--;
    mMissionCapacity--;
    return true;
}

bool Datapad::SerializeWaypoints(swganh::messages::BaseSwgMessage* message)
{
    auto lock = AcquireLock();
    return(SerializeWaypoints(message, lock));

}

bool Datapad::SerializeWaypoints(swganh::messages::BaseSwgMessage* message, boost::unique_lock<boost::mutex>& lock)
{	
	return(waypoints_.Serialize(message));	
}

void Datapad::AddWaypoint(const std::shared_ptr<WaypointObject>& waypoint)
{
    auto lock = AcquireLock();
    AddWaypoint(waypoint, lock);
}


void Datapad::AddWaypoint(const std::shared_ptr<WaypointObject>& waypoint, boost::unique_lock<boost::mutex>& lock)
{
	
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
	if(!player)	{
		//the parent isnt in the main object map as long as its still being loaded
		//this will suit us well as we dont want to create deltas in this case
		waypoints_.add_initialize(waypoint->getId(), waypoint);
		return;
	}

    //waypoint->setp  >SetContainer(shared_from_this());
	waypoints_.add(waypoint->getId(), waypoint);
    
	lock.unlock();

	auto dispatcher = GetEventDispatcher();
	dispatcher->Dispatch(std::make_shared<PlayerObjectEvent>("PlayerObject::Waypoint", player));
}

void Datapad::ModifyWaypoint(uint64_t waypoint_id)
{
    auto lock = AcquireLock();
    ModifyWaypoint(waypoint_id, lock);
}

void Datapad::ModifyWaypoint(uint64_t way_object_id, boost::unique_lock<boost::mutex>& lock)
{
    waypoints_.update(way_object_id);

	lock.unlock();
	LOG(info) << "Datapad::ModifyWaypoint : " << way_object_id;
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
	auto dispatcher = GetEventDispatcher();
	dispatcher->Dispatch(std::make_shared<PlayerObjectEvent>("PlayerObject::Waypoint", (player)));
   
}

void Datapad::RemoveWaypoint(uint64_t waypoint_id)
{
    auto lock = AcquireLock();
    RemoveWaypoint(waypoint_id, lock);
}

void Datapad::RemoveWaypoint(uint64_t waypoint_id, boost::unique_lock<boost::mutex>& lock)
{
    auto find_iter = std::find_if(waypoints_.begin(), waypoints_.end(), [waypoint_id] (std::pair<uint64_t, PlayerWaypointSerializer> stored_waypoint)
	{
        return waypoint_id == stored_waypoint.first;
    });

    if (find_iter == waypoints_.end())
    {
        return;
    }

    waypoints_.remove(find_iter);

	lock.unlock();

	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
    auto dispatcher = GetEventDispatcher();
	dispatcher->Dispatch(std::make_shared<PlayerObjectEvent>("PlayerObject::Waypoint", (player)));
}

std::vector<std::shared_ptr<WaypointObject>> Datapad::GetWaypoints()
{
    auto lock = AcquireLock();
    return GetWaypoints(lock);
}

std::vector<std::shared_ptr<WaypointObject>> Datapad::GetWaypoints(boost::unique_lock<boost::mutex>& lock)
{
    std::vector<std::shared_ptr<WaypointObject>> waypoints;
	auto it = waypoints_.begin();
	for(it = waypoints_.begin() ; it != waypoints_.end(); it++)
    {
		std::shared_ptr<WaypointObject> wp = std::static_pointer_cast<WaypointObject>(gWorldManager->getSharedObjectById((*it).first));
        waypoints.push_back(wp);
	}

    return waypoints;
}
