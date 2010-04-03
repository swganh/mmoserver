/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

//common includes
#include "Deed.h"
#include "HarvesterFactory.h"
#include "Heightmap.h"
#include "HarvesterObject.h"
#include "HouseObject.h"
#include "CellObject.h"
#include "PlayerStructure.h"
#include "PlayerObject.h"
#include "Inventory.h"
#include "Item.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectFactory.h"

#include "UIManager.h"
#include "StructureManager.h"
#include "ResourceManager.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"

#include "StructureHeightmapAsyncContainer.h"


//======================================================================================================================
//
// Modifies the Admin List
//
void	ObjectController::_handleModifyPermissionList(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)

{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleModifyPermissionList Player not found",MSG_HIGH);
		return;
	}
	
	//find out where our structure is
	string dataStr;
	message->getStringUnicode16(dataStr);
	
	string playerStr,list,action;
	
	dataStr.convert(BSTRType_ANSI);

	int8 s1[64],s2[32],s3[32];
	sscanf(dataStr.getAnsi(),"%32s %16s %16s",s1, s2, s3);
	playerStr = s1;
	list = s2;
	action = s3;

	if(playerStr.getLength() > 40)
	{
		gMessageLib->sendSystemMessage(player,L"","player_structure","permission_40_char ");
		return;
	}

	gLogger->logMsgF(" %s %s %s",MSG_HIGH, playerStr.getAnsi(), list.getAnsi(), action.getAnsi());

	//TODO is target a structure?? used when using the commandline option
	uint64 id = player->getTargetId();
	Object* object = gWorldManager->getObjectById(id);
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	//if we have no structure that way, see whether we have a structure were we just used the adminlist
	if(!structure)
	{
		id = player->getStructurePermissionId();
		Object* object = gWorldManager->getObjectById(id);
		structure = dynamic_cast<PlayerStructure*>(object);
	}
	
	if(!structure)
	{
		gLogger->logMsgF("ObjectController::_handleModifyPermissionList No structure found :(",MSG_HIGH);
		return;
	}

	//is the structure in Range???
	float fAdminListDistance = gWorldConfig->getConfiguration("Player_Admin_List_Distance",(float)32.0);
	
	if(player->getParentId())
	{
		if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId())))
		{
			if(HouseObject* house = dynamic_cast<HouseObject*>(gWorldManager->getObjectById(cell->getParentId())))
			{
				if(house->getId() != structure->getId())
				{
					gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
					return;
				}
			}
		}
		
	}
	else
	if(!player->mPosition.inRange2D(structure->mPosition,fAdminListDistance))
	{
		gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		return;
	}

	player->setStructurePermissionId(0);
	
	StructureAsyncCommand command;
	command.PlayerId = player->getId();
	command.StructureId = structure->getId();
	command.List = list;
	command.PlayerStr = playerStr;

	if(action == "add")
	{
		command.Command = Structure_Command_AddPermission;	
		gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
	}

	if(action == "remove")
	{
		command.Command = Structure_Command_RemovePermission;	
		gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
	}
		
}


