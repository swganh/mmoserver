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

#include "SpatialIndexManager.h"
#include "WorldManager.h"
#include "AdminManager.h"
#include "Buff.h"
#include "BuffEvent.h"
#include "BuffManager.h"
#include "BuildingObject.h"
#include "CellObject.h"
#include "DataPad.h"
#include "HouseObject.h"
#include "PlayerObject.h"
#include "CharacterLoginHandler.h"
#include "Container.h"
#include "ConversationManager.h"
#include "CraftingSessionFactory.h"
#include "CraftingTool.h"
#include "CreatureSpawnRegion.h"
#include "GroupManager.h"
#include "GroupObject.h"
#include "Inventory.h"
#include "Heightmap.h"
#include "MissionManager.h"
#include "MountObject.h"
#include "NpcManager.h"
#include "NPCObject.h"
#include "PlayerStructure.h"
#include "ResourceCollectionManager.h"
#include "ResourceManager.h"
#include "SchematicManager.h"
#include "TreasuryManager.h"
#include "Terminal.h"
#include "VehicleController.h"
#include "WorldConfig.h"
#include "WaypointObject.h"
#include "ZoneOpcodes.h"
#include "ZoneServer.h"
#include "RegionObject.h"
#include "HarvesterFactory.h"
#include "HarvesterObject.h"
#include "FactoryFactory.h"
#include "FactoryObject.h"
#include "Inventory.h"
#include "MissionObject.h"
#include "ObjectFactory.h"
#include "Shuttle.h"
#include "TicketCollector.h"
#include "ConfigManager/ConfigManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "MessageLib/MessageLib.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"
#include "Utils/Scheduler.h"
#include "Utils/VariableTimeScheduler.h"
#include "Utils/utils.h"
#include "Common/MessageFactory.h"

#include <cassert>

