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
#include "ZoneServer/GameSystemManagers/Structure Manager/StructureManager.h"

#include "anh/logger.h"

#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/Objects/PlayerStructureTerminal.h"
#include "FactoryFactory.h"
#include "ZoneServer/Objects/nonPersistantObjectFactory.h"
#include "HarvesterObject.h"
#include <ZoneServer/GameSystemManagers/Resource Manager/Resource.h>
#include <ZoneServer/GameSystemManagers/Structure Manager/HouseObject.h>
#include "ZoneServer/GameSystemManagers/Structure Manager/FactoryObject.h"
#include "Zoneserver/Objects/Inventory.h"
#include "Zoneserver/Objects/Datapad.h"
#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceContainer.h"
#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceType.h"
#include "ZoneServer/Objects/Object/ObjectFactory.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/ManufacturingSchematic.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "PlayerStructure.h"

#include "ZoneServer/WorldManager.h"
#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"
#include "Zoneserver/Objects/RegionObject.h"
#include "MessageLib/MessageLib.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"
#include "Zoneserver/Objects/Deed.h"

#include "ZoneServer\Services\equipment\equipment_service.h"

#include "Common/OutOfBand.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "anh/Utils/rand.h"
#include "Utils/MathFunctions.h"

#include <ZoneServer\Services\terrain\terrain_service.h>
#include <anh\app\swganh_kernel.h>
#include <anh\service/service_manager.h>


#include <cassert>

using ::common::OutOfBand;

bool						StructureManager::mInsFlag    = false;
StructureManager*			StructureManager::mSingleton  = NULL;


//======================================================================================================================

StructureManager::StructureManager(swganh::database::Database* database,MessageDispatch* dispatch)
{
    LOG(info) << "Beginning structure manager initialization";
    
    mBuildingFenceInterval = gWorldConfig->getConfiguration<uint16>("Zone_BuildingFenceInterval",(uint16)10000);
    //uint32 structureCheckIntervall = gWorldConfig->getConfiguration("Zone_structureCheckIntervall",(uint32)3600);
    uint32 structureCheckIntervall = gWorldConfig->getConfiguration<uint32>("Zone_structureCheckIntervall",(uint32)30);

    mDatabase = database;
    mMessageDispatch = dispatch;
    StructureManagerAsyncContainer* asyncContainer;

    // load our structure data
    //todo load buildings from building table and use appropriate stfs there
    //are harvesters on there too
    asyncContainer = new StructureManagerAsyncContainer(Structure_Query_LoadDeedData, 0);
    std::stringstream sql;
	sql << "SELECT sdd.id, sdd.DeedType, sdd.SkillRequirement, s_td.object_string, s_td.lots_used, s_td.stf_name, s_td.stf_file, s_td.healing_modifier, s_td.repair_cost, s_td.fp_length, s_td.fp_width, s_td.planetMask from "
		<< mDatabase->galaxy() << ".structure_deed_data sdd INNER JOIN "
		<< mDatabase->galaxy() << ".structure_type_data s_td ON sdd.StructureType = s_td.type";
	mDatabase->executeSqlAsync(this,asyncContainer,sql.str());

    //items
    asyncContainer = new StructureManagerAsyncContainer(Structure_Query_LoadstructureItem, 0);
    mDatabase->executeSqlAsync(this,asyncContainer,"SELECT sit.structure_id, sit.cell, sit.item_type , sit.relX, sit.relY, sit.relZ, sit.dirX, sit.dirY, sit.dirZ, sit.dirW, sit.tan_type, it.object_string, it.stf_name, it.stf_file from %s.structure_item_template sit INNER JOIN %s.item_types it ON (it.id = sit.item_type) WHERE sit.tan_type = %u",mDatabase->galaxy(),mDatabase->galaxy(),TanGroup_Item);

    //statics
    asyncContainer = new StructureManagerAsyncContainer(Structure_Query_LoadstructureItem, 0);

    mDatabase->executeSqlAsync(this,asyncContainer,"SELECT sit.structure_id, sit.cell, sit.item_type , sit.relX, sit.relY, sit.relZ, sit.dirX, sit.dirY, sit.dirZ, sit.dirW, sit.tan_type, st.object_string, st.name, st.file from %s.structure_item_template sit INNER JOIN %s.static_types st ON (st.id = sit.item_type) WHERE sit.tan_type = %u",mDatabase->galaxy(),mDatabase->galaxy(),TanGroup_Static);


    //terminals
    asyncContainer = new StructureManagerAsyncContainer(Structure_Query_LoadstructureItem, 0);
    mDatabase->executeSqlAsync(this,asyncContainer,"SELECT sit.structure_id, sit.cell, sit.item_type , sit.relX, sit.relY, sit.relZ, sit.dirX, sit.dirY, sit.dirZ, sit.dirW, sit.tan_type, tt.object_string, tt.name, tt.file from %s.structure_item_template sit INNER JOIN %s.terminal_types tt ON (tt.id = sit.item_type) WHERE sit.tan_type = %u",mDatabase->galaxy(),mDatabase->galaxy(),TanGroup_Terminal);

    // load our NoBuildRegions
    asyncContainer = new StructureManagerAsyncContainer(Structure_Query_NoBuildRegionData, 0);
    mDatabase->executeProcedureAsync(this,asyncContainer,"CALL %s.sp_PlanetNoBuildRegions", mDatabase->galaxy());

    //=========================
    //check regularly the harvesters - they might have been turned off by the db, harvesters without condition might need to be deleted
    //do so every hour if no other timeframe is set
    gWorldManager->getPlayerScheduler()->addTask(fastdelegate::MakeDelegate(this,&StructureManager::_handleStructureDBCheck),7,structureCheckIntervall*1000,NULL);
    
    LOG(info) << "Structure Manager initialization complete";
}


