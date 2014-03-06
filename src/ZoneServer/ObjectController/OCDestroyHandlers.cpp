/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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
#include "ZoneServer/Objects/CraftingTool.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/BuildingObject.h"
#include "Zoneserver/Objects/Datapad.h"
#include "Zoneserver/Objects/Inventory.h"
#include "Zoneserver/Objects/Item.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/ManufacturingSchematic.h"
#include "Zoneserver/ObjectController/ObjectController.h"
#include "ZoneServer/ObjectController/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/Objects/Object/ObjectFactory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/Objects/VehicleController.h"

#include "ZoneServer\Services\equipment\equipment_service.h"

#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceContainer.h"
#include "Zoneserver/Objects/SurveyTool.h"
#include "Zoneserver/Objects/waypoints/WaypointObject.h"
#include "ZoneServer/Objects/wearable.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"

#include "MessageLib/MessageLib.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"


//======================================================================================================================
//
// server destroy object
// remove from db if indicated, then call worldmanager destroyObject
// I doubt we need an ObjectController Destroy AND a WorldManager Destroy!
//the ObjectController IS THE CHILD of an Object

void ObjectController::destroyObject(uint64 objectId)
{
	
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	Datapad*		datapad			= playerObject->getDataPad();
	Object*			object			= gWorldManager->getObjectById(objectId);

	auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(playerObject->GetCreature(), "inventory"));

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

	

	// or something else
	if(object == NULL)
	{
		DLOG(info) << "ObjController::destroyObject: could not find object " << objectId;
		return;
	}

	// waypoint
	if(object->getType() == ObjType_Waypoint)
	{
		// delete from db
		gObjectFactory->deleteObjectFromDB(object);
		
		if(!datapad)	{
			LOG(error) << "ObjectController::destroyObject cant get datapad to destroy waypoint : " << object->getId();
			return;
		}

		datapad->RemoveWaypoint(object->getId());

		//remove from grid and/or container
		gWorldManager->destroyObject(object);
		
	}

	//Inangible Objects
	if(object->getType() == ObjType_Intangible)
	{
		
		gObjectFactory->deleteObjectFromDB(object);
		
		//remove from grid and/or container
		gWorldManager->destroyObject(object);
		
	}


	// tangible
	else if(object->getType() == ObjType_Tangible)
	{
		TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);
		
		auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
		auto inventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(playerObject->GetCreature(), "inventory"));

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

		}
		
		
		// reset pending ui callbacks
		playerObject->resetUICallbacks(object);

		

		// delete from db CAVE :: mark if its an Object saved in the db!!!!
		// temporary placed instruments are not saved in the db
		gObjectFactory->deleteObjectFromDB(object);

		//remove from grid and/or container and/or World
		gWorldManager->destroyObject(object);
		
	}
}

//======================================================================================================================
//
// server destroy object
//

void ObjectController::_handleServerDestroyObject(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    BString volume;

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

	auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto held_item = dynamic_cast<Item*>(equip_service->GetEquippedObject(playerObject, "hold_r"));

    // first, stop playing, if its currently in use
    if(playerObject->GetCreature()->getPerformingState() == PlayerPerformance_Music)
    {
        // equipped instrument
        if(item == held_item  || playerObject->getPlacedInstrumentId())
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
            return;
        }

        permanentInstrument = dynamic_cast<Item*>(gWorldManager->getObjectById(tempInstrument->getPersistantCopy()));

        if(!permanentInstrument)
        {
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