//======================================================================================================================
//
// returns the id of the first object that has a private owner that match the requested one.
//
// This function is not used yet.
uint64 WorldManager::getObjectOwnedBy(uint64 theOwner)
{
	gLogger->log(LogManager::DEBUG,"WorldManager::getObjectOwnedBy: Invoked");
	ObjectMap::iterator it = mObjectMap.begin();
	uint64 ownerId = 0;

	while (it != mObjectMap.end())
	{
		if ( ((*it).second)->getPrivateOwner() == theOwner)
		{
			ownerId = (*it).first;
			gLogger->log(LogManager::DEBUG,"WorldManager::getObjectOwnedBy: Found an object with id = %"PRIu64"", ownerId);
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
		gLogger->log(LogManager::NOTICE,"WorldManager::addObject Object already existant added several times or ID messup ???");
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

			gLogger->log(LogManager::DEBUG,"New Player: %"PRIu64", Total Players on zone : %i",player->getId(),(getPlayerAccMap())->size() + 1);
			// insert into the player map
			mPlayerAccMap.insert(std::make_pair(player->getAccountId(),player));			
			
			// not used currently - use with grid regions later ??
			player->setSubZoneId(0);
			

			// add ham to regeneration scheduler
			player->getHam()->updateRegenRates();	// ERU: Note sure if this is needed here.
			player->getHam()->checkForRegen();
			player->getStomach()->checkForRegen();

			// onPlayerEntered event, notify scripts
			string params;
			params.setLength(sprintf(params.getAnsi(),"%s %s %u",getPlanetNameThis(),player->getFirstName().getAnsi(),static_cast<uint32>(mPlayerAccMap.size())));

			mWorldScriptsListener.handleScriptEvent("onPlayerEntered",params);

			//******************************************************************
			//just do this once when the player is created the first time!!!
			//the spatialIndexhandler will be called whenever we teleport or change planet!
			//these functions will be moved to a containerhandler at some point
			gSpatialIndexManager->registerPlayerToContainer(player,player);
			gSpatialIndexManager->registerPlayerToContainer(player->getInventory(),player);

	
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

		case ObjType_Region:
		{
			RegionObject* region = dynamic_cast<RegionObject*>(object);

			mRegionMap.insert(std::make_pair(key,region));

			gSpatialIndexManager->addRegion(region);

			if(region->getActive())
				addActiveRegion(region);
		}
		break;

		case ObjType_Intangible:
		{
			//they dont get added here in the firstplace ...
			gLogger->log(LogManager::NOTICE,"Object of type ObjType_Intangible UNHANDLED in WorldManager::addObject:");
		}
		break;

		default:
		{
			gLogger->log(LogManager::CRITICAL,"Unhandled ObjectType in WorldManager::addObject: PRId32",object->getType());
			// Please, when adding new stufff, at least take the time to add a stub for that type.
			// Better fail always, than have random crashes.
			assert(false && "WorldManager::addObject Unhandled ObjectType");
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

			gSpatialIndexManager->RemoveObjectFromWorld(player);


			// onPlayerLeft event, notify scripts
			string params;
			params.setLength(sprintf(params.getAnsi(),"%s %s %u",getPlanetNameThis(),player->getFirstName().getAnsi(),static_cast<uint32>(mPlayerAccMap.size())));

			mWorldScriptsListener.handleScriptEvent("onPlayerLeft",params);
			
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
				building->prepareDestruction();
				
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
			else
				gLogger->log(LogManager::DEBUG,"WorldManager::destroyObject: nearly did not remove: %"PRIu64"s knownObjectList",object->getId());


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
			
			if(object->getId() == 2533274790395904)
			{
				gLogger->log(LogManager::CRITICAL," WorldManager::destroyObject: %I64u",(object->getId()));
			}
		}
		break;

		case ObjType_Region:
		{
			RegionMap::iterator it = mRegionMap.find(object->getId());

			if(it != mRegionMap.end())
			{
				mRegionMap.erase(it);
			}
			else
			{
				gLogger->log(LogManager::DEBUG,"Worldmanager::destroyObject: Could not find region %"PRIu64"",object->getId());
			}

			gSpatialIndexManager->RemoveRegion(dynamic_cast<RegionObject*>(object));

		}
		break;

		case ObjType_Waypoint:
		{
			uint64 parentId = object->getParentId();
			
			Datapad* pad = dynamic_cast<Datapad*>(gWorldManager->getObjectById(parentId));
			
			//update the datapad
			if(!pad || !(pad->removeData(object->getId())))
			{
				gLogger->log(LogManager::DEBUG,"Worldmanager::destroyObject: Error removing Waypoint from datapad %"PRIu64"",pad->getId());
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
				gLogger->log(LogManager::DEBUG,"WorldManager::destroyObject : Error removing Data from datapad %"PRIu64"",object->getId());
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
			gLogger->log(LogManager::CRITICAL,"Unhandled ObjectType in WorldManager::destroyObject: %u",(uint32)(object->getType()));

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
		gLogger->log(LogManager::CRITICAL,"WorldManager::destroyObject: error removing from objectmap: %"PRIu64"",object->getId());
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
		gLogger->log(LogManager::DEBUG,"WorldManager::destroyObject: error removing from objectmap: %"PRIu64"",key);
	}
}




//======================================================================================================================

void WorldManager::_loadAllObjects(uint64 parentId)
{
	int8	sql[2048];
	WMAsyncContainer* asynContainer = new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_AllObjectsChildObjects);

	sprintf(sql,"(SELECT \'terminals\',terminals.id FROM terminals INNER JOIN terminal_types ON (terminals.terminal_type = terminal_types.id)"
				" WHERE (terminal_types.name NOT LIKE 'unknown') AND (terminals.parent_id = %"PRIu64") AND (terminals.planet_id = %"PRIu32"))"
				" UNION (SELECT \'containers\',containers.id FROM containers INNER JOIN container_types ON (containers.container_type = container_types.id)"
				" WHERE (container_types.name NOT LIKE 'unknown') AND (containers.parent_id = %"PRIu64") AND (containers.planet_id = %u))"
				" UNION (SELECT \'ticket_collectors\',ticket_collectors.id FROM ticket_collectors WHERE (parent_id=%"PRIu64") AND (planet_id=%u))"
				" UNION (SELECT \'persistent_npcs\',persistent_npcs.id FROM persistent_npcs WHERE (parentId=%"PRIu64") AND (planet_id = %"PRIu32"))"
				" UNION (SELECT \'shuttles\',shuttles.id FROM shuttles WHERE (parentId=%"PRIu64") AND (planet_id = %"PRIu32"))"
				" UNION (SELECT \'items\',items.id FROM items WHERE (parent_id=%"PRIu64") AND (planet_id = %"PRIu32"))"
				" UNION (SELECT \'resource_containers\',resource_containers.id FROM resource_containers WHERE (parent_id=%"PRIu64") AND (planet_id = %"PRIu32"))",
				parentId,mZoneId,parentId,mZoneId,parentId,mZoneId,parentId,mZoneId,parentId
				,mZoneId,parentId,mZoneId,parentId,mZoneId);

	mDatabase->ExecuteSqlAsync(this,asynContainer,sql);

	//gConfig->read<float>("FillFactor"
}

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
