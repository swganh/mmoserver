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
//#include "UIManager.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "Common/Message.h"
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

	//get the crates containing container - inventory is a tangible, too - we can use the unified interface thks to virtual functions :)
	//add inventories to worldmanager ?
	TangibleObject* tO = dynamic_cast<TangibleObject* >(gWorldManager->getObjectById(crate->getParentId()));
	if(!tO)
	{
		tO = dynamic_cast<TangibleObject* >(inventory);
		if(!tO)
		{
			gLogger->logMsg("ObjectController::_ExtractObject: Crates parent does not exist!");
			assert(false);
			return;
		}
	}
	
	if(!tO->checkCapacity())
	{
		//check if we can fit an additional item in our inventory
		
		return;
	}

	//create the new item
	gObjectFactory->requestNewClonedItem(tO,crate->getLinkedObject()->getId(),tO->getId());

	//decrease crate content
	int32 content = crate->decreaseContent(1);
	if(!content)
	{
		gMessageLib->sendDestroyObject(crate->getId(),playerObject);
		gObjectFactory->deleteObjectFromDB(crate->getId());
		inventory->deleteObject(crate);
		return;
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
