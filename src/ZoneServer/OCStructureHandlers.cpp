/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

//common includes
#include "Deed.h"
#include "nonPersistantObjectFactory.h"
#include "HarvesterFactory.h"
#include "Heightmap.h"
#include "PlayerStructure.h"
#include "Inventory.h"
#include "Item.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "StructureManager.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"


//specific includes

//======================================================================================================================
//
// Modifies the Admin List
//
void	ObjectController::_handleModifyPermissionList(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)

{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(!player)
	{
		//gMessageLib->sendSystemMessage(entertainer,L"","performance","flourish_not_performing");
		return;
	}
	
	//find out where our structure is
	string dataStr;
	gLogger->hexDump(message->getData(), message->getSize());
	message->getStringUnicode16(dataStr);
	
	string playerStr,list,action;
	
	dataStr.convert(BSTRType_ANSI);

	sscanf(dataStr.getAnsi(),"%s %s %s",playerStr.getAnsi(), list.getAnsi(), action.getAnsi());

	gLogger->logMsgF(" %s %s %s",MSG_HIGH, playerStr.getAnsi(), list.getAnsi(), action.getAnsi());

	if(action == "add")
		gStructureManager->addNametoPermissionList(targetId, player->getId(), playerStr, list);

	if(action == "remove")
		gStructureManager->addNametoPermissionList(targetId, player->getId(), playerStr, list);
}
//======================================================================================================================
//
// Places a structure in the game world
//

void	ObjectController::_handleStructurePlacement(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
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
	
	float x,y,z,dir;
	uint64 deedId;

	swscanf(dataStr.getUnicode16(),L"%I64u %f %f %f",&deedId, &x, &z, &dir);

	gLogger->logMsgF(" ID %I64u x %f y %f dir %f",MSG_HIGH, deedId, x, z, dir);
	
	//slow query - use for building placement only
	y = Heightmap::Instance()->getHeight(x,z);
	
	//now get our deed
	Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	Deed* deed = dynamic_cast<Deed*>(inventory->getObjectById(deedId));

	//TODO
	//still have to check the region whether were allowed to build
	//still have to remove the deed out of the inventory (assign to harvester)

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
			PlayerStructure* structure =  gNonPersistantObjectFactory->requestBuildingFenceObject(x,y,z, player);
			gObjectFactory->requestnewHarvesterbyDeed(gWorldManager,deed,player->getClient(),x,y,z,dir,"",player, structure->getId());
		}
		break;
	}
		
	
}