/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

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
#include "TravelMapHandler.h"
#include "BankTerminal.h"
#include "CurrentResource.h"
#include "WorldConfig.h"
#include "UIManager.h"
#include "Item.h"
#include "SurveyTool.h"
#include "Wearable.h"
#include "NPCObject.h"
#include "CraftingTool.h"


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
			gLogger->logMsg("ObjectController::_handleResourceContainerTransfer: Error in requestStr\n");
			return;
		}

		ResourceContainer* targetContainer = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(_atoi64(dataElements[0].getAnsi())));

		if(targetContainer && targetContainer->getResourceId() == selectedContainer->getResourceId())
		{
			uint32	targetAmount	= targetContainer->getAmount();
			uint32	selectedAmount	= selectedContainer->getAmount();
			uint32	maxAmount		= targetContainer->getMaxAmount();
			uint32	newAmount;

			gLogger->logMsg("transfer  resi\n");
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

	gLogger->logMsgF("ObjectController::_handleResourceContainerSplit: Container : %I64u\n",MSG_NORMAL,targetId);

	if(selectedContainer)
	{
		string dataStr;

		message->getStringUnicode16(dataStr);
		dataStr.convert(BSTRType_ANSI);

		BStringVector dataElements;
		uint16 elementCount = dataStr.split(dataElements,' ');

		if(!elementCount)
		{
			gLogger->logMsg("ObjectController::_handleResourceContainerSplit: Error in requestStr\n");
			return;
		}

		uint32	splitOffAmount	= _atoi64(dataElements[0].getAnsi());
		uint64	parentId		= _atoi64(dataElements[1].getAnsi());

		// update selected container contents
		selectedContainer->setAmount(selectedContainer->getAmount() - splitOffAmount);

		gMessageLib->sendResourceContainerUpdateAmount(selectedContainer,playerObject);

		mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%lld",selectedContainer->getAmount(),selectedContainer->getId());

		// create a new one
		gObjectFactory->requestNewResourceContainer(dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)),(selectedContainer->getResource())->getId(),parentId,99,splitOffAmount);
	}
}

//======================================================================================================================