//======================================================================================================================
StructureManager::~StructureManager()
{
    mInsFlag = false;
    delete(mSingleton);

}
//======================================================================================================================
StructureManager*	StructureManager::Init(swganh::database::Database* database, MessageDispatch* dispatch)
{
    if(!mInsFlag)
    {
        mSingleton = new StructureManager(database,dispatch);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;

}

//======================================================================================================================

void StructureManager::Shutdown()
{

}


//=======================================================================================================================
// after altering a structures permission list we will update the permissions of the players
// on the structures KnownObjectsList
//

void StructureManager::updateKownPlayerPermissions(PlayerStructure* structure)
{
    HouseObject* house = dynamic_cast<HouseObject*>(structure);
    if(!house)    {
        LOG(warning) << "Structure is not a HouseObject";
        return;
    }

	//get all players in range and alter their permissionlists
	PlayerObjectSet resultSet;

	gSpatialIndexManager->getPlayersInRange(structure,&resultSet, true);
	PlayerObjectSet::iterator it = resultSet.begin();

	while(it != resultSet.end())	{
		PlayerObject* player = dynamic_cast<PlayerObject*>(*it);
		
		if(player)	{
			house->checkCellPermission(player);
		}
		it++;
	}
}

//=======================================================================================================================
//checks for a name on a permission list
//=======================================================================================================================
void StructureManager::checkNameOnPermissionList(uint64 structureId, uint64 playerId, std::string name_ansi, std::string list, StructureAsyncCommand command)
{

    StructureManagerAsyncContainer* asyncContainer;

    asyncContainer = new StructureManagerAsyncContainer(Structure_Query_Check_Permission, 0);

	std::stringstream sql;
	sql << "SELECT " << mDatabase->galaxy() << ".sf_CheckPermissionList(" << structureId << ",'" << mDatabase->escapeString(name_ansi)
		<< "','" << list << "')";

    mDatabase->executeSqlAsync(this,asyncContainer,sql.str());

    asyncContainer->mStructureId = structureId;
    asyncContainer->mPlayerId = playerId;
    asyncContainer->command = command;
    sprintf(asyncContainer->name,"%s",name_ansi);


    // 0 is Name on list
    // 1 name doesnt exist
    // 2 name not on list
    // 3 Owner
}


//=======================================================================================================================
//removes a name from a permission list
//=======================================================================================================================
void StructureManager::removeNamefromPermissionList(uint64 structureId, uint64 playerId,  std::string name_ansi, BString list)
{
    StructureManagerAsyncContainer* asyncContainer;

    asyncContainer = new StructureManagerAsyncContainer(Structure_Query_Remove_Permission, 0);
	
	std::stringstream sql;
	sql << "SELECT " << mDatabase->galaxy() << ".sf_RemovePermissionList(" << structureId << ",'" << mDatabase->escapeString(name_ansi)
		<< "','" << list.getAnsi() << "')";

    mDatabase->executeSqlAsync(this,asyncContainer, sql.str());

    asyncContainer->mStructureId = structureId;
    asyncContainer->mPlayerId = playerId;
    sprintf(asyncContainer->name,"%s",name_ansi);

    // 0 is sucess
    // 1 name doesnt exist
    // 2 name not on list
    // 3 Owner cannot be removed
}



//=======================================================================================================================
//adds a name to a permission list
//=======================================================================================================================
void StructureManager::addNametoPermissionList(uint64 structureId, uint64 playerId,  std::string name_ansi, BString list)
{
    //we have shown that we are on the admin list, so the name we proposed now will get added

    StructureManagerAsyncContainer* asyncContainer;

    asyncContainer = new StructureManagerAsyncContainer(Structure_Query_Add_Permission, 0);

    asyncContainer->mStructureId = structureId;
    asyncContainer->mPlayerId = playerId;
    sprintf(asyncContainer->name,"%s",name_ansi.c_str());
	
	std::stringstream sql;
	sql << "SELECT " << mDatabase->galaxy() << ".sf_AddPermissionList(" << structureId << ",'" << mDatabase->escapeString(name_ansi)
		<< "','" << list.getAnsi() << "')";
    mDatabase->executeSqlAsync(this,asyncContainer,sql.str());


    // 0 is sucess
    // 1 name doesnt exist
    // 2 name already on list


}



//=======================================================================================================================
//handles callbacks of db creation of items
//=======================================================================================================================
void StructureManager::getDeleteStructureMaintenanceData(uint64 structureId, uint64 playerId)
{
    // load our structures maintenance data
    // that means the maintenance attribute and the energy attribute

    StructureManagerAsyncContainer* asyncContainer;
    asyncContainer = new StructureManagerAsyncContainer(Structure_UpdateAttributes, 0);
    mDatabase->executeSqlAsync(this,asyncContainer, "(SELECT \'power\', sa.value FROM %s.structure_attributes sa WHERE sa.structure_id = %"PRIu64" AND sa.attribute_id = 384) UNION (SELECT \'maintenance\'	, sa.value FROM structure_attributes sa WHERE sa.structure_id = %"PRIu64" AND sa.attribute_id = 382)  ",mDatabase->galaxy(),structureId, structureId);


    asyncContainer->mStructureId = structureId;
    asyncContainer->mPlayerId = playerId;
    asyncContainer->command.Command = Structure_Command_Destroy;

    //382 = examine_maintenance
    //384 = examine_power
}


//======================================================================================================================
//looks up the data for a specific deed
//======================================================================================================================

StructureDeedLink* StructureManager::getDeedData(uint32 type)
{
    DeedLinkList::iterator it = mDeedLinkList.begin();
    //bool found = false;
    while(it != mDeedLinkList.end())
    {
        if ((*it)->item_type == type )
        {
            return (*it);
        }
        it++;
    }

    return NULL;
}

//======================================================================================================================
// returns true when we are NOT within 25m of a campregion!!
// so that camps are not to close to each other

bool StructureManager::checkCampRadius(PlayerObject* player)
{
	float				range  = 25.0;

	//search the grids subcells
	RegionObject*	object;
	ObjectSet		objList;

	gSpatialIndexManager->getObjectsInRange(player,&objList,ObjType_Region,range*2, false);

	
	ObjectSet::iterator objIt = objList.begin();

    while(objIt != objList.end())
    {
        object = (RegionObject*)(*objIt);

        if(object->getRegionType() == Region_Camp)
        {
            return false;
        }

        ++objIt;
    }

    return true;

}

//======================================================================================================================
//returns true when we are NOT within 5m of a city
//======================================================================================================================

bool StructureManager::checkCityRadius(PlayerObject* player)
{
	float				range  = 5.0;

	//search the grids subcells
	RegionObject*	object;
	ObjectSet		objList;

	gSpatialIndexManager->getObjectsInRange(player,&objList,ObjType_Region,range*2, false);

    ObjectSet::iterator objIt = objList.begin();

    while(objIt != objList.end())
    {
        object = (RegionObject*)(*objIt);

        if(object->getRegionType() == Region_City)
        {
            return false;
        }

        ++objIt;
    }

    return true;

}

//======================================================================================================================
//returns true when we are within a camp
//======================================================================================================================

bool StructureManager::checkinCamp(PlayerObject* player)
{
	float				range  = 1.0;

	//search the grids subcells
	RegionObject*	object;
	ObjectSet		objList;

	gSpatialIndexManager->getObjectsInRange(player,&objList,ObjType_Region,range*2, false);

    ObjectSet::iterator objIt = objList.begin();

    while(objIt != objList.end())
    {
        object = (RegionObject*)(*objIt);

        if(object->getRegionType() == Region_Camp)
        {
            return true;
        }

        ++objIt;
    }

    return false;

}
//======================================================================================================================
//override
//returns true if we're in a no build region
bool StructureManager::checkNoBuildRegion(PlayerObject* player)
{
	if (checkNoBuildRegion(player->GetCreature()->mPosition) /*||!checkCityRadius(player)*/)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("faction_perk", "no_build_area"), player);
        return true;
    }

    return false;
}
//======================================================================================================================
//returns true if we're in a no build region
//======================================================================================================================
bool StructureManager::checkNoBuildRegion(const glm::vec3& vec3)
{
    NoBuildRegionList* regionList = gStructureManager->getNoBuildRegionList();
    NoBuildRegionList::iterator it = regionList->begin();
    while(it != regionList->end())
    {
        if (gWorldManager->getZoneId() == (*it)->planet_id)
        {
            float pX = vec3.x;
            float pZ = vec3.z;
            float rX = (*it)->mPosition.x;
            float rZ = (*it)->mPosition.z;

            float height = (*it)->height;
            float width = (*it)->width;
            float radiusSq = (*it)->mRadiusSq;

            // are we a circle
            if ((*it)->isCircle)
            {
                // formula, we do it this way to avoid the costly square root
                if ((((pX - rX)*(pX - rX)) + ((pZ - rZ)*(pZ - rZ))) <= radiusSq)
                {
                    return true;
                }
            }
            else
            {
                // this is a rectangle region.

                // Convert the player position to a vec 2.
                glm::vec2 player_position(vec3.x, vec3.z);
                glm::vec2 region_center(rX, rZ);

                if (IsPointInRectangle(player_position, region_center, width, height)) {
                    return true;
                }
            }
        }
        ++it;
    }
    return false;
}
//=========================================================================================0
// gets the code to confirm structure destruction
//
BString StructureManager::getCode()
{
    int8	serial[12],chance[9];
    bool	found = false;
    uint8	u;

    for(uint8 i = 0; i < 6; i++)
    {
        while(!found)
        {
            found = true;
            u = static_cast<uint8>(static_cast<double>(gRandom->getRand()) / (RAND_MAX + 1.0f) * (122.0f - 48.0f) + 48.0f);

            //only 1 to 9 or a to z
            if(u >57)
                found = false;

            if(u < 48)
                found = false;

        }
        chance[i] = u;
        found = false;
    }
    chance[6] = 0;

    sprintf(serial,"%s",chance);

    return(BString(serial));
}

