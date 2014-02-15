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

#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"
#include "ZoneServer/WorldManager.h"

#include <cassert>

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "MessageLib/MessageLib.h"

#include "anh/Utils/Scheduler.h"
#include "Utils/VariableTimeScheduler.h"
#include "Utils/utils.h"
#include "NetworkManager/MessageFactory.h"

#include "Zoneserver/GameSystemManagers/AdminManager.h"
#include "Zoneserver/GameSystemManagers/Buff Manager/Buff.h"
#include "Zoneserver/GameSystemManagers/Buff Manager/BuffEvent.h"
#include "Zoneserver/GameSystemManagers/Buff Manager/BuffManager.h"

#include "Zoneserver/Objects/Datapad.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/GameSystemManagers/CharacterLoginHandler.h"
#include "ZoneServer/GameSystemManagers/Conversation Manager/ConversationManager.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/CraftingSessionFactory.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/SchematicManager.h"

#include "ZoneServer/Objects/CraftingTool.h"
#include "ZoneServer/GameSystemManagers/Spawn Manager/CreatureSpawnRegion.h"

#include "ZoneServer/GameSystemManagers/Group Manager/GroupManager.h"
#include "ZoneServer/GameSystemManagers/Group Manager/GroupObject.h"

#include "ZoneServer/GameSystemManagers/Structure Manager/FactoryFactory.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/FactoryObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/HarvesterFactory.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/HarvesterObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/PlayerStructure.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/BuildingObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/CellObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/HouseObject.h"

#include "Zoneserver/Objects/Inventory.h"
#include "ZoneServer/GameSystemManagers/Mission Manager/MissionManager.h"
#include "ZoneServer/GameSystemManagers/Mission Manager/MissionObject.h"
#include "ZoneServer/Objects/MountObject.h"
#include "Zoneserver/GameSystemManagers/NPC Manager/NpcManager.h"
#include "ZoneServer/GameSystemManagers/NPC Manager/NPCObject.h"
#include "ZoneServer/Objects/Object/ObjectFactory.h"

#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceManager.h"

#include "ZoneServer/Objects/Shuttle.h"
#include "ZoneServer/GameSystemManagers/Spawn Manager/SpawnPoint.h"
#include "ZoneServer/Objects/Terminal.h"
#include "ZoneServer/GameSystemManagers/Travel Manager/TicketCollector.h"
#include "ZoneServer/GameSystemManagers/Treasury Manager/TreasuryManager.h"
#include "ZoneServer/WorldConfig.h"
#include "Zoneserver/Objects/waypoints/WaypointObject.h"
#include "ZoneServer/Objects/VehicleController.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/ZoneServer.h"


#include "ZoneServer\Services\ham\ham_service.h"

using std::dynamic_pointer_cast;
using std::shared_ptr;



//======================================================================================================================
//
// adds an object to the world->to cells and the SI only, use manual if adding to containers, or cells on preload
//

bool WorldManager::addSharedObject(std::shared_ptr<Object> &object, bool manual)
{
    uint64 key = object->getId();

	if(getObjectById(key))
    {
        LOG(error) << "WorldManager::addObject Object(" << key<<") already exists added several times or ID messup ???";
        return false;
    }
	object_map_.insert(std::make_pair(key, object));
	//object_map_.insert(key,object);
    
	if(manual)
		return true;

	initializeObject(object);
	
	return true;
}

