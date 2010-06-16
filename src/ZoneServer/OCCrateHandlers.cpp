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

#include <cassert>

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

	if(!crate)
	{
		gLogger->log(LogManager::DEBUG,"ObjectController::_ExtractObject: Crate does not exist!");
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
			gLogger->log(LogManager::CRITICAL,"ObjectController::_ExtractObject: Crates parent does not exist!");
			assert(false && "ObjectController::_ExtractObject inventory must be a tangible object");
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
		TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(crate->getParentId()));
		tO->deleteObject(crate);
		return;
	}
	
	if(content < 0)
	{
		gLogger->log(LogManager::CRITICAL,"ObjectController::_ExtractObject: the crate now has negative content!");
		assert(false && "ObjectController::_ExtractObject crate must not have negative content");
		return;
	}

	gMessageLib->sendUpdateCrateContent(crate,playerObject);

	return;

	
}