//======================================================================================================================
//
// Handle deletion of destroyed Structures / building fences and other stuff
//

bool StructureManager::_handleStructureObjectTimers(uint64 callTime, void* ref)
{
	//iterate through all harvesters to delete
	ObjectIDList* objectList = gStructureManager->getStrucureDeleteList();
	ObjectIDList::iterator it = objectList->begin();

	while(it != objectList->end())
	{
		PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById((*it)));

		if(!structure)
		{
			DLOG(info) << "StructureManager::_handleStructureObjectTimers: No structure";
			it = objectList->erase(it);
			continue;
		}


		if(structure->getTTS()->todo == ttE_Delete)
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById( structure->getTTS()->playerId ));
			if(structure->canRedeed())
			{	
				auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
				auto inventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(player->GetCreature(), "inventory"));

				if(!inventory->checkSlots(1))				{
					gMessageLib->SendSystemMessage(std::u16string(),player,"player_structure","inventory_full");
					it = objectList->erase(it);
					continue;
				}

				gMessageLib->SendSystemMessage(std::u16string(),player,"player_structure","deed_reclaimed");

				//update the deeds attributes and set the new owner id (owners inventory = characterid +1)
				StructureManagerAsyncContainer* asyncContainer;
				asyncContainer = new StructureManagerAsyncContainer(Structure_UpdateStructureDeed, 0);
				asyncContainer->mPlayerId		= structure->getOwner();
				asyncContainer->mStructureId	= structure->getId();
				int8 sql[150];
				//this will update the deeds attributes an change deed ownership in the db
				sprintf(sql, "select %s.sf_DefaultHarvesterUpdateDeed(%"PRIu64",%"PRIu64")", mDatabase->galaxy(),  structure->getId(),structure->getOwner()+INVENTORY_OFFSET);
				mDatabase->executeSqlAsync(this,asyncContainer,sql);

			}
			else
			//delete the deed
			{

				gMessageLib->SendSystemMessage(std::u16string(),player,"player_structure","structure_destroyed");
				
				//deletes the deed
				int8 sql[200];
				sprintf(sql,"DELETE FROM %s.items WHERE parent_id = %"PRIu64" AND item_family = 15",mDatabase->galaxy(),structure->getId());
				mDatabase->executeSqlAsync(NULL,NULL,sql);
				
				gObjectFactory->deleteObjectFromDB(structure);
				
				gSpatialIndexManager->RemoveObjectFromWorld(structure);
				gWorldManager->destroyObject(structure);
				
				UpdateCharacterLots(structure->getOwner());

			}
		}

		if(structure->getTTS()->todo == ttE_BuildingFence)
		{
			if(Anh_Utils::Clock::getSingleton()->getLocalTime() < structure->getTTS()->projectedTime)
			{
				DLOG(info) << "StructureManager::_handleStructureObjectTimers: intervall to short - delayed";
				break;
			}

			//gLogger->log(LogManager::DEBUG,"StructureManager::_handleStructureObjectTimers: building fence");

			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById( structure->getTTS()->playerId ));
			PlayerStructure* fence = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(structure->getTTS()->buildingFence));

			if(!player)
			{
												
				gSpatialIndexManager->RemoveObjectFromWorld(fence);
				gWorldManager->destroyObject(fence);

				gWorldManager->handleObjectReady(structure,player->getClient());
				
				it = objectList->erase(it);
				continue;
			}

			if(!fence)
			{
				DLOG(info) << "StructureManager::_handleStructureObjectTimers: No fence";
				it = objectList->erase(it);
				continue;
				return false;
			}

			//destroy the fence
			gSpatialIndexManager->RemoveObjectFromWorld(fence);
			gWorldManager->destroyObject(fence);

			//create the structure in the world
			gSpatialIndexManager->createInWorld(structure);

			//send the EMail
			gMessageLib->sendConstructionComplete(player,structure);

			/*
			if(structure->getPlayerStructureFamily() == PlayerStructure_House)
			{
				float x,y,z;
				x = structure->mPosition.x+7;
				y = structure->mPosition.y+1;
				z = structure->mPosition.z+7;
				PlayerStructure* sign = gNonPersistantObjectFactory->requestBuildingSignObject(x,y,z,player,"sign","sign_name",structure->getCustomName());
				HouseObject* house = dynamic_cast<HouseObject*>(structure);
				house->setSign(sign);
			}
			*/
			

		}


		it = objectList->erase(it);
	
	}

	return (false);
}




