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

#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Inventory.h"
#include "Item.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "ResourceContainer.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "Common/Message.h"

#include <boost/lexical_cast.hpp>

#include <cassert>

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
			gLogger->log(LogManager::DEBUG,"ObjectController::_handleResourceContainerTransfer: Error in requestStr");
			return;
		}

		ResourceContainer* targetContainer = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(boost::lexical_cast<uint64>(dataElements[0].getAnsi())));

		if(targetContainer && targetContainer->getResourceId() == selectedContainer->getResourceId())
		{
			uint32	targetAmount	= targetContainer->getAmount();
			uint32	selectedAmount	= selectedContainer->getAmount();
			uint32	maxAmount		= targetContainer->getMaxAmount();
			uint32	newAmount;

			gLogger->log(LogManager::DEBUG,"transfer  resi");
			// all fits
			if((newAmount = targetAmount + selectedAmount) <= maxAmount)
			{
				// update target container
				targetContainer->setAmount(newAmount);

				gMessageLib->sendResourceContainerUpdateAmount(targetContainer,playerObject);

				mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%"PRIu64"",newAmount,targetContainer->getId());

				// delete old container
				gMessageLib->sendDestroyObject(selectedContainer->getId(),playerObject);

				gObjectFactory->deleteObjectFromDB(selectedContainer);
				TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(selectedContainer->getParentId()));
				tO->deleteObject(selectedContainer);



			}
			// target container full, update both contents
			else if(newAmount > maxAmount)
			{
				uint32 selectedNewAmount = newAmount - maxAmount;
				targetContainer->setAmount(maxAmount);
				selectedContainer->setAmount(selectedNewAmount);

				gMessageLib->sendResourceContainerUpdateAmount(targetContainer,playerObject);
				gMessageLib->sendResourceContainerUpdateAmount(selectedContainer,playerObject);

				mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%"PRIu64"",maxAmount,targetContainer->getId());
				mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%"PRIu64"",selectedNewAmount,selectedContainer->getId());
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

	gLogger->log(LogManager::DEBUG,"ObjectController::_handleResourceContainerSplit: Container : %I64u",targetId);

	if(!selectedContainer)
	{
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleResourceContainerSplit: Container does not exist!");
		return;
	}

	string dataStr;

	message->getStringUnicode16(dataStr);
	dataStr.convert(BSTRType_ANSI);

	BStringVector dataElements;
	uint16 elementCount = dataStr.split(dataElements,' ');

	if(!elementCount)
	{
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleResourceContainerSplit: Error in requestStr");
		return;
	}

	uint32	splitOffAmount	= boost::lexical_cast<uint32>(dataElements[0].getAnsi());
	uint64	parentId		= boost::lexical_cast<uint64>(dataElements[1].getAnsi());


	if(selectedContainer->getParentId() == inventory->getId())
	{
		//check if we can fit an additional resource container in our inventory
		if(!inventory->checkSlots(1))
		{
			gMessageLib->sendSystemMessage(playerObject,L"","error_message","inv_full");
			return;
		}

		mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%"PRIu64"",selectedContainer->getAmount(),selectedContainer->getId());

		// create a new one
		// update selected container contents
		selectedContainer->setAmount(selectedContainer->getAmount() - splitOffAmount);
		gMessageLib->sendResourceContainerUpdateAmount(selectedContainer,playerObject);

		gObjectFactory->requestNewResourceContainer(inventory,(selectedContainer->getResource())->getId(),parentId,99,splitOffAmount);
		return;
	}

	Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(parentId));
	if(!item)
	{
		gLogger->log(LogManager::DEBUG,"ObjectController::_ExtractObject: resourcecontainers parent does not exist!");
		assert(false && "ObjectController::_ExtractObject resourcecontainers parent does not exist");
		return;
	}
	
	if(!item->checkCapacity())
	{
		//check if we can fit an additional item in our inventory
		gMessageLib->sendSystemMessage(playerObject,L"","container_error_message","container3");
		return;
	}
	// update selected container contents
	selectedContainer->setAmount(selectedContainer->getAmount() - splitOffAmount);

	gMessageLib->sendResourceContainerUpdateAmount(selectedContainer,playerObject);

	gObjectFactory->requestNewResourceContainer(item,(selectedContainer->getResource())->getId(),parentId,99,splitOffAmount);
	
}