//======================================================================================================================
//
// Places a structure in the game world
//
void ObjectController::_handleStructurePlacement(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	// TODO - are we in structure placement mode ???
	if(!player)
	{
		//gMessageLib->sendSystemMessage(entertainer,L"","performance","flourish_not_performing");
		return;
	}	

	//find out where our structure is
	string dataStr;
	message->getStringUnicode16(dataStr);
	
	float x,z,dir;
	uint64 deedId;

	swscanf(dataStr.getUnicode16(),L"%I64u %f %f %f",&deedId, &x, &z, &dir);

	gLogger->logMsgF(" ID %I64u x %f y %f dir %f",MSG_HIGH, deedId, x, z, dir);
	
	
	//now get our deed
	//Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	Deed* deed = dynamic_cast<Deed*>(gWorldManager->getObjectById(deedId));
	if(!deed)
	{
		gLogger->logMsgF(" ObjectController::_handleStructurePlacement deed not found :( ",MSG_HIGH);		
		return;
	}

	//todo : check if the type of building is allowed on the planet

	//check the region whether were allowed to build
	if(!gStructureManager->checkCityRadius(player))
	{
		gMessageLib->sendSystemMessage(player,L"","camp","error_nobuild");
		return;
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
			StructureHeightmapAsyncContainer* container = new StructureHeightmapAsyncContainer(this, HeightmapCallback_StructureHarvester);
			
			container->oCallback = gObjectFactory;
			container->ofCallback = gStructureManager;
			container->deed = deed;
			container->dir = dir;
			container->x = x;
			container->z = z;
			container->customName = "";
			container->player = player;

			container->addToBatch(x,z);

			Heightmap::Instance()->addNewHeightMapJob(container);
		}
		break;

		case	ItemType_factory_clothing:
		case	ItemType_factory_food:
		case	ItemType_factory_item:
		case	ItemType_factory_structure:
		{
			StructureHeightmapAsyncContainer* container = new StructureHeightmapAsyncContainer(this, HeightmapCallback_StructureFactory);
			
			container->oCallback = gObjectFactory;
			container->ofCallback = gStructureManager;
			container->deed = deed;
			container->dir = dir;
			container->x = x;
			container->z = z;
			container->customName = "";
			container->player = player;

			container->addToBatch(x,z);

			Heightmap::Instance()->addNewHeightMapJob(container);
		}
		break;

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
			StructureHeightmapAsyncContainer* container = new StructureHeightmapAsyncContainer(this, HeightmapCallback_StructureHouse);
			
			container->oCallback = gObjectFactory;
			container->ofCallback = gStructureManager;
			container->deed = deed;
			container->x = x;
			container->z = z;
			container->dir = dir;
			container->customName = "";
			container->player = player;

			//We need to give the thing several points to grab (because we want the max height)
			StructureDeedLink* deedLink;
			deedLink = gStructureManager->getDeedData(deed->getItemType());

			uint32 halfLength = (deedLink->length/2);
			uint32 halfWidth = (deedLink->width/2);

			container->addToBatch(x, z);

			if(dir == 0 || dir == 2)
			{
				//Orientation 1
				container->addToBatch(x-halfLength, z-halfWidth);
				gLogger->logMsgF("%f %f\n", MSG_NORMAL,x-halfLength, z-halfWidth);
				container->addToBatch(x+halfLength, z-halfWidth);
				gLogger->logMsgF("%f %f\n", MSG_NORMAL,x+halfLength, z-halfWidth);
				container->addToBatch(x-halfLength, z+halfWidth);
				gLogger->logMsgF("%f %f\n", MSG_NORMAL,x-halfLength, z+halfWidth);
				container->addToBatch(x+halfLength, z+halfWidth);
				gLogger->logMsgF("%f %f\n", MSG_NORMAL,x+halfLength, z+halfWidth);
			}

			if(dir == 1 || dir == 3)
			{
				//Orientation 2
				container->addToBatch(x-halfWidth, z-halfLength);
				container->addToBatch(x+halfWidth, z-halfLength);
				container->addToBatch(x-halfWidth, z+halfLength);
				container->addToBatch(x+halfWidth, z+halfLength);
			}

			Heightmap::Instance()->addNewHeightMapJob(container);
		}
		break;

	}
		
	
}

void ObjectController::HeightmapStructureHandler(HeightmapAsyncContainer* ref)
{
	StructureHeightmapAsyncContainer* container = static_cast<StructureHeightmapAsyncContainer*>(ref);

	switch(container->type)
	{
	case HeightmapCallback_StructureHouse:
	{
		HeightResultMap* mapping = container->getResults();
		HeightResultMap::iterator it = mapping->begin();

		float highest = 0;
		bool worked = false;
		while(it != mapping->end() && it->second != NULL)
		{
			worked = true;

			if(it->second->height > highest)
				highest = it->second->height;

			it++;
		}

		if(worked)
		{
			container->oCallback->requestnewHousebyDeed(container->ofCallback,container->deed,container->player->getClient(),
														container->x,highest,container->z,container->dir,container->customName,
														container->player);
		}
		break;
	}
	case HeightmapCallback_StructureFactory:
	{
		HeightResultMap* mapping = container->getResults();
		HeightResultMap::iterator it = mapping->begin();
		if(it != mapping->end() && it->second != NULL)
		{
			container->oCallback->requestnewFactorybyDeed(container->ofCallback,container->deed,container->player->getClient(),
														it->first.first,it->second->height,it->first.second,container->dir,
														container->customName, container->player);
		}
		break;
	}
	case HeightmapCallback_StructureHarvester:
	{
		HeightResultMap* mapping = container->getResults();
		HeightResultMap::iterator it = mapping->begin();
		if(it != mapping->end() && it->second != NULL)
		{
			container->oCallback->requestnewHarvesterbyDeed(container->ofCallback,container->deed,container->player->getClient(),
				it->first.first,it->second->height,it->first.second,container->dir,container->customName,
														container->player);
		}
		break;
	}
	}
}