//=======================================================================================================================
//processes a succesfull PermissionList verification
//=======================================================================================================================

void StructureManager::processVerification(StructureAsyncCommand command, bool owner)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(command.PlayerId));

	if(!player)
	{
		DLOG(info) << "StructureManager::processVerification : No Player";
		return;
	}

	switch(command.Command)
	{

		case Structure_Command_CellEnterDenial:
		{
			//our structure is public
			//we just queried the ban list
			//make sure were not the owner, however
			if(owner)
			{
				//building->updateCellPermissions(player,true);
				return;
			}

			BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(command.StructureId));
			if(!building)
			{
				DLOG(info) << "StructureManager::processVerification : No building (Structure_Command_CellEnterDenial) ";
				return;
			}
			//now send cell permission update to the player

			building->updateCellPermissions(player,false);
		
		}
		break;

		case Structure_Command_CellEnter:
		{
			//we just queried the access list as the structure is private
			//owner and everyone on entry and admin list may enter
			
			BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(command.StructureId));
			if(!building)
			{
				DLOG(info) << "StructureManager::processVerification : No building (Structure_Command_CellEnter) ";
				return;
			}
			//now send cell permission update to the player

			building->updateCellPermissions(player,true);
		
		}
		break;
		
		case Structure_Command_StopFactory:
		{
			FactoryObject* factory = dynamic_cast<FactoryObject*>(gWorldManager->getObjectById(command.StructureId));
			if(!factory)
			{
				DLOG(info) << "StructureManager::processVerification : No Factory (Structure_Command_StopFactory) ";
				return;
			}

			gMessageLib->SendSystemMessage(L"You stop manufacturing items", player);
			factory->setActive(false);

			//now turn the factory on - in db and otherwise
			mDatabase->executeSqlAsync(0,0,"UPDATE %s.factories f SET f.active = 0 WHERE f.ID = %I64u",mDatabase->galaxy(),command.StructureId);
			gMessageLib->SendUpdateFactoryWorkAnimation(factory);

		}
		break;

		case Structure_Command_StartFactory:
		{
			FactoryObject* factory = dynamic_cast<FactoryObject*>(gWorldManager->getObjectById(command.StructureId));
			if(!factory)
			{
				DLOG(info) << "StructureManager::processVerification : No Factory (Structure_Command_AccessInHopper) ";
				return;
			}

			//is a schematic installed?
			if(!factory->getManSchemID())
			{
				gMessageLib->SendSystemMessage(L"You need to add a schematic before you can start producing items.", player);
				DLOG(info) << "StructureManager::processVerification : No Factory (Structure_Command_AccessInHopper) ";
				return;
			}

			gMessageLib->SendSystemMessage(L"You start manufacturing items", player);
			factory->setActive(true);

			//now turn the factory on - in db and otherwise
			mDatabase->executeSqlAsync(0,0,"UPDATE %s.factories f SET f.active = 1 WHERE f.ID = %I64u",mDatabase->galaxy(),command.StructureId);
			gMessageLib->SendUpdateFactoryWorkAnimation(factory);

		}
		break;

		case Structure_Command_AccessOutHopper:
		{
		 	FactoryObject* factory = dynamic_cast<FactoryObject*>(gWorldManager->getObjectById(command.StructureId));
			if(!factory)
			{
				DLOG(info) << "StructureManager::processVerification : No Factory (Structure_Command_AccessInHopper) ";
				return;
			}

			Item* outHopper = dynamic_cast<Item*>(gWorldManager->getObjectById(factory->getOutputHopper()));
			if(!outHopper)
			{
				DLOG(info) << "StructureManager::processVerification : No outHopper (Structure_Command_AccessInHopper) ";
				return;
			}

			//create the outputhoppers contents


			gContainerManager->registerPlayerToContainer(outHopper,player);
			

			gFactoryFactory->upDateHopper(factory,factory->getOutputHopper(),player->getClient(),factory);
			return;


		}
		break;

		case Structure_Command_AccessInHopper:
		{
			FactoryObject* factory = dynamic_cast<FactoryObject*>(gWorldManager->getObjectById(command.StructureId));
			if(!factory)
			{
				DLOG(info) << "StructureManager::processVerification : No Factory (Structure_Command_AccessInHopper) ";
				return;
			}

			//send the hopper as tangible if we havnt done that already ...
			Item* inHopper = dynamic_cast<Item*>(gWorldManager->getObjectById(factory->getIngredientHopper()));
			if(!inHopper)
			{
				DLOG(info) << "StructureManager::processVerification : No inHopper (Structure_Command_AccessInHopper) ";
				return;
			}

			//now register the hopper with the player who opened it
			gContainerManager->registerPlayerToContainer(inHopper,player);


			gFactoryFactory->upDateHopper(factory,factory->getIngredientHopper(),player->getClient(),factory);
			return;
		
		}
		break;

		case Structure_Command_RemoveSchem:
		{
			FactoryObject* factory = dynamic_cast<FactoryObject*>(gWorldManager->getObjectById(command.StructureId));
			if(!factory)
			{
				DLOG(info) << "StructureManager::processVerification : No Factory (Structure_Command_AddSchem) ";
				return;
			}

			//do we have a schematic that needs to be put back into the inventory???

			if(!factory->getManSchemID())
			{
				//nothing to do for us
				return;
			}

			//return the old schematic to the Datapad
			Datapad* datapad			= player->getDataPad();
			//Inventory*	inventory	= dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
			
			if(!datapad->getCapacity())	{
				gMessageLib->SendSystemMessage(::common::OutOfBand("manf_station","schematic_not_removed"),player);
				return;
			}
			
			//change the ManSchems Owner ID and load it into the datapad
			gObjectFactory->requestTanoNewParent(datapad,factory->getManSchemID() ,datapad->getId(),TanGroup_ManufacturingSchematic);
			mDatabase->executeSqlAsync(0,0,"UPDATE %s.factories SET ManSchematicID = 0 WHERE ID = %I64u",mDatabase->galaxy(),command.StructureId);

			//finally reset the schem ID in the factory
			factory->setManSchemID(0);
			gMessageLib->SendSystemMessage(std::u16string(),player,"manf_station","schematic_removed");
			
		}
		break;

		case Structure_Command_AddSchem:
		{
			FactoryObject* factory = dynamic_cast<FactoryObject*>(gWorldManager->getObjectById(command.StructureId));
			if(!factory)	{
				gMessageLib->SendSystemMessage(::common::OutOfBand("manf_station","schematic_not_added"),player);
				DLOG(info) << "StructureManager::processVerification : No Factory (Structure_Command_AddSchem) ";
				return;
			}

			//do we have a schematic that needs to be put back into the inventory???

			if(factory->getManSchemID())
			{
				if(factory->getManSchemID() == command.SchematicId)
				{
					//nothing to do for us
					return;
				}

				//first return the old schematic to the Datapad
				Datapad* datapad			= player->getDataPad();
				//Inventory*	inventory	= dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
				
				//change the ManSchems Owner ID and load it into the datapad
				gObjectFactory->requestTanoNewParent(datapad,factory->getManSchemID() ,datapad->getId(),TanGroup_ManufacturingSchematic);
		
			}

			
			PlayerObject* player	= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(command.PlayerId));
			Datapad* datapad			= player->getDataPad();
			
			TangibleObject* tO = dynamic_cast<TangibleObject*>(datapad->getManufacturingSchematicById(command.SchematicId));
			if(!tO->hasInternalAttribute("craft_tool_typemask"))
			{
				gMessageLib->SendSystemMessage(L"old schematic it will be deprecated once factory schematic type checks are implemented", player);
				tO->addInternalAttribute("craft_tool_typemask","0xffffffff");
			}

			uint32 mask = tO->getInternalAttribute<uint32>("craft_tool_typemask");
			
			if((mask&&factory->getMask())!=mask)
			{
					gMessageLib->SendSystemMessage(L"this schematic will not fit into the factory anymore as soon as schematictype checks are implemented", player);
					
					int8 s[512];
					sprintf(s,"schematic Mask %u vs factory Mask %u",mask,factory->getMask());
					BString message(s);
					message.convert(BSTRType_Unicode16);
					gMessageLib->SendSystemMessage(message.getUnicode16(), player);
			}

			factory->setManSchemID(command.SchematicId);
			
			//link the schematic to the factory in the db
			mDatabase->executeSqlAsync(0,0,"UPDATE %s.factories SET ManSchematicID = %I64u WHERE ID = %I64u",mDatabase->galaxy(),command.SchematicId,command.StructureId);
			mDatabase->executeSqlAsync(0,0,"UPDATE %s.items SET parent_id = %I64u WHERE ID = %I64u",mDatabase->galaxy(),command.StructureId,command.SchematicId);
			
			//remove the schematic from the player
			

			datapad->removeManufacturingSchematic(command.SchematicId);
			gMessageLib->sendDestroyObject(command.SchematicId,player);

			
			gMessageLib->SendSystemMessage(common::OutOfBand("manf_station","schematic_added"),player);
			//gMessageLib->sendSystemMessage(player,
			
			//remove the added Manufacturing schematic
			datapad->removeManufacturingSchematic(command.SchematicId);

		}
		break;

		case Structure_Command_AccessSchem:
		{
			PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(command.StructureId));

			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_UpdateAttributes,player->getClient());
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			asyncContainer->command			= command;
			mDatabase->executeSqlAsync(this,asyncContainer,
				 "		(SELECT \'schematicCustom\', i.customName FROM %s.factories f INNER JOIN %s.items i ON (i.id = f.ManSchematicID) WHERE f.ID = %I64u)"
				 "UNION (SELECT \'schematicName\', it.stf_name FROM %s.factories f INNER JOIN %s.items i ON (i.id = f.ManSchematicID) INNER JOIN %s.item_types it ON (i.item_type = it.id) WHERE f.ID = %I64u)"
				 "UNION (SELECT \'schematicFile\', it.stf_file FROM %s.factories f INNER JOIN %s.items i ON (i.id = f.ManSchematicID) INNER JOIN %s.item_types it ON (i.item_type = it.id) WHERE f.ID = %I64u)",
				mDatabase->galaxy(),mDatabase->galaxy(),command.StructureId,
                mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),command.StructureId,
                mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),command.StructureId);
			
		}
		break;

		case Structure_Command_ViewStatus:
		{
			PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(command.StructureId));
			
			//the structure might have been deleted between the last and the current refresh
			if(!structure)	{
				gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure","no_valid_structurestatus"),player);
				return;
			}
			if(player->GetCreature()->getTargetId() != structure->getId())
			{
				PlayerStructureTerminal* terminal = dynamic_cast<PlayerStructureTerminal*>(gWorldManager->getObjectById(player->GetCreature()->getTargetId()));
				if(!terminal||(terminal->getStructure() != command.StructureId))	{
					gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure","changed_structurestatus"),player);
					return;
				}
			}

			//read the relevant attributes in then display the status page
			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_UpdateAttributes,player->getClient());
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			asyncContainer->command			= command;
			
			mDatabase->executeSqlAsync(this,asyncContainer,
				            "(SELECT \'name\', c.firstname  FROM %s.characters c WHERE c.id = %"PRIu64")"
				            "UNION (SELECT \'power\', sa.value FROM %s.structure_attributes sa WHERE sa.structure_id = %"PRIu64" AND sa.attribute_id = 384)"
				            "UNION (SELECT \'maintenance\', sa.value FROM %s.structure_attributes sa WHERE sa.structure_id = %"PRIu64" AND sa.attribute_id = 382) ",
                            mDatabase->galaxy(),structure->getOwner(),
                            mDatabase->galaxy(),command.StructureId,
                            mDatabase->galaxy(),command.StructureId);


			
		}
		break;

		case Structure_Command_DepositPower:
		{
			PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(command.StructureId));

			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_UpdateAttributes,player->getClient());
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			asyncContainer->command			= command;
			mDatabase->executeSqlAsync(this,asyncContainer,
				"(SELECT \'power\', sa.value FROM %s.structure_attributes sa WHERE sa.structure_id = %"PRIu64" AND sa.attribute_id = 384)"
				,mDatabase->galaxy(),structure->getId());
		}
		break;

		case Structure_Command_PayMaintenance:
		{
			PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(command.StructureId));

			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_UpdateAttributes,player->getClient());
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			asyncContainer->command			= command;
			mDatabase->executeSqlAsync(this,asyncContainer,"(SELECT \'maintenance\', sa.value FROM %s.structure_attributes sa WHERE sa.structure_id = %"PRIu64" AND sa.attribute_id = 382)"
														 " UNION (SELECT \'condition\', s.condition FROM %s.structures s WHERE s.id = %"PRIu64")",
                                                         mDatabase->galaxy(),structure->getId(),
                                                         mDatabase->galaxy(),structure->getId());
		}
		break;

		// callback for retrieving a variable amount of the selected resource
		case Structure_Command_RetrieveResource:
		{
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(command.StructureId));

			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_ResourceRetrieve,player->getClient());
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			asyncContainer->command 		= command;
	
			mDatabase->executeSqlAsync(harvester,asyncContainer,"SELECT %s.sf_DiscardResource(%"PRIu64",%"PRIu64",%u) ",mDatabase->galaxy(),harvester->getId(),command.ResourceId,command.Amount);

		}
		break;

		// callback for discarding a variable amount of the selected resource
		case Structure_Command_DiscardResource:
		{
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(command.StructureId));

			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_ResourceDiscard,player->getClient());
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			asyncContainer->command 		= command;
			
			mDatabase->executeSqlAsync(harvester,asyncContainer,"SELECT %s.sf_DiscardResource(%"PRIu64",%"PRIu64",%u) ",mDatabase->galaxy(),harvester->getId(),command.ResourceId,command.Amount);

		}
		break;

		case Structure_Command_GetResourceData:
		{
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(command.StructureId));

			if(!harvester)
				return;

			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_GetResourceData,player->getClient());
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			mDatabase->executeSqlAsync(harvester,asyncContainer,"SELECT hr.resourceID, hr.quantity FROM %s.harvester_resources hr WHERE hr.ID = '%"PRIu64"' ",mDatabase->galaxy(),harvester->getId());

		}
		break;

		case Structure_Command_DiscardHopper:
		{
			//send the db update
			StructureManagerAsyncContainer* asyncContainer;

			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(command.StructureId));

			asyncContainer = new StructureManagerAsyncContainer(Structure_HopperDiscard, 0);
			asyncContainer->mStructureId	= command.StructureId;
			asyncContainer->mPlayerId		= command.PlayerId;
			mDatabase->executeSqlAsync(harvester,asyncContainer,"select %s.sf_DiscardHopper(%I64u)",mDatabase->galaxy(),command.StructureId);

		}
		break;	 

		case Structure_Command_OperateHarvester:
		{
			PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(command.StructureId));
			gMessageLib->sendOperateHarvester(structure,player);
		}
		return;

		case Structure_Command_RenameStructure:
		{
			if(owner)
			{
				PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(command.StructureId));
				createRenameStructureBox(player, structure);
			}
			else	{
				gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure","rename_must_be_owner"),player);
			}
			
		}
		return;

		case Structure_Command_TransferStructure:
		{
			if(owner)
				gStructureManager->TransferStructureOwnership(command);
			else	{
				gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure","not_owner"),player);
			}
			
		}
		return;

		case Structure_Command_Destroy: 
		{		
			if(owner)
			{
				FactoryObject* factory = dynamic_cast<FactoryObject*>(gWorldManager->getObjectById(command.StructureId));
			
				if(factory)
				{
					if(factory->getManSchemID())
					{
						gMessageLib->SendSystemMessage(L"You need to remove the manufacturing schematic before destroying the structure", player);
						return;
					}

					TangibleObject* hopper = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(factory->getIngredientHopper()));
					if(hopper && hopper->getHeadCount())	{
						gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure","clear_input_hopper_for_delete"),player);
						return;
					}
					hopper = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(factory->getOutputHopper()));
					if(hopper&&hopper->getHeadCount())	{
						gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure","clear_output_hopper_for_delete"),player);
						return;
					}
				}

				gStructureManager->getDeleteStructureMaintenanceData(command.StructureId, command.PlayerId);
			}
			else	{
				gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure","destroy_must_be_owner"),player);
			}
			
			
		}
		break;

		

		case Structure_Command_AddPermission:
		{
			//make sure we do not add ourselves to the ban list

			addNametoPermissionList(command.StructureId, command.PlayerId, command.PlayerStr, command.List);

		}
		break;

		case Structure_Command_RemovePermission:
		{

			removeNamefromPermissionList(command.StructureId, command.PlayerId, command.PlayerStr, command.List);
	
		}
	
	}
}