void WorldManager::initializeObject(std::shared_ptr <Object> &object)
{
	switch(object->getType())
	{
		//kets add our Region to the zmap
		case ObjType_Region:
		{

			//mRegionMap.insert(std::make_pair(key,object));
			std::shared_ptr<RegionObject> region = std::static_pointer_cast<RegionObject>(object);
			gSpatialIndexManager->addRegion(region);
			break;
		}

		case ObjType_Player:
		{

			std::shared_ptr <PlayerObject> player = std::dynamic_pointer_cast<PlayerObject>(object);

			// insert into the player map
			mPlayerAccMap.insert(std::make_pair(player->getAccountId(),player.get()));			

			LOG(info) << "New Player: " << player->getId() << ", Total Players on zone " << (getPlayerAccMap())->size();
			
			// not used currently - use with grid regions later ??
			player->setSubZoneId(0);
			
			// add ham to regeneration scheduler
			auto ham = getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");
			ham->addToRegeneration(player->getId());

			player->getStomach()->checkForRegen();

			// onPlayerEntered event, notify scripts
            //std::stringstream params;
			//params << getPlanetNameThis() << " " << player->getFirstName() 
                    //<< " " << static_cast<uint32>(mPlayerAccMap.size());

			//mWorldScriptsListener.handleScriptEvent("onPlayerEntered",params.str().c_str());

		}
		break;

		case ObjType_Structure:
		{
	
			mStructureList.push_back(object->getId());
			
			//create the building in the world
			gSpatialIndexManager->createInWorld(object.get());

		}
		break;

		case ObjType_Building:
		{
			mStructureList.push_back(object->getId());
			std::shared_ptr <BuildingObject> building = std::dynamic_pointer_cast<BuildingObject>(object);
			
			//create the building in the world
			gSpatialIndexManager->createInWorld(object.get());
		}
		break;


		case ObjType_Cell:
		{
		}
		break;

		case ObjType_Tangible:
		{

			//create the tangible in the world
			gSpatialIndexManager->createInWorld(object.get());
			
		}
		break;

		case ObjType_NPC:
		case ObjType_Creature:
		case ObjType_Lair:
		{
			CreatureObject* creature = dynamic_cast<CreatureObject*>(object.get());

			if(creature->getCreoGroup() == CreoGroup_Shuttle)
				mShuttleList.push_back(dynamic_cast<Shuttle*>(creature));

			//create the creature in the world
			gSpatialIndexManager->createInWorld(creature);
			
		}
		break;

		case ObjType_Intangible:
		{
			//they dont get added here in the firstplace ...
			DLOG(info) << "Object of type ObjType_Intangible UNHANDLED in WorldManager::addObject:";
		}
		break;

		default:
		{
			// Please, when adding new stufff, at least take the time to add a stub for that type.
		
		}
		break;
	}

}

bool WorldManager::addObject(Object* object,bool manual)
{
    uint64 key = object->getId();

    //make sure objects arnt added several times!!!!
    if(getObjectById(key))
    {
        LOG(info) << "WorldManager::addObject Object(" << key<<") already exists added several times or ID messup ???";
        return false;
    }

	std::shared_ptr<Object> object_shared(object);

	object_map_.insert(std::make_pair(key, object_shared));
    

    // if we want to set the parent manually or the object is from the snapshots and not a building, return
    if(manual)    {
        return true;
    }
	initializeObject(object_shared);

		
	return true;
}
bool WorldManager::addObject(std::shared_ptr<Object> object, bool manual)
{
    uint64 key = object->getId();

	if(getObjectById(key))
    {
        LOG(error) << "WorldManager::addObject Object(" << key<<") already exists added several times or ID messup ???";
        return false;
    }
	object_map_.insert(std::make_pair(key, object));
	//object_map_.insert(key,object);
    
	if(manual)
		return true;

	initializeObject(object);
	
	return true;
}

void WorldManager::destroyObject(uint64 objId)
{
	std::shared_ptr<Object> object = this->getSharedObjectById(objId);
	if(!object)	{
		LOG(error) <<"WorldManager::destroyObject : Object : " << objId << "couldnt be found in the Object Map";
		return;
	}
	destroyObject(object);
	
}


