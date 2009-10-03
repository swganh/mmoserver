/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectFactory.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "WorldConfig.h"
#include "UIManager.h"
#include "Item.h"
#include "SurveyTool.h"
#include "Wearable.h"
#include "CraftingTool.h"


//======================================================================================================================
//
// server destroy object
//

void ObjectController::destroyObject(uint64 objectId)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	Datapad*		datapad			= dynamic_cast<Datapad*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
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
		gLogger->logMsgF("ObjController::destroyObject: could not find object %lld",MSG_NORMAL,objectId);

		return;
	}

	// waypoint
	if(object->getType() == ObjType_Waypoint)
	{
		// update our datapad
		if(!(datapad->removeWaypoint(objectId)))
		{
			gLogger->logMsgF("ObjController::handleDestroyObject: Error removing Waypoint from datapad %lld",MSG_NORMAL,objectId);
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
			gLogger->logMsgF("ObjController::handleDestroyObject: Error removing Data from datapad %lld",MSG_NORMAL,objectId);
		}

		gObjectFactory->deleteObjectFromDB(object);
		gMessageLib->sendDestroyObject(objectId,playerObject);

	}


	// tangible 
	else if(object->getType() == ObjType_Tangible)
	{
		TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);

		// items
		if(Item* item = dynamic_cast<Item*>(tangibleObject))
		{
			// Does the item have a owner?
			/*
			if (item->getOwner() != 0)
			{
				gLogger->logMsgF("ObjController::handleDestroyObject: OwnerId = %llu, playerId = %llu", MSG_NORMAL, item->getOwner(), playerObject->getId());
				// Yes, is it my item?
				if (item->getOwner() != playerObject->getId())
				{
					// Not allowed to delete this item.
					gMessageLib->sendSystemMessage(playerObject,L"","error_message","insufficient_permissions");
					return;
				}
			}
			*/

			// handle any family specifics
			switch(item->getItemFamily())
			{
				case ItemFamily_CraftingTools:	_handleDestroyCraftingTool(dynamic_cast<CraftingTool*>(item));	break;
				case ItemFamily_Instrument:		_handleDestroyInstrument(item);									break;

				default:break;
			}

			// destroy it for the player
			gMessageLib->sendDestroyObject(objectId,playerObject);

			// Also update the world...if the object is not private.
			if ((item->getParentId() != playerObject->getId()) && (item->getParentId() != dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getId()))
			{
				PlayerObjectSet* inRangePlayers	= playerObject->getKnownPlayers();
				PlayerObjectSet::iterator it = inRangePlayers->begin();
				while(it != inRangePlayers->end())
				{
					PlayerObject* targetObject = (*it);
					gMessageLib->sendDestroyObject(tangibleObject->getId(),targetObject);
					targetObject->removeKnownObject(tangibleObject);
					++it;
				}
				tangibleObject->destroyKnownObjects();	
			}

			// update the equiplist, if its an equipable item
			if(item->hasInternalAttribute("equipped"))
			{
				if(item->getInternalAttribute<bool>("equipped"))
				{
					// remove from creatures slotmap
					playerObject->getEquipManager()->removeEquippedObject(object);

					// send out the new equiplist
					gMessageLib->sendEquippedListUpdate_InRange(playerObject);

					// destroy it for players in range
					PlayerObjectSet* objList		= playerObject->getKnownPlayers();
					PlayerObjectSet::iterator it	= objList->begin();

					while(it != objList->end())
					{
						gMessageLib->sendDestroyObject(objectId,(*it));

						++it;
					}
				}
			}
		}
		else
		if(ResourceContainer* container = dynamic_cast<ResourceContainer*>(object))
		{
			//gLogger->logMsg("destroy ressourcecontainer");
			gMessageLib->sendDestroyObject(object->getId(),playerObject);
		}

		// reset pending ui callbacks
		playerObject->resetUICallbacks(object);

		// delete from db CAVE :: mark if its an Object saved in the db!!!!
		// temporary placed instruments are not saved in the db
		gObjectFactory->deleteObjectFromDB(object);

		// remove from inventory
		if(object->getParentId() == dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getId())
		{
			dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->deleteObject(object);
		}
		// remove from world
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
		if(item == dynamic_cast<Item*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Instrument))
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
			gLogger->logMsg("ObjectController::handleDestroyInstrument : no temporary Instrument\n");
			return;
		}

		permanentInstrument = dynamic_cast<Item*>(gWorldManager->getObjectById(tempInstrument->getPersistantCopy()));
		
		if(!permanentInstrument)
		{
			gLogger->logMsg("ObjectController::handleDestroyInstrument : no parent Instrument\n");
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