//=======================================================================================================================
//processes a structure transfer
//=======================================================================================================================

void StructureManager::TransferStructureOwnership(StructureAsyncCommand command)
{
    //at this point we have already made sure that the command is issued by the owner
    PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(command.StructureId));

    //if we have no structure that way, see whether we have a structure were we just used the adminlist
    if(!structure)
    {
        return;
    }

    //step 1 make sure the recipient has enough free lots!

    //step 1a -> get the recipients ID

    //the recipient MUST be online !!!!! ???????

    StructureManagerAsyncContainer* asyncContainer;
    asyncContainer = new StructureManagerAsyncContainer(Structure_StructureTransfer_Lots_Recipient, 0);
    asyncContainer->mStructureId = command.StructureId;
    asyncContainer->mPlayerId = command.PlayerId;
    asyncContainer->mTargetId = command.RecipientId;

    mDatabase->executeSqlAsync(this,asyncContainer,"SELECT %s.sf_getLotCount(%"PRIu64")",mDatabase->galaxy(),command.PlayerId);

}

uint32 StructureManager::getCurrentPower(PlayerObject* player)
{
    uint32 power = 0;

	auto inventory = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService")->GetEquippedObject(player, "inventory");
	inventory->ViewObjects(player, 0, true, [&] (Object* object) {

        // we are looking for resource containers
        ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(object);
        if(resCont)
        {
            uint16 category = resCont->getResource()->getType()->getCategoryId();

            if(category == 475 || category == 476||category == 477||((category >= 618)&&category <=651 )||category ==903||category == 904 )
            {
                float pe = (float) (resCont->getResource()->getAttribute(ResAttr_PE)/500);//7
                if(pe < 1.0)
                    pe = 1.0;

                uint32 containerPower = 0;
                if (pe > 500)
                    containerPower = (uint32)(resCont->getAmount()* (pe/500));
                else
                    containerPower = (uint32)(resCont->getAmount());

                power += containerPower;
            }
        }
    });

    return power;
}