void WorldManager::destroyObject(Object* object)
{
	LOG(info) << "WorldManager::destroyObject - remove Object " << object->getId() <<" from world";

	Object* testObject = this->getObjectById(object->getId());
	if(!testObject)	{
		LOG(error) << "WorldManager::destroyObject " << object->getId() <<" couldnt be found in the Main Object Map";
		return;
	}

	switch(object->getType())
	{
		//players are always in the grid
		case ObjType_Player:
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(object);

			// remove us from the player map
			gWorldManager->removePlayerfromAccountMap(player->getId());
			
			// move to the nearest cloning center, if we are incapped or dead
			if(player->states.checkPosture(CreaturePosture_Incapacitated)
			|| player->states.checkPosture(CreaturePosture_Dead))
			{
				// bring up the clone selection window
				ObjectSet						inRangeBuildings;
				BStringVector					buildingNames;
				std::vector<BuildingObject*>	buildings;
				BuildingObject*					nearestBuilding = NULL;

				gSpatialIndexManager->getObjectsInRange(object,&inRangeBuildings,ObjType_Building,8192,false);

				ObjectSet::iterator buildingIt = inRangeBuildings.begin();

				while(buildingIt != inRangeBuildings.end())
				{
					BuildingObject* building = dynamic_cast<BuildingObject*>(*buildingIt);

					// TODO: This code is not working as intended if player dies inside, since buildings use world coordinates and players inside have cell coordinates.
					// Tranformation is needed before the correct distance can be calculated.
					if(building && building->getBuildingFamily() == BuildingFamily_Cloning_Facility)
					{
						if(!nearestBuilding
							|| (nearestBuilding != building && (glm::distance(player->getWorldPosition(), building->mPosition) < glm::distance(player->getWorldPosition(), nearestBuilding->mPosition))))
						{
							nearestBuilding = building;
						}
					}

					++buildingIt;
				}
				/*
				if(nearestBuilding)
				{
					if(nearestBuilding->getSpawnPoints()->size())
					{
						if(SpawnPoint* sp = nearestBuilding->getRandomSpawnPoint())
						{
							// update the database with the new values
							gWorldManager->getKernel()->GetDatabase()->executeSqlAsync(0,0,"UPDATE %s.characters SET parent_id=%"PRIu64",oX=%f,oY=%f,oZ=%f,oW=%f,x=%f,y=%f,z=%f WHERE id=%"PRIu64"",mDatabase->galaxy(),sp->mCellId
								,sp->mDirection.x,sp->mDirection.y,sp->mDirection.z,sp->mDirection.w
								,sp->mPosition.x,sp->mPosition.y,sp->mPosition.z
								,player->getId());
						}
					}
				}
				*/
			}

			// make sure we stop entertaining if we are an entertainer
			gEntertainerManager->stopEntertaining(player);

			LOG(error) << " going to remove Player : " << player->getId() << "from simulation" ;
			gSpatialIndexManager->RemoveObjectFromWorld(player);
			LOG(error) << "removed Player : " << player->getId() << "from simulation" ;

			// onPlayerLeft event, notify scripts
			std::stringstream params;
			params << getPlanetNameThis() << " " << player->getFirstName()
                    << " " << static_cast<uint32>(mPlayerAccMap.size());

			//mWorldScriptsListener.handleScriptEvent("onPlayerLeft",params.str().c_str());
			
			delete player->getClient();
			
			player->setClient(NULL);
			player->setConnectionState(PlayerConnState_Destroying);
			LOG(error) << "Player : " << player->getId() << " delete client" ;

		}
		break;

		case ObjType_NPC:
		case ObjType_Creature:
		{
			CreatureObject* creature = dynamic_cast<CreatureObject*>(object);
			LOG(error) << "remove creature / NPC from world id : " << creature->getId();

			// if its a shuttle, remove it from the shuttle list
			if(creature->getCreoGroup() == CreoGroup_Shuttle)
			{
				ShuttleList::iterator shuttleIt = mShuttleList.begin();
				while(shuttleIt != mShuttleList.end())
				{
					if((*shuttleIt)->getId() == creature->getId())
					{
						mShuttleList.erase(shuttleIt);
						break;
					}

					++shuttleIt;
				}
			}
		}
		break;


		case ObjType_Structure:
		{

			//remove it out of the worldmanagers structurelist now that it is deleted
			ObjectIDList::iterator itStruct = mStructureList.begin();
			while(itStruct != mStructureList.end())
			{
				if((*itStruct)==object->getId())
					itStruct = mStructureList.erase(itStruct);
				else
					itStruct++;
			}
			
		}
		break;

		// called when a structure gets destroyed - not on shutdown
		case ObjType_Building:
		{

			BuildingObject* building = dynamic_cast<BuildingObject*>(object);
			if(building)
			{				
				//remove it out of the worldmanagers structurelist now that it is deleted
				ObjectIDList::iterator itStruct = mStructureList.begin();
				while(itStruct != mStructureList.end())
				{
					if((*itStruct)==object->getId())
						itStruct = mStructureList.erase(itStruct);
					else
						itStruct++;
				}
			
			}
		}
		break;

		case ObjType_Cell:
		{
			// do nothing - cells get destroyed in the structures destructor
			// the cells ID will be removed below
			
		}
		break;

		case ObjType_Tangible:
		{
			
		}
		break;



		case ObjType_Intangible:
		{
			uint64 parentId = object->getParentId();
			
			Datapad* pad = dynamic_cast<Datapad*>(gWorldManager->getObjectById(parentId));
			
			//update the datapad
			if(!pad || !(pad->removeData(object->getId())))
			{
				DLOG(warning) << "WorldManager::destroyObject : Error removing Data from datapad " << object->getId();
			}

			if(VehicleController* vehicle = dynamic_cast<VehicleController*>(object))
			{
				vehicle->Store();
			}
		

			// intangibles are controllers / pets in the datapad
			// they are NOT in the world
			PlayerObject* owner = dynamic_cast<PlayerObject*>(getObjectById(pad->getParentId()));
			if(owner)
			{
				gMessageLib->sendDestroyObject(object->getId(),owner);
			}
			
			
		}
		break;

		default:
		{
			DLOG(warning) << "Unhandled ObjectType in WorldManager::destroyObject: " << (uint32)(object->getType());


		}
		break;
	}

	gSpatialIndexManager->RemoveObjectFromWorld(object);

	// finally delete it
	SharedObjectMap::iterator objMapIt = object_map_.find(object->getId());

	if(objMapIt != object_map_.end())	{
		object_map_.erase(objMapIt);
	}
}