//======================================================================================================================
//
// Modifies the Admin List
//
void	ObjectController::_handleTransferStructure(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// requirement we have the structure targeted AND give the name of the recipient on the commandline
	// OR we have the recipient targeted and stand NEXT to the structure were about to transfer

	//do we have a valid donor ?
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleTransferStructure Player not found",MSG_HIGH);
		return;
	}


	// is the player online and near 30m ?
	// we get the  players id as targetid if yes, otherwise we get the name as string
	// however, we do not want players that  are not online
	
	//now get the target player
	PlayerObject*	recipient	= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId));

	if(!recipient)
	{
		gMessageLib->sendSystemMessage(player,L"","player_structure","no_transfer_target");
		return;
	}

	//do we have a valid structure ??? check our target first
	uint64 id = player->getTargetId();
	Object* object = gWorldManager->getObjectById(id);
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	if(!structure)
	{
		// we need to get the nearest structure that we own
		// for now dustoff
		gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		gLogger->logMsgF("ObjectController::_handleTransferStructure No structure found :(",MSG_HIGH);
		return;
	}
	
	//is the structure in Range???
	float fTransferDistance = gWorldConfig->getConfiguration("Player_Transfer_Structure_Distance",(float)8.0);
	if(!player->mPosition.inRange2D(structure->mPosition,fTransferDistance))
	{
		gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		return;
	}

	StructureAsyncCommand command;
	command.PlayerId = player->getId();
	command.StructureId = structure->getId();
	command.RecipientId = recipient->getId();
	command.PlayerStr = recipient->getFirstName().getAnsi();
	command.Command = Structure_Command_TransferStructure;	

	gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
		
}





//======================================================================================================================
//
// Modifies the Admin List
//
void	ObjectController::_handleNameStructure(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)

{
	// requirement we have the structure targeted AND give the name of the recipient on the commandline
	// OR we have the recipient targeted and stand NEXT to the structure were about to transfer

	//do we have a valid donor ?
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleTransferStructure Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid structure ??? check our target first
	uint64 id = player->getTargetId();
	Object* object = gWorldManager->getObjectById(id);
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	if(!structure)
	{
		gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		return;
	}
	
	//is the structure in Range???
	float fTransferDistance = gWorldConfig->getConfiguration("Player_Structure_Operate_Distance",(float)10.0);
	if(!player->mPosition.inRange2D(structure->mPosition,fTransferDistance))
	{
		gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		return;
	}

	//find out where our structure is
	string dataStr;
	message->getStringUnicode16(dataStr);
	
	string nameStr;
	
	dataStr.convert(BSTRType_ANSI);

	sscanf(dataStr.getAnsi(),"%s",nameStr.getAnsi());

	if(nameStr.getLength() > 68)
	{
		gMessageLib->sendSystemMessage(player,L"","player_structure","not_valid_name");
		return;
	}

	StructureAsyncCommand command;
	command.Command = Structure_Command_RenameStructure;
	command.PlayerId = player->getId();
	command.StructureId = structure->getId();

	gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
		
}