uint32 StructureManager::deductPower(PlayerObject* player, uint32 amount)
{
    auto inventory = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService")->GetEquippedObject(player, "inventory");
	inventory->ViewObjects(player, 0, true, [&] (Object* object) {

        // we are looking for resource containers
        ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(object);
        if(resCont)
        {
            uint16 category = resCont->getResource()->getType()->getCategoryId();

            if(category == 475 || category == 476||category == 477||((category >= 618)&&category <=651 )||category ==903||category == 904 )
            {
                float pe = resCont->getResource()->getAttribute(ResAttr_PE);//7

                uint32 containerPower = 0;
                if (pe > 500)
                    containerPower = (uint32)(resCont->getAmount()* (pe/500));
                else
                    containerPower = (uint32)(resCont->getAmount());

                uint32 tdAmount = amount;
                if(tdAmount >containerPower)
                    tdAmount = containerPower;
                //default amount is how much to delete, unless pe > 500
                uint32 todelete = tdAmount;
                if (pe > 500)
                    todelete /= (uint32)(pe/500);
                uint32 newAmount = resCont->getAmount()-todelete;
                if(newAmount <0)
                {
                    assert(false && "StructureManager::deductPower new amount cannot be negative");
                }

                resCont->setAmount(newAmount);
                if (resCont->getAmount() == 0)
                {
                    // delete container at 0 amount
					TangibleObject* container = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(resCont->getParentId()));
					gContainerManager->deleteObject(resCont, container);

                    amount -= tdAmount;
                    return;
                }
                else
                {
                    gMessageLib->sendResourceContainerUpdateAmount(resCont,player);
                    mDatabase->executeSqlAsync(NULL,NULL,"UPDATE %s.resource_containers SET amount=%u WHERE id=%"PRIu64"",mDatabase->galaxy(),newAmount,resCont->getId());

                }


                amount -= tdAmount;
            }


        }
    });

    if(amount>0)
    {
        DLOG(info) << "StructureManager::deductPower() couldnt deduct the entire amount !!!!!";
    }
    return (!amount);
}


//======================================================================================================================
//
// Handle deletion of destroyed Structures / building fences and other stuff
//


//======================================================================================================================
//
// Handle deletion of destroyed Structures / building fences and other stuff
//

bool StructureManager::_handleStructureDBCheck(uint64 callTime, void* ref)
{
    //iterate through all harvesters which are marked inactive in the db

    StructureManagerAsyncContainer* asyncContainer;
    asyncContainer = new StructureManagerAsyncContainer(Structure_GetInactiveHarvesters, 0);
    mDatabase->executeSqlAsync(this,asyncContainer,"SELECT h.ID, s.condition FROM %s.harvesters h INNER JOIN %s.structures s ON (h.ID = s.ID) WHERE active = 0 AND s.zone = %u",mDatabase->galaxy(),mDatabase->galaxy(), gWorldManager->getZoneId());

    asyncContainer = new StructureManagerAsyncContainer(Structure_GetDestructionStructures, 0);
    mDatabase->executeSqlAsync(this,asyncContainer,"SELECT h.ID, s.condition FROM %s.harvesters h INNER JOIN %s.structures s ON (h.ID = s.ID) WHERE active = 0 AND( s.condition >= 1000) AND s.zone = %u", mDatabase->galaxy(),mDatabase->galaxy(),gWorldManager->getZoneId());

    return (true);
}


//==========================================================================================0
//asynchronously updates the lot count of a player
void StructureManager::UpdateCharacterLots(uint64 charId)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(charId));

    if(!player)
        return;

    StructureManagerAsyncContainer* asyncContainer;
    asyncContainer = new StructureManagerAsyncContainer(Structure_UpdateCharacterLots, 0);
    asyncContainer->mPlayerId = charId;

    mDatabase->executeSqlAsync(this,asyncContainer,"SELECT %s.sf_getLotCount(%"PRIu64")",mDatabase->galaxy(),charId);

}