void WorldManager::destroyObject(std::shared_ptr<Object> object)
{
	switch(object->getType())
	{
		case ObjType_Region:		{
			gSpatialIndexManager->RemoveRegion(std::static_pointer_cast<RegionObject>(object));
		}
		break;
	

		case ObjType_Waypoint:
		{
			uint64 parentId = object->getParentId();
			
			Datapad* pad = dynamic_cast<Datapad*>(gWorldManager->getObjectById(parentId+DATAPAD_OFFSET));
			
			//update the datapad
			if(!pad)
			{
				DLOG(warning) << "Worldmanager::destroyObject: Error removing Waypoint from datapad " << parentId;
				break;
			}

			pad->RemoveWaypoint(object->getId());

		}
		break;
	}

	// finally delete it
	SharedObjectMap::iterator objMapIt = object_map_.find(object->getId());

	if(objMapIt != object_map_.end())	{
		object_map_.erase(objMapIt);
	}
	
}

//======================================================================================================================
//
// simply erase an object ID out of the worlds ObjectMap *without* accessing the object
// use this for objects that arnt part of the spatial Index anymore
// for example in a destructor

void WorldManager::eraseObject(uint64 key)
{
	SharedObjectMap::iterator shared_it = object_map_.find(key);
	if(shared_it != object_map_.end())
    {
        object_map_.erase(shared_it);
		return;
    }


    DLOG(info) << "WorldManager::destroyObject: error removing from objectmap: " << key;
    
}

//======================================================================================================================
bool WorldManager::_handleGeneralObjectTimers(uint64 callTime, void* ref)
{
    CreatureObjectDeletionMap::iterator it = mCreatureObjectDeletionMap.begin();
    while (it != mCreatureObjectDeletionMap.end())
    {
        //  The timer has expired?
        if (callTime >= ((*it).second))
        {
            // Is it a valid object?
            CreatureObject* creature = dynamic_cast<CreatureObject*>(getObjectById((*it).first));
            if (creature)
            {
                // Yes, handle it. We may put up a copy of this npc...
                NpcManager::Instance()->handleExpiredCreature((*it).first);
                this->destroyObject(creature);
                mCreatureObjectDeletionMap.erase(it++);
            }
            else
            {
                // Remove the invalid object...from this list.
                mCreatureObjectDeletionMap.erase(it++);
            }
        }
        else
        {
            ++it;
        }
    }

    PlayerObjectReviveMap::iterator reviveIt = mPlayerObjectReviveMap.begin();
    while (reviveIt != mPlayerObjectReviveMap.end())
    {
        //  The timer has expired?
        if (callTime >= ((*reviveIt).second))
        {

            PlayerObject* player = dynamic_cast<PlayerObject*>(getObjectById((*reviveIt).first));
            if (player)
            {
                // Yes, handle it.
                // Send the player to closest cloning facility.
                player->cloneAtNearestCloningFacility();

                // The cloning request removes itself from here, have to restart the iteration.
                reviveIt = mPlayerObjectReviveMap.begin();
            }
            else
            {
                // Remove the invalid object...
                mPlayerObjectReviveMap.erase(reviveIt++);
            }
        }
        else
        {
            ++reviveIt;
        }
    }
    return (true);
}

//======================================================================================================================
//
//	getNearest Terminal
//

Object* WorldManager::getNearestTerminal(PlayerObject* player, TangibleType terminalType, float searchrange)
{
	//this will get the nearest terminal in the world - we need to check playerbuildings, too
	ObjectSet		inRangeObjects;

	//gets all terminals in range even those in building
	gSpatialIndexManager->getObjectsInRange(player,&inRangeObjects,(ObjType_Tangible),searchrange,true);
    
    ObjectSet::iterator it = inRangeObjects.begin();

    float	range = 0.0;
    Object* nearestTerminal = NULL;

    while(it != inRangeObjects.end())
    {
		Terminal* terminal = dynamic_cast<Terminal*> (*it);
		if(terminal&&(terminal->getTangibleType() == terminalType))
		{
            float nr = glm::distance(terminal->getWorldPosition(), player->getWorldPosition());
			
			//double check the distance
			if((nearestTerminal && (nr < range ))||(!nearestTerminal))
			{
				range = nr;
				nearestTerminal = terminal;
			}
		}
		

		++it;
	}
	return nearestTerminal;
}

Object*	WorldManager::getObjectById(uint64 objId)
{
	
    SharedObjectMap::iterator it = object_map_.find(objId);

    if(it != object_map_.end())
    {
		return((*it).second.get());
    }

    return(nullptr);
}

std::shared_ptr<Object>	WorldManager::getSharedObjectById(uint64 objId)
{
	SharedObjectMap::iterator it = object_map_.find(objId);

    if(it != object_map_.end())
    {
        return((*it).second);
    }

    return(nullptr);

}