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
#include "Datapad.h"
#include "Inventory.h"
#include "Item.h"
#include "ManufacturingSchematic.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "VehicleController.h"

//#include "IntangibleObject.h"
#include "ResourceContainer.h"
#include "SurveyTool.h"
#include "WaypointObject.h"
#include "Wearable.h"
#include "WorldManager.h"
#include "WorldConfig.h"
#include "UIManager.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"


//======================================================================================================================
//
// server destroy object
//

void ObjectController::destroyObject(uint64 objectId)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	Datapad* datapad				= playerObject->getDataPad();
	Object*			object			= gWorldManager->getObjectById(objectId);

	//could be a schematic!
	ManufacturingSchematic* schem	= datapad->getManufacturingSchematicById(objectId);

	if(schem != NULL)
	{
		//delete schematic
		datapad->removeManufacturingSchematic(objectId);

		//delete schematic object
		gObjectFactory->deleteObjectFromDB(schem);
		gMessageLib->sendDestroyObject(objectId,playerObject);

		return;
	}

	// could be a waypoint
	if(object == NULL)
	{
		object = datapad->getWaypointById(objectId);
	}

	// or something else
	if(object == NULL)
	{
		gLogger->log(LogManager::DEBUG,"ObjController::destroyObject: could not find object %"PRIu64"",objectId);

		return;
	}

	// waypoint
	if(object->getType() == ObjType_Waypoint)
	{
		// update our datapad
		if(!(datapad->removeWaypoint(objectId)))
		{
			gLogger->log(LogManager::DEBUG,"ObjController::handleDestroyObject: Error removing Waypoint from datapad %"PRIu64"",objectId);
		}

		gMessageLib->sendUpdateWaypoint(dynamic_cast<WaypointObject*>(object),ObjectUpdateDelete,playerObject);

		// delete from db
		gObjectFactory->deleteObjectFromDB(object);

		delete(object);
	}

	//Inangible Objects
	if(object->getType() == ObjType_Intangible)
	{
		//update the datapad
		if(!(datapad->removeData(objectId)))
		{
			gLogger->log(LogManager::DEBUG,"ObjController::handleDestroyObject: Error removing Data from datapad %"PRIu64"",objectId);
		}

		if(VehicleController* vehicle = dynamic_cast<VehicleController*>(object))
		{
			vehicle->Store();
		}
		
		gObjectFactory->deleteObjectFromDB(object);
		gMessageLib->sendDestroyObject(objectId,playerObject);

		delete(object);

	}


	// tangible
	else if(object->getType() == ObjType_Tangible)
	{
		TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);
		Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

		// items
		if(Item* item = dynamic_cast<Item*>(tangibleObject))
		{
			// handle any family specifics
			switch(item->getItemFamily())
			{
				case ItemFamily_CraftingTools:	_handleDestroyCraftingTool(dynamic_cast<CraftingTool*>(item));	break;
				case ItemFamily_Instrument:		_handleDestroyInstrument(item);									break;

				default:break;
			}

			// update the equiplist, if its an equipable item
			CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(item->getParentId()));
			if(creature)
			{
				// remove from creatures slotmap
				creature->getEquipManager()->removeEquippedObject(object);

				//unequip it
				object->setParentId(inventory->getId());
				gMessageLib->sendContainmentMessage_InRange(object->getId(),inventory->getId(),0xffffffff,creature);

				// send out the new equiplist
				gMessageLib->sendEquippedListUpdate_InRange(creature);				
			}
		}
		//tangible includes items and resourcecontainers
		if(TangibleObject* tangible = dynamic_cast<TangibleObject*>(object))
		{
			//if(tangible->getObjectMainParent(object) != inventory->getId())
			if(tangibleObject->getKnownPlayers()->size())
			{
				//this automatically destroys the object for the players in its vicinity
				tangibleObject->destroyKnownObjects();
			}
			else
			{
				// destroy it for the player
				gMessageLib->sendDestroyObject(objectId,playerObject);
			}

		}
		
		// reset pending ui callbacks
		playerObject->resetUICallbacks(object);

		// delete from db CAVE :: mark if its an Object saved in the db!!!!
		// temporary placed instruments are not saved in the db
		gObjectFactory->deleteObjectFromDB(object);

		//it might be in a cell or in a container or in the inventory :)
		ObjectContainer* oc = dynamic_cast<ObjectContainer*>(gWorldManager->getObjectById(object->getParentId()));
		if(oc)
		{
			oc->deleteObject(object);
		}
		else
		{
			gWorldManager->destroyObject(object);
		}
	}
}

//======================================================================================================================
//
// server destroy object
//

void ObjectController::_handleServerDestroyObject(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	string volume;

	message->getStringUnicode16(volume);

	destroyObject(targetId);
}

//========================================================================================
//
// handleDestroyCraftingTool
//

void ObjectController::_handleDestroyCraftingTool(CraftingTool* tool)
{
	// if the tool is currently busy, remove it from the processlist
	// and delete the item it contains
	// keeping items in tools, when logging out, is currently not supported

	if(Item* currentToolItem = tool->getCurrentItem())
	{
		gWorldManager->removeBusyCraftTool(tool);

		gMessageLib->sendDestroyObject(currentToolItem->getId(),dynamic_cast<PlayerObject*>(mObject));
		gObjectFactory->deleteObjectFromDB(currentToolItem);
	}
}

//========================================================================================
//
// handleDestroyInstrument
//

void ObjectController::_handleDestroyInstrument(Item* item)
{
	PlayerObject*	playerObject		= dynamic_cast<PlayerObject*>(mObject);
	Item*			tempInstrument		= NULL;
	Item*			permanentInstrument	= NULL;

	// first, stop playing, if its currently in use
	if(playerObject->getPerformingState() == PlayerPerformance_Music)
	{
		// equipped instrument
		if(item == dynamic_cast<Item*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hold_Left))
		|| playerObject->getPlacedInstrumentId())
		{
			gEntertainerManager->stopEntertaining(playerObject);
		}
	}

	// handle destruction of instanced instruments, placed in world
	if(playerObject->getPlacedInstrumentId())
	{
		// get the instruments
		tempInstrument = dynamic_cast<Item*>(gWorldManager->getObjectById(playerObject->getPlacedInstrumentId()));

		if(!tempInstrument)
		{
			gLogger->log(LogManager::DEBUG,"ObjectController::handleDestroyInstrument : no temporary Instrument");
			return;
		}

		permanentInstrument = dynamic_cast<Item*>(gWorldManager->getObjectById(tempInstrument->getPersistantCopy()));

		if(!permanentInstrument)
		{
			gLogger->log(LogManager::DEBUG,"ObjectController::handleDestroyInstrument : no parent Instrument");
			return;
		}

		// the temporary gets ALWAYS deleted
		// update the attributes of the permanent Instrument
		if(tempInstrument == item)
		{
			permanentInstrument->setPlaced(false);
			permanentInstrument->setNonPersistantCopy(0);
			playerObject->setPlacedInstrumentId(0);
		}
		// it is the permanent Instrument delete the temporary copy too
		else if(permanentInstrument == item)
		{
			destroyObject(tempInstrument->getId());
		}
	}
}

//========================================================================================



