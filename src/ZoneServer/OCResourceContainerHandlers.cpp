/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "BankTerminal.h"
#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Inventory.h"
#include "Item.h"
#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "ResourceContainer.h"
#include "SurveyTool.h"
#include "TravelMapHandler.h"
#include "UIManager.h"
#include "Wearable.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"

#include <boost/lexical_cast.hpp>

//======================================================================================================================
//
// transfers resources across resource containers
//

void ObjectController::_handleResourceContainerTransfer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		playerObject		= dynamic_cast<PlayerObject*>(mObject);
	ResourceContainer*	selectedContainer	= dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(targetId));

	if(selectedContainer)
	{
		string dataStr;

		message->getStringUnicode16(dataStr);
		dataStr.convert(BSTRType_ANSI);

		BStringVector dataElements;

		uint16 elementCount = dataStr.split(dataElements,' ');

		if(!elementCount)
		{
			gLogger->logMsg("ObjectController::_handleResourceContainerTransfer: Error in requestStr");
			return;
		}

		ResourceContainer* targetContainer = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(boost::lexical_cast<uint64>(dataElements[0].getAnsi())));

		if(targetContainer && targetContainer->getResourceId() == selectedContainer->getResourceId())
		{
			uint32	targetAmount	= targetContainer->getAmount();
			uint32	selectedAmount	= selectedContainer->getAmount();
			uint32	maxAmount		= targetContainer->getMaxAmount();
			uint32	newAmount;

			gLogger->logMsg("transfer  resi");
			// all fits
			if((newAmount = targetAmount + selectedAmount) <= maxAmount)
			{
				// update target container
				targetContainer->setAmount(newAmount);

				gMessageLib->sendResourceContainerUpdateAmount(targetContainer,playerObject);

				mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%lld",newAmount,targetContainer->getId());

				// delete old container
				gMessageLib->sendDestroyObject(selectedContainer->getId(),playerObject);

				gObjectFactory->deleteObjectFromDB(selectedContainer);
				dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->deleteObject(selectedContainer);


			}
			// target container full, update both contents
			else if(newAmount > maxAmount)
			{
				uint32 selectedNewAmount = newAmount - maxAmount;
				targetContainer->setAmount(maxAmount);
				selectedContainer->setAmount(selectedNewAmount);

				gMessageLib->sendResourceContainerUpdateAmount(targetContainer,playerObject);
				gMessageLib->sendResourceContainerUpdateAmount(selectedContainer,playerObject);

				mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%lld",maxAmount,targetContainer->getId());
				mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%lld",selectedNewAmount,selectedContainer->getId());
			}
		}
	}
}

//======================================================================================================================
//
// splits up a resource container
//

void ObjectController::_handleResourceContainerSplit(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		playerObject		= dynamic_cast<PlayerObject*>(mObject);
	ResourceContainer*	selectedContainer	= dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(targetId));

	Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	gLogger->logMsgF("ObjectController::_handleResourceContainerSplit: Container : %I64u",MSG_NORMAL,targetId);

	if(!selectedContainer)
	{
		gLogger->logMsg("ObjectController::_handleResourceContainerSplit: Container does not exist!");
		return;
	}

	//check if we can fit an additional resource container in our inventory
	if(!inventory->checkSlots(1))
	{
		gMessageLib->sendSystemMessage(playerObject,L"","error_message","inv_full");
		return;
	}

	string dataStr;

	message->getStringUnicode16(dataStr);
	dataStr.convert(BSTRType_ANSI);

	BStringVector dataElements;
	uint16 elementCount = dataStr.split(dataElements,' ');

	if(!elementCount)
	{
		gLogger->logMsg("ObjectController::_handleResourceContainerSplit: Error in requestStr");
		return;
	}

	uint32	splitOffAmount	= boost::lexical_cast<uint32>(dataElements[0].getAnsi());
	uint64	parentId		= boost::lexical_cast<uint64>(dataElements[1].getAnsi());

	// update selected container contents
	selectedContainer->setAmount(selectedContainer->getAmount() - splitOffAmount);

	gMessageLib->sendResourceContainerUpdateAmount(selectedContainer,playerObject);

	mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%lld",selectedContainer->getAmount(),selectedContainer->getId());

	// create a new one
	gObjectFactory->requestNewResourceContainer(inventory,(selectedContainer->getResource())->getId(),parentId,99,splitOffAmount);

}

//======================================================================================================================

