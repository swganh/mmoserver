/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

//common includes
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "WorldConfig.h"
#include "UIManager.h"
#include "HarvesterFactory.h"
#include "Item.h"
#include "Deed.h"

//specific includes


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
	
	float x,z,dir;
	uint64 deedId;

	swscanf(dataStr.getUnicode16(),L"%I64u %d %d %d",&deedId, &x, &z, &dir);

	gLogger->logMsgF(" ID %I64u x %d y %d dir %d",MSG_HIGH, deedId, x, z, dir);
	
	//now get our deed
	Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	Deed* deed = dynamic_cast<Deed*>(inventory->getObjectById(deedId));

	//now check on how to proceed
	switch(deed->getItemType())
	{
		case	ItemType_personal_fusion_generator:
		case	ItemType_personal_solar_generator:
		case	ItemType_personal_wind_generator:
		{
			gObjectFactory->requestnewHarvesterbyDeed(NULL,deed,player->getClient(),x,z,dir,"");
		}
		break;
	}
	//

	//now place it
	// call the factory
	//1) send the messages to all players around

	/*
	gMessageLib->sendCreateHarvester(,player);

	//create it in the world	
	gWorldManager->addObject(camp);
	
	PlayerObjectSet*			inRangePlayers	= player->getKnownPlayers();
	PlayerObjectSet::iterator	it				= inRangePlayers->begin();
	while(it != inRangePlayers->end())
	{
		PlayerObject* targetObject = (*it);
		gMessageLib->sendCreateTangible(camp,targetObject);
		targetObject->addKnownObjectSafe(camp);
		camp->addKnownObjectSafe(targetObject);
		++it;
	}

	gMessageLib->sendCreateTangible(camp,player);
	player->addKnownObjectSafe(camp);
	camp->addKnownObjectSafe(player);
	gMessageLib->sendDataTransform(camp);
	  */
	//2) put it in the db

	//3) 

	
}