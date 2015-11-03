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

#include "ContainerManager.h"
#include "WorldManager.h"

#include <cassert>

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "MessageLib/MessageLib.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"
#include "Utils/Scheduler.h"
#include "Utils/VariableTimeScheduler.h"
#include "Utils/utils.h"
#include "NetworkManager/MessageFactory.h"

#include "AdminManager.h"
#include "Buff.h"
#include "BuffEvent.h"
#include "BuffManager.h"
#include "BuildingObject.h"
#include "CellObject.h"
#include "Datapad.h"
#include "HouseObject.h"
#include "PlayerObject.h"
#include "CharacterLoginHandler.h"
#include "Container.h"
#include "ConversationManager.h"
#include "CraftingSessionFactory.h"
#include "CraftingTool.h"
#include "CreatureSpawnRegion.h"
#include "FactoryFactory.h"
#include "FactoryObject.h"
#include "GroupManager.h"
#include "GroupObject.h"
#include "HarvesterFactory.h"
#include "HarvesterObject.h"
#include "Heightmap.h"
#include "Inventory.h"
#include "MissionManager.h"
#include "MissionObject.h"
#include "MountObject.h"
#include "NpcManager.h"
#include "NPCObject.h"
#include "ObjectFactory.h"
#include "PlayerStructure.h"
#include "ResourceManager.h"
#include "SchematicManager.h"
#include "Shuttle.h"
#include "SpawnPoint.h"
#include "Terminal.h"
#include "TicketCollector.h"
#include "TreasuryManager.h"
#include "WorldConfig.h"
#include "WaypointObject.h"
#include "VehicleController.h"
#include "ZoneOpcodes.h"
#include "ZoneServer.h"

using std::dynamic_pointer_cast;
using std::shared_ptr;

//======================================================================================================================
//
// returns the id of the first object that has a private owner that match the requested one.
//
// This function is not used yet.
uint64 WorldManager::getObjectOwnedBy(uint64 theOwner)
{
    ObjectMap::iterator it = mObjectMap.begin();
    uint64 ownerId = 0;

    while (it != mObjectMap.end())
    {
        if ( ((*it).second)->getPrivateOwner() == theOwner)
        {
            ownerId = (*it).first;
            break;
        }
        it++;
    }
    return ownerId;
}


//======================================================================================================================
//
// adds an object to the world->to cells and the SI only, use manual if adding to containers, or cells on preload
//

bool WorldManager::addObject(Object* object,bool manual)
{
    uint64 key = object->getId();

    //make sure objects arnt added several times!!!!
    if(getObjectById(key))
    {
        LOG(INFO) << "WorldManager::addObject Object(" << key<<") already exists added several times or ID messup ???";
        return false;
    }

    mObjectMap.insert(key,object);

    // if we want to set the parent manually or the object is from the snapshots and not a building, return
    if(manual)
    {
        return true;
    }

#if defined(_MSC_VER)
    if(object->getId() < 0x0000000100000000 && object->getType() != ObjType_Building)
#else
    if(object->getId() < 0x0000000100000000LLU && object->getType() != ObjType_Building)
#endif
	{
		// check if a crafting station - in that case add
		Item* item = dynamic_cast<Item*> (object);

		if(item)
		{
			if(!(item->getItemFamily() == ItemFamily_CraftingStations))
				return true;
		}
		else
		{
			return true;
		}
	}

	switch(object->getType())
	{
		// player, when a player enters a planet
		case ObjType_Player:
		{

			PlayerObject* player = dynamic_cast<PlayerObject*>(object);

			LOG(INFO) << "New Player: " << player->getId() << ", Total Players on zone " << (getPlayerAccMap())->size() + 1;
			// insert into the player map
			mPlayerAccMap.insert(std::make_pair(player->getAccountId(),player));			
			
			// not used currently - use with grid regions later ??
			player->setSubZoneId(0);
			

			// add ham to regeneration scheduler
			player->getHam()->updateRegenRates();	// ERU: Note sure if this is needed here.
			player->getHam()->checkForRegen();
			player->getStomach()->checkForRegen();

			// onPlayerEntered event, notify scripts
            std::stringstream params;
			params << getPlanetNameThis() << " " << player->getFirstName().getAnsi() 
                    << " " << static_cast<uint32>(mPlayerAccMap.size());

			mWorldScriptsListener.handleScriptEvent("onPlayerEntered",params.str().c_str());

		}
		break;

		case ObjType_Structure:
		{
	
			mStructureList.push_back(object->getId());
			
			//create the building in the world
			gSpatialIndexManager->createInWorld(object);

		}
		break;

		case ObjType_Building:
		{
			mStructureList.push_back(object->getId());
			BuildingObject* building = dynamic_cast<BuildingObject*>(object);
			
			//create the building in the world
			gSpatialIndexManager->createInWorld(object);
		}
		break;


		case ObjType_Cell:
		{
		}
		break;

		case ObjType_Tangible:
		{

			//create the tangible in the world
			gSpatialIndexManager->createInWorld(object);
			
		}
		break;

		case ObjType_NPC:
		case ObjType_Creature:
		case ObjType_Lair:
		{
			CreatureObject* creature = dynamic_cast<CreatureObject*>(object);

			if(creature->getCreoGroup() == CreoGroup_Shuttle)
				mShuttleList.push_back(dynamic_cast<Shuttle*>(creature));

			//create the creature in the world
			gSpatialIndexManager->createInWorld(creature);
			
		}
		break;

		case ObjType_Intangible:
		{
			//they dont get added here in the firstplace ...
			DLOG(INFO) << "Object of type ObjType_Intangible UNHANDLED in WorldManager::addObject:";
		}
		break;

		default:
		{
			// Please, when adding new stufff, at least take the time to add a stub for that type.
		}
		break;
	}

	return true;
}
bool WorldManager::addObject(std::shared_ptr<Object> object, bool manual)
{
    uint64 key = object->getId();

    mObjectMap.insert(key,object.get());
    
	if(manual)
		return true;

	switch(object->getType())
	{
		//kets add our Region to the zmap
		case ObjType_Region:
		{

			//mRegionMap.insert(std::make_pair(key,object));
			std::shared_ptr<RegionObject> region = std::static_pointer_cast<RegionObject>(object);
			gSpatialIndexManager->addRegion(region);

		}
		break;
	}
	
	return true;
}
//======================================================================================================================
// WorldManager::destroyObject(Object* object) removes an Object out of the main Object list
// the db is NOT touched; just stop any subsystems and / or prepare removal
// SpatialIndexManager::RemoveObjectFromWorld removes an object from the world (grid/cell) and sends destroys
void WorldManager::destroyObject(Object* object)
{
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

				if(nearestBuilding)
				{
					if(nearestBuilding->getSpawnPoints()->size())
					{
						if(SpawnPoint* sp = nearestBuilding->getRandomSpawnPoint())
						{
							// update the database with the new values
							gWorldManager->getDatabase()->executeSqlAsync(0,0,"UPDATE %s.characters SET parent_id=%" PRIu64 ",oX=%f,oY=%f,oZ=%f,oW=%f,x=%f,y=%f,z=%f WHERE id=%" PRIu64 "",mDatabase->galaxy(),sp->mCellId
								,sp->mDirection.x,sp->mDirection.y,sp->mDirection.z,sp->mDirection.w
								,sp->mPosition.x,sp->mPosition.y,sp->mPosition.z
								,player->getId());
						}
					}
				}
			}

			gSpatialIndexManager->RemoveObjectFromWorld(player);

			// onPlayerLeft event, notify scripts
			std::stringstream params;
			params << getPlanetNameThis() << " " << player->getFirstName().getAnsi() 
                    << " " << static_cast<uint32>(mPlayerAccMap.size());

			mWorldScriptsListener.handleScriptEvent("onPlayerLeft",params.str().c_str());
			
			delete player->getClient();
			
			player->setClient(NULL);
			player->setConnectionState(PlayerConnState_Destroying);


		}
		break;

		case ObjType_NPC:
		case ObjType_Creature:
		{
			CreatureObject* creature = dynamic_cast<CreatureObject*>(object);

			// remove any timers we got running
			removeCreatureHamToProcess(creature->getHam()->getTaskId());

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

		case ObjType_Waypoint:
		{
			uint64 parentId = object->getParentId();
			
			Datapad* pad = dynamic_cast<Datapad*>(gWorldManager->getObjectById(parentId+DATAPAD_OFFSET));
			
			//update the datapad
			if(!pad || !(pad->removeWaypoint(object->getId())))
			{
				DLOG(WARNING) << "Worldmanager::destroyObject: Error removing Waypoint from datapad " << parentId;
				return;
			}

			PlayerObject* owner = dynamic_cast<PlayerObject*>(getObjectById(pad->getParentId()));
			if(owner)
			{
				gMessageLib->sendUpdateWaypoint(dynamic_cast<WaypointObject*>(object),ObjectUpdateDelete,owner);
			}

			//waypoints are not part of the main Objectmap
			delete(object);
			return;
		}
		break;

		case ObjType_Intangible:
		{
			uint64 parentId = object->getParentId();
			
			Datapad* pad = dynamic_cast<Datapad*>(gWorldManager->getObjectById(parentId));
			
			//update the datapad
			if(!pad || !(pad->removeData(object->getId())))
			{
				DLOG(WARNING) << "WorldManager::destroyObject : Error removing Data from datapad " << object->getId();
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
			DLOG(WARNING) << "Unhandled ObjectType in WorldManager::destroyObject: " << (uint32)(object->getType());

			// Please, when adding new stufff, at least take the time to add a stub for that type.
			// Better fail always, than have random crashes.
			assert(false && "WorldManager::destroyObject Unhandled ObjectType");
		}
		break;
	}


	// finally delete it
	ObjectMap::iterator objMapIt = mObjectMap.find(object->getId());

	if(objMapIt != mObjectMap.end())
	{
		mObjectMap.erase(objMapIt);
	}
	else
	{
		delete(object);
		DLOG(WARNING) << "WorldManager::destroyObject: error removing from objectmap: " << object->getId();
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
	}


	// finally delete it
	ObjectMap::iterator objMapIt = mObjectMap.find(object->getId());

	if(objMapIt != mObjectMap.end())	{
		mObjectMap.erase(objMapIt);
	}
	
}

//======================================================================================================================
//
// simply erase an object ID out of the worlds ObjectMap *without* accessing the object
// I am aware this is somewhat a hack, though it is necessary, that the worldobjectlist can provide
// the objectcontroller with the IDs for the items in our datapad and inventory
// proper ObjectOwnership would normally require that these objects dont get added to the worlds object list
// perhaps it is possible to update the ObjectController at some later point to search the characters inventory / datapad
// but please be advised that the same problems do apply to items in houses / hoppers/ chests
// the objectcontroller can only provide them with a menu when it knows how to find the relevant Object
//

void WorldManager::eraseObject(uint64 key)
{

    // finally delete it
    ObjectMap::iterator objMapIt = mObjectMap.find(key);

    if(objMapIt != mObjectMap.end())
    {
        mObjectMap.erase(objMapIt);
    }
    else
    {
        DLOG(INFO) << "WorldManager::destroyObject: error removing from objectmap: " << key;
    }
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
