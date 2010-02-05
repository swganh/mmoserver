/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
//#include "BankTerminal.h"
//#include "CraftingTool.h"
//#include "CurrentResource.h"
#include "Inventory.h"
#include "Item.h"
//#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "FactoryCrate.h"
//#include "SurveyTool.h"
#include "UIManager.h"
//#include "Wearable.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
//#include "DatabaseManager/Database.h"
//#include "DatabaseManager/DatabaseResult.h"
//#include "DatabaseManager/DataBinding.h"
#include "Common/Message.h"
//#include "Common/MessageFactory.h"

#include <boost/lexical_cast.hpp>

//======================================================================================================================
//splits a factory crate

void	ObjectController::_handleFactoryCrateSplit(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

}
//======================================================================================================================
//extracts an item out of a factory crate

void	ObjectController::_ExtractObject(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		playerObject		= dynamic_cast<PlayerObject*>(mObject);
	FactoryCrate*		crate				= dynamic_cast<FactoryCrate*>(gWorldManager->getObjectById(targetId));

	Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	//gLogger->logMsgF("ObjectController::_ExtractObject: Container : %I64u",MSG_NORMAL,targetId);

	if(!crate)
	{
		gLogger->logMsg("ObjectController::_ExtractObject: Crate does not exist!");
		return;
	}

	//get the crates containing container

	if(crate->getParentId() == inventory->getId())
	{
		if(!inventory->checkSlots(1))
		{
			//check if we can fit an additional item in our inventory
			gMessageLib->sendSystemMessage(playerObject,L"","error_message","inv_full");
			return;
		}
		//create the new item
		gObjectFactory->requestNewClonedItem(inventory,crate->getLinkedObject()->getId(),inventory->getId());

		//decrease crate content
		int32 content = crate->decreaseContent(1);
		if(!content)
		{
			gMessageLib->sendDestroyObject(crate->getId(),playerObject);
			gObjectFactory->deleteObjectFromDB(crate->getId());
			inventory->deleteObject(crate);
		}
			if(content < 0)
			{
				gLogger->logMsg("ObjectController::_ExtractObject: the crate now has negative content!");
				assert(false);
				return;
			}
		return;
	}

	Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(crate->getParentId()));
	if(!item)
	{
		gLogger->logMsg("ObjectController::_ExtractObject: Crates parent does not exist!");
		assert(false);
		return;
	}
	
	if(!item->checkCapacity())
	{
		//check if we can fit an additional item in our inventory
		gMessageLib->sendSystemMessage(playerObject,L"","container_error_message","container3");
		return;
	}

	//create the new item
	gObjectFactory->requestNewClonedItem(item,crate->getLinkedObject()->getId(),item->getId());

	//decrease crate content
	int32 content = crate->decreaseContent(1);
	if(!content)
	{
		gMessageLib->sendDestroyObject(crate->getId(),playerObject);
		gObjectFactory->deleteObjectFromDB(crate->getId());
		inventory->deleteObject(crate);
	}
	
	if(content < 0)
	{
		gLogger->logMsg("ObjectController::_ExtractObject: the crate now has negative content!");
		assert(false);
		return;
	}

	gMessageLib->sendUpdateCrateContent(crate,playerObject);

	return;

	
}