//======================================================================================================================
//
// provides the harvester with the current resources
//
void	ObjectController::_handleHarvesterGetResourceData(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterGetResourceData Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid structure ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	if(!structure)
	{
		//gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		gLogger->logMsgF(" ObjectController::_handleHarvesterGetResourceData Structure not found",MSG_HIGH);
		return;
	}
	
	//is the structure in Range???
	float fTransferDistance = gWorldConfig->getConfiguration("Player_Structure_Operate_Distance",(float)10.0);
	if(!player->mPosition.inRange2D(structure->mPosition,fTransferDistance))
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterGetResourceData Structure not in Range",MSG_HIGH);
		return;
	}

	HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

	StructureAsyncCommand command;

	command.Command = Structure_Command_GetResourceData;
	command.PlayerId = player->getId();
	command.StructureId = structure->getId();

	gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

	return;
	gMessageLib->sendHarvesterResourceData(structure,player);

	gLogger->logMsgF(" ObjectController::_handleHarvesterGetResourceData :: hino 7 baseline",MSG_HIGH);
	gMessageLib->sendBaselinesHINO_7(harvester,player);

	//add the structure to the timer so the resource amounts are updated while we look at the hopper
	//harvester->getTTS()->todo		= ttE_UpdateHopper;
	//harvester->getTTS()->playerId	= player->getId();
	//structure->getTTS()->projectedTime = 5000 + Anh_Utils::Clock::getSingleton()->getLocalTime();
	//gStructureManager->addStructureforHopperUpdate(harvester->getId());

	// this needs to be handled zoneserverside - otherwise the addition of a res will trigger a racecondition 
	// between the sql write query and the sql read please note that the harvesting itself happens through stored procedures
	// and we cant keep the updatecounters synchronized
	

}



//======================================================================================================================
//
// Selects the resource for extraction
//
void	ObjectController::_handleHarvesterSelectResource(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	gLogger->logMsgF(" ObjectController::_handleHarvesterSelectResource",MSG_HIGH);

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterSelectResource Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid structure ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	if(!structure)
	{
		//gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		gLogger->logMsgF(" ObjectController::_handleHarvesterSelectResource Structure not found",MSG_HIGH);
		return;
	}
	
	//is the structure in Range???
	float fTransferDistance = gWorldConfig->getConfiguration("Player_Structure_Operate_Distance",(float)10.0);
	if(!player->mPosition.inRange2D(structure->mPosition,fTransferDistance))
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterGetResourceData Structure not in Range",MSG_HIGH);
		return;
	}

	HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

	//get the relevant Resource
	string dataStr;
	message->getStringUnicode16(dataStr);

	uint64 resourceId;
	swscanf(dataStr.getUnicode16(),L"%I64u",&resourceId);

	Resource* tmpResource = gResourceManager->getResourceById(resourceId);
	
	if((!tmpResource)||(!tmpResource->getCurrent()))
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterGetResourceData No valid resource!",MSG_HIGH);
		return;
	}

	harvester->setCurrentResource(resourceId);

	// update the current resource in the db 
	mDatabase->ExecuteSqlAsync(0,0,"UPDATE harvesters SET ResourceID=%"PRIu64" WHERE id=%"PRIu64" ",resourceId,harvester->getId());


	CurrentResource* cR = reinterpret_cast<CurrentResource*>(tmpResource);
		//resource = reinterpret_cast<CurrentResource*>(gResourceManager->getResourceByNameCRC(resourceName.getCrc()));

	float posX, posZ;
	float ratio = 0.0;

	posX	= harvester->mPosition.mX;
	posZ	= harvester->mPosition.mZ;
	
	
	if(cR)
	{
		ratio	= cR->getDistribution((int)posX + 8192,(int)posZ + 8192);
		if(ratio > 1.0)
		{
			ratio = 1.0;
		}
	}

	float ber = harvester->getSpecExtraction();

	harvester->setCurrentExtractionRate(ber*ratio);

	// now enter the new resource in the hoppers resource list if its isnt already in there
	// TODO keep the list up to date by removing unnecessary resources
	// to this end read the list anew and delete every resource with zero amount
	// have a stored function do this
	
	if(!harvester->checkResourceList(resourceId))
	{
		//do *not* add to list - otherwise we get a racecondition with the asynch update from db !!!
		//harvester->getResourceList()->push_back(std::make_pair(resourceId,float(0.0)));
		//add to db
		mDatabase->ExecuteSqlAsync(0,0,"INSERT INTO harvester_resources VALUES(%"PRIu64",%"PRIu64",0,0)",harvester->getId(),resourceId);
	}

	// update the current extractionrate in the db for the stored procedure handling the harvesting
	mDatabase->ExecuteSqlAsync(0,0,"UPDATE harvesters SET rate=%f WHERE id=%"PRIu64" ",(ber*ratio),harvester->getId());

	//now send the updates
	gMessageLib->sendCurrentResourceUpdate(harvester,player);
	gMessageLib->sendCurrentExtractionRate(harvester,player);

}


//======================================================================================================================
//
// Turns a harvester on
//

void	ObjectController::_handleHarvesterActivate(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterSelectResource Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid structure ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	if(!structure)
	{
		//gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		gLogger->logMsgF(" ObjectController::_handleHarvesterSelectResource Structure not found",MSG_HIGH);
		return;
	}
	
	//is the structure in Range???
	float fTransferDistance = gWorldConfig->getConfiguration("Player_Structure_Operate_Distance",(float)10.0);
	if(!player->mPosition.inRange2D(structure->mPosition,fTransferDistance))
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterActivate Structure not in Range",MSG_HIGH);
		return;
	}

	HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

	harvester->setActive(true);

	//send the respective delta
	gMessageLib->sendHarvesterActive(harvester);
	
	//send the db update
	mDatabase->ExecuteSqlAsync(0,0,"UPDATE harvesters SET active= 1 WHERE id=%"PRIu64" ",harvester->getId());

}

//======================================================================================================================
//
// Turns a harvester off
//

void	ObjectController::_handleHarvesterDeActivate(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterSelectResource Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid structure ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	if(!structure)
	{
		//gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		gLogger->logMsgF(" ObjectController::_handleHarvesterSelectResource Structure not found",MSG_HIGH);
		return;
	}
	
	//is the structure in Range???
	float fTransferDistance = gWorldConfig->getConfiguration("Player_Structure_Operate_Distance",(float)10.0);
	if(!player->mPosition.inRange2D(structure->mPosition,fTransferDistance))
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterGetResourceData Structure not in Range",MSG_HIGH);
		return;
	}

	HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

	harvester->setActive(false);

	//send the respective delta
	gMessageLib->sendHarvesterActive(harvester);
	
	//send the db update
	mDatabase->ExecuteSqlAsync(0,0,"UPDATE harvesters SET active = 0 WHERE id=%"PRIu64" ",harvester->getId());

}


//======================================================================================================================
//
// Discards the contents of a harvesters Hopper
//
void	ObjectController::_handleDiscardHopper(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterSelectResource Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid structure ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	if(!structure)
	{
		//gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		gLogger->logMsgF(" ObjectController::_handleHarvesterSelectResource Structure not found",MSG_HIGH);
		return;
	}
	
	//is the structure in Range???
	float fTransferDistance = gWorldConfig->getConfiguration("Player_Structure_Operate_Distance",(float)10.0);
	if(!player->mPosition.inRange2D(structure->mPosition,fTransferDistance))
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterGetResourceData Structure not in Range",MSG_HIGH);
		return;
	}

	HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

	StructureAsyncCommand command;

	command.Command = Structure_Command_DiscardHopper;
	command.PlayerId = player->getId();
	command.StructureId = structure->getId();

	gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

	//mDatabase->ExecuteSqlAsync(0,0,"SELECT sf_DiscardHopper(%"PRIu64") ",harvester->getId());
//gMessageLib->SendHarvesterHopperUpdate(harvester,player);

}


//=============================================================================================================================
// discards x amount of specified resource

void	ObjectController::handleResourceEmptyHopper(Message* message)
{
	uint64 playerId;
	uint64 harvesterId;

	message->getUint64(playerId);	
	message->getUint64(harvesterId);

	PlayerObject*   player  = dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::ResourceEmptyHopper Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid structure ??? 
	Object* object = gWorldManager->getObjectById(harvesterId);
	PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

	if(!structure)
	{
		//gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
		gLogger->logMsgF(" ObjectController::_handleHarvesterSelectResource Structure not found",MSG_HIGH);
		return;
	}
	
	//is the structure in Range???
	float fTransferDistance = gWorldConfig->getConfiguration("Player_Structure_Operate_Distance",(float)10.0);
	if(!player->mPosition.inRange2D(structure->mPosition,fTransferDistance))
	{
		gLogger->logMsgF(" ObjectController::_handleHarvesterGetResourceData Structure not in Range",MSG_HIGH);
		return;
	}

	uint64 resourceId;
	uint32 amount;
	uint8 b1, b2;

	message->getUint64(resourceId);
	message->getUint32(amount);
	message->getUint8(b1);
	message->getUint8(b2);

	HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

	StructureAsyncCommand command;

	if(b1 == 0)
	{
		command.Command		=	Structure_Command_RetrieveResource;
		command.PlayerId	=	player->getId();		   
		command.StructureId =	structure->getId();
		command.ResourceId	=	resourceId; 
		command.Amount		=	amount;
		command.b1 = b1;
		command.b2 = b2;
	}
	if(b1 == 1)
	{
		command.Command		=	Structure_Command_DiscardResource;
		command.PlayerId	=	player->getId();		   
		command.StructureId =	structure->getId();
		command.ResourceId	=	resourceId; 
		command.Amount		=	amount;
		command.b1 = b1;
		command.b2 = b2;

	}

	gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"HOPPER",command);

}



//======================================================================================================================
//
// moves an item
//
void	ObjectController::_handleItemMoveForward(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid item ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);

	if(!object)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation item not found",MSG_HIGH);
		return;
	}

	if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId())))
	{
		if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId())))
		{
			if(!building->hasAdminRights(player->getId()))
			{
				gLogger->logMsgF(" ObjectController::_handleItemRotation no admin rights",MSG_HIGH);
				return;
			}
		}
		else
		{
			gLogger->logMsgF(" ObjectController::_handleItemRotation no structure",MSG_HIGH);
			return;
		}
	}
	else
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation no cell",MSG_HIGH);
		return;
	}


	//we somehow need to calculate the vector of the movement *away* from us
	player->mDirection.normalize();
	player->mPosition.normalize();
	object->mPosition.normalize();
	object->mPosition.mX -= (float)((1-player->mDirection.mX) * 0.10);
	object->mPosition.mZ -= (float)(player->mDirection.mY * 0.10);
	object->mPosition.mY -= (float)(player->mDirection.mZ * 0.10);
    
	//Anh_Math::Vector3 v3 = object->mPosition - player->mPosition;
	//v3.normalize();
	//object->mPosition += v3 * 0.1;
	//object->mPosition.normalize();
	
	gMessageLib->sendDataTransformWithParent(object);
	object->updateWorldPosition();

}


//======================================================================================================================
//
// moves an item
//
void	ObjectController::_handleItemMoveUp(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid item ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);

	if(!object)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation item not found",MSG_HIGH);
		return;
	}

	if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId())))
	{
		if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId())))
		{
			if(!building->hasAdminRights(player->getId()))
			{
				gLogger->logMsgF(" ObjectController::_handleItemRotation no admin rights",MSG_HIGH);
				return;
			}
		}
		else
		{
			gLogger->logMsgF(" ObjectController::_handleItemRotation no structure",MSG_HIGH);
			return;
		}
	}
	else
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation no cell",MSG_HIGH);
		return;
	}


	object->mPosition.mY += static_cast<float>(0.10);
	gMessageLib->sendDataTransformWithParent(object);
	object->updateWorldPosition();
}


//======================================================================================================================
//
// moves an item
//
void	ObjectController::_handleItemMoveDown(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid item ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);

	if(!object)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation item not found",MSG_HIGH);
		return;
	}

	if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId())))
	{
		if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId())))
		{
			if(!building->hasAdminRights(player->getId()))
			{
				gLogger->logMsgF(" ObjectController::_handleItemRotation no admin rights",MSG_HIGH);
				return;
			}
		}
		else
		{
			gLogger->logMsgF(" ObjectController::_handleItemRotation no structure",MSG_HIGH);
			return;
		}
	}
	else
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation no cell",MSG_HIGH);
		return;
	}


	object->mPosition.mY -= static_cast<float>(0.10);
	gMessageLib->sendDataTransformWithParent(object);
	object->updateWorldPosition();
}




//======================================================================================================================
//
// moves an item
//
void	ObjectController::_handleItemMoveBack(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid item ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);

	if(!object)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation item not found",MSG_HIGH);
		return;
	}

	if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId())))
	{
		if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId())))
		{
			if(!building->hasAdminRights(player->getId()))
			{
				gLogger->logMsgF(" ObjectController::_handleItemRotation no admin rights",MSG_HIGH);
				return;
			}
		}
		else
		{
			gLogger->logMsgF(" ObjectController::_handleItemRotation no structure",MSG_HIGH);
			return;
		}
	}
	else
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation no cell",MSG_HIGH);
		return;
	}

	//we somehow need to calculate the vector of the movement *away* from us

	player->mDirection.normalize();
	object->mPosition.mX += (float)((1-player->mDirection.mX) * 0.10);
	object->mPosition.mZ += (float)(player->mDirection.mY * 0.10);
	object->mPosition.mY += (float)(player->mDirection.mZ * 0.10);

	gMessageLib->sendDataTransformWithParent(object);
	object->updateWorldPosition();

}





//======================================================================================================================
//
// rotates an item	 90d to right
//
void	ObjectController::_handleItemRotationRight90(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid item ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);

	if(!object)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation item not found",MSG_HIGH);
		return;
	}

	if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId())))
	{
		if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId())))
		{
			if(!building->hasAdminRights(player->getId()))
			{
				gLogger->logMsgF(" ObjectController::_handleItemRotation no admin rights",MSG_HIGH);
				return;
			}
		}
		else
		{
			gLogger->logMsgF(" ObjectController::_handleItemRotation no structure",MSG_HIGH);
			return;
		}
	}
	else
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation no cell",MSG_HIGH);
		return;
	}
	
	
	object->mDirection.rotatex(90);
	gMessageLib->sendDataTransformWithParent(object);
	object->updateWorldPosition();
	

}

//======================================================================================================================
//
// rotates an item 90d to left
//
void	ObjectController::_handleItemRotationLeft90(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid item ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);

	if(!object)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation item not found",MSG_HIGH);
		return;
	}

	if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId())))
	{
		if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId())))
		{
			if(!building->hasAdminRights(player->getId()))
			{
				gLogger->logMsgF(" ObjectController::_handleItemRotation no admin rights",MSG_HIGH);
				return;
			}
		}
		else
		{
			gLogger->logMsgF(" ObjectController::_handleItemRotation no structure",MSG_HIGH);
			return;
		}
	}
	else
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation no cell",MSG_HIGH);
		return;
	}
	
	
	object->mDirection.rotatex(-90);
	gMessageLib->sendDataTransformWithParent(object);
	object->updateWorldPosition();
	

}

//======================================================================================================================
//
// rotates an item
//
void	ObjectController::_handleItemRotation(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation Player not found",MSG_HIGH);
		return;
	}

	//do we have a valid item ??? 
	uint64 id = targetId;
	Object* object = gWorldManager->getObjectById(id);

	if(!object)
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation item not found",MSG_HIGH);
		return;
	}

	if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId())))
	{
		if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId())))
		{
			if(!building->hasAdminRights(player->getId()))
			{
				gLogger->logMsgF(" ObjectController::_handleItemRotation no admin rights",MSG_HIGH);
				return;
			}
		}
		else
		{
			gLogger->logMsgF(" ObjectController::_handleItemRotation no structure",MSG_HIGH);
			return;
		}
	}
	else
	{
		gLogger->logMsgF(" ObjectController::_handleItemRotation no cell",MSG_HIGH);
		return;
	}


	string dataStr;
	message->getStringUnicode16(dataStr);

	uint32 degrees;
	int8 direction[32];
	dataStr.convert(BSTRType_ANSI);
	sscanf(dataStr.getAnsi(),"%s %u",&direction, &degrees);

	gLogger->logMsgF(" ObjectController::_handleItemRotation direction %s",MSG_HIGH,direction);
	
	if(strcmp(direction,"left") == 0)
	{
		object->mDirection.rotatex(-(float) degrees);
		gMessageLib->sendDataTransformWithParent(object);
	}

	if(strcmp(direction,"right") == 0)
	{
		object->mDirection.rotatex((float) degrees);
		gMessageLib->sendDataTransformWithParent(object);
	}

	object->updateWorldPosition();
}