//======================================================================================================================
bool StructureManager::HandlePlaceStructure(Object* object, Object* target, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(object);

    if(!player)
    {
        return false;
    }

    //find out where our structure is
    BString dataStr;
    message->getStringUnicode16(dataStr);

    float dir;
    glm::vec3 pVec;
    pVec.x = 0;
    pVec.z = 0;
    uint64 deedId;

    swscanf(dataStr.getUnicode16(),L"%"WidePRIu64 L" %f %f %f",&deedId, &pVec.x, &pVec.z, &dir);

    //check the region whether were allowed to build
    if(checkNoBuildRegion(pVec) /*|| !checkCityRadius(player)*/)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("faction_perk", "no_build_area"), player);
        return false;
    }

    Deed* deed = dynamic_cast<Deed*>(gWorldManager->getObjectById(deedId));
    if(!deed)
    {
        return false;
    }

    switch(deed->getItemType())
    {
    case	ItemType_generator_fusion_personal:
    case	ItemType_generator_solar_personal:
    case	ItemType_generator_wind_personal:

    case	ItemType_harvester_flora_personal:
    case	ItemType_harvester_flora_heavy:
    case	ItemType_harvester_flora_medium:
    case	ItemType_harvester_gas_personal:
    case	ItemType_harvester_gas_heavy:
    case	ItemType_harvester_gas_medium:
    case	ItemType_harvester_liquid_personal:
    case	ItemType_harvester_liquid_heavy:
    case	ItemType_harvester_liquid_medium:

    case	ItemType_harvester_moisture_personal:
    case	ItemType_harvester_moisture_heavy:
    case	ItemType_harvester_moisture_medium:

    case	ItemType_harvester_ore_personal:
    case	ItemType_harvester_ore_heavy:
    case	ItemType_harvester_ore_medium:
    {
        float height;
		
		//please note that the zone id at one point needs to be exchanged for the scene_id!!!!!!! 
		auto terrain = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::terrain::TerrainService>("TerrainService");
		height = terrain->GetHeight(gWorldManager->getZoneId(), pVec.x,pVec.z);
		
		//todo we need to implement custom names
		std::string customName = "";

		gObjectFactory->requestnewHarvesterbyDeed(gStructureManager, deed, player->getClient(),
                     pVec.x, height,  pVec.z, dir, customName, player);
    }
    break;

    case	ItemType_factory_clothing:
    case	ItemType_factory_food:
    case	ItemType_factory_item:
    case	ItemType_factory_structure:
    {
       
		float height;
		
		//please note that the zone id at one point needs to be exchanged for the scene_id!!!!!!! 
		auto terrain = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::terrain::TerrainService>("TerrainService");
		height = terrain->GetHeight(gWorldManager->getZoneId(), pVec.x,pVec.z);
		
		//todo we need to implement custom names
		std::string customName = "";

		gObjectFactory->requestnewFactorybyDeed(gStructureManager, deed, player->getClient(),
                     pVec.x, height,  pVec.z, dir, customName, player);


    }
    break;

    case	ItemType_deed_cityhall_corellia:
    case	ItemType_deed_cityhall_naboo:
    case	ItemType_deed_cityhall_tatooine:
    {
        //FOR CIVIC STRUCTURES
        PlayerObject* player = dynamic_cast<PlayerObject*>(object);
        if(player)
        {
            // TODO: Enum for skills
            if(!player->GetCreature()->checkSkill(623)) //novice Politician
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "place_cityhall"), player);
                break;
            }
        }
        else
        {
            break;
        }
        //NO BREAK!!!!
    }

    case	ItemType_deed_guildhall_corellian:
    case	ItemType_deed_guildhall_naboo:
    case	ItemType_deed_guildhall_tatooine:
    case	ItemType_deed_naboo_large_house:
    case	ItemType_deed_naboo_medium_house:
    case	ItemType_deed_naboo_small_house_2:
    case	ItemType_deed_naboo_small_house:

    case	ItemType_deed_corellia_large_house:
    case	ItemType_deed_corellia_large_house_2:
    case	ItemType_deed_corellia_medium_house:
    case	ItemType_deed_corellia_medium_house_2:

    case	ItemType_deed_corellia_small_house_1:
    case	ItemType_deed_corellia_small_house_2:
    case	ItemType_deed_corellia_small_house_3:
    case	ItemType_deed_corellia_small_house_4:

    case	ItemType_deed_generic_large_house_1:
    case	ItemType_deed_generic_large_house_2:
    case	ItemType_deed_generic_medium_house_1:
    case	ItemType_deed_generic_medium_house_2:
    case	ItemType_deed_generic_small_house_1:
    case	ItemType_deed_generic_small_house_2:
    case	ItemType_deed_generic_small_house_3:
    case	ItemType_deed_generic_small_house_4:

    case	ItemType_deed_tatooine_large_house:
    case	ItemType_deed_tatooine_medium_house:
    case	ItemType_deed_tatooine_small_house:
    case	ItemType_deed_tatooine_small_house_2:
    {
		

        

        //We need to give the thing several points to grab (because we want the max height)
        StructureDeedLink* deedLink;
        deedLink = gStructureManager->getDeedData(deed->getItemType());

        float halfLength = (deedLink->length/2);
        float halfWidth = (deedLink->width/2);
		
		//please note that the zone id at one point needs to be exchanged for the scene_id!!!!!!! 
		auto terrain = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::terrain::TerrainService>("TerrainService");
		pVec.y = terrain->GetHeight(gWorldManager->getZoneId(), pVec.x,pVec.z);

        if(dir == 0 || dir == 2)
        {
            //Orientation 1
            pVec.y = std::max(pVec.y, terrain->GetHeight(gWorldManager->getZoneId(), pVec.x-halfLength, pVec.z-halfWidth));
            pVec.y = std::max(pVec.y, terrain->GetHeight(gWorldManager->getZoneId(), pVec.x+halfLength, pVec.z-halfWidth));
			pVec.y = std::max(pVec.y, terrain->GetHeight(gWorldManager->getZoneId(), pVec.x-halfLength, pVec.z+halfWidth));
            pVec.y = std::max(pVec.y, terrain->GetHeight(gWorldManager->getZoneId(), pVec.x+halfLength, pVec.z+halfWidth));
        }
        else if(dir == 1 || dir == 3)
        {
            //Orientation 2
            pVec.y = std::max(pVec.y, terrain->GetHeight(gWorldManager->getZoneId(), pVec.x-halfWidth, pVec.z-halfLength));
            pVec.y = std::max(pVec.y, terrain->GetHeight(gWorldManager->getZoneId(), pVec.x+halfWidth, pVec.z-halfLength));
            pVec.y = std::max(pVec.y, terrain->GetHeight(gWorldManager->getZoneId(), pVec.x-halfWidth, pVec.z+halfLength));
            pVec.y = std::max(pVec.y, terrain->GetHeight(gWorldManager->getZoneId(), pVec.x+halfWidth, pVec.z+halfLength));
        }
		
		//todo we need to implement custom names
		std::string customName = "";
        gObjectFactory->requestnewHousebyDeed(gStructureManager, deed, player->getClient(),
                     pVec.x, pVec.y, pVec.z, dir, customName, player);

		LOG (info) << "building build at : x " << pVec.x << "y " << pVec.y << " z " << pVec.z;

    }
    break;

    }
    return true;
}
