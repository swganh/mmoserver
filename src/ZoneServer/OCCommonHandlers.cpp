/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Bank.h"
#include "BankTerminal.h"
#include "CellObject.h"
#include "Container.h"
#include "ContainerObjectFactory.h"
#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Datapad.h"
#include "IntangibleObject.h"
#include "Inventory.h"
#include "Item.h"
#include "ManufacturingSchematic.h"
#include "MissionObject.h"
#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "QuadTree.h"
#include "ResourceContainer.h"
#include "ResourceManager.h"
#include "Shuttle.h"
#include "SurveyTool.h"
#include "TravelMapHandler.h"
#include "TravelTerminal.h"
#include "TreasuryManager.h"
#include "Tutorial.h"
#include "UIManager.h"
#include "Wearable.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"
#include "Utils/clock.h"
#include "ZoneTree.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "CraftingSession.h"


void ObjectController::_handleBoardTransport(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);

	ObjectSet		inRangeObjects;
	float boardingRange = 25.0;

	if(playerObject->getPosture() == CreaturePosture_SkillAnimating)
	{
		gMessageLib->sendSystemMessage(playerObject,L"You cannot do that at this time.");
		return;
		//gEntertainerManager->stopEntertaining(player);
	}

	string str;
	message->getStringUnicode16(str);
	str.convert(BSTRType_ANSI);
	str.toLower();
	if((str.getCrc() != BString("transport").getCrc()))
	{
		gMessageLib->sendSystemMessage(playerObject,L"","travel","boarding_what_shuttle");
		return;
	}


	mSI->getObjectsInRange(playerObject,&inRangeObjects,ObjType_Creature | ObjType_NPC,boardingRange);

	// iterate through the results
	ObjectSet::iterator it = inRangeObjects.begin();

	while(it != inRangeObjects.end())
	{
		CreatureObject* creature = dynamic_cast<CreatureObject*> (*it);

		if(creature)
		{

			if(creature->getCreoGroup() == CreoGroup_Shuttle)
			{
				Shuttle* shuttle = dynamic_cast<Shuttle*> (creature);

				if(shuttle)
				{

					// in range check
					if(playerObject->getParentId() !=  shuttle->getParentId() )
					{
						gMessageLib->sendSystemMessage(playerObject,L"","travel","boarding_too_far");
						return;
					}

					if (!shuttle->avaliableInPort())
					{
						gMessageLib->sendSystemMessage(playerObject,L"","travel","shuttle_not_available");
						return;
					}

					shuttle->useShuttle(playerObject);
					return;
				}
			}

		}

		++it;
	}

	gMessageLib->sendSystemMessage(playerObject,L"","structure/structure_messages","boarding_what_shuttle");
}

//=============================================================================
//
// open container
//

void ObjectController::_handleOpenContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// gLogger->logMsg("ObjController::_handleOpenContainer:");

	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	Object*			itemObject		= gWorldManager->getObjectById(targetId);

	// gLogger->hexDump(message->getData(),message->getSize());
/*
	46 5E CE 80 23 00 00 00 16 01 00 00 79 69 00 00
	02 00 00 00 00 00 00 00 00 00 00 00 86 75 17 70
	B3 2C 81 00 00 00 00 00 02 00 00 00 31 00 20 00

	// 86 75 17 70 = The command OpenContainer
	// B3 2C 81 00 00 00 00 00 = container id
	// 02 00 00 00 31 00 20 00 = 2, "1 ", i.e Number of uni-chars (32 bits) and no of Open " 1", " 2".... " 21" where the " " is the last byte in the data.
*/

	// Let's make a "fake open" if the object is not a container.
	if (itemObject)
	{
		bool aContainer = false;

		if (gWorldConfig->isTutorial())
		{
			playerObject->getTutorial()->containerOpen(targetId);
		}

		if (itemObject->getType() == ObjType_Tangible)
		{
			TangibleObject* tangObj = dynamic_cast<TangibleObject*>(itemObject);
			if (tangObj->getTangibleGroup() == TanGroup_Container)
			{
				// Request container contence.
				gContainerFactory->requestObject(this,targetId,TanGroup_Container,0,playerObject->getClient());
				aContainer = true;
			}
		}
		if (!aContainer)
		{
			// Open an empty container. And we will actually see what kind of container it is.
			gMessageLib->sendOpenedContainer(targetId, playerObject);
		}
	}
	else
	{
		gLogger->logMsgF("ObjectController::_handleOpenContainer: INVALID Object id %"PRIu64"",MSG_NORMAL,targetId);
	}
}

void ObjectController::_handleCloseContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// gLogger->logMsg("ObjController::_handleCloseContainer:");

	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	//Object*			itemObject		= gWorldManager->getObjectById(targetId);

	if (gWorldConfig->isTutorial())
	{
		playerObject->getTutorial()->containerClose(targetId);
	}


	// gLogger->hexDump(message->getData(),message->getSize());
	// gLogger->logMsgF("ObjectController::_handleCloseContainer: targetId = %"PRIu64"",MSG_NORMAL,targetId);
	// gMessageLib->sendOpenedContainer(targetId, playerObject);
}


//=============================================================================
//
// transfer item
//

void ObjectController::_handleTransferItem(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	=	dynamic_cast<PlayerObject*>(mObject);
	Object*			itemObject		=	gWorldManager->getObjectById(targetId);
	Inventory*		inventory		=	dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	string			dataStr;
	uint64			targetContainerId;
	uint32			linkType;
	float			x,y,z;


	gLogger->logMsg("ObjController::_handleTransferItem: called");

	message->getStringUnicode16(dataStr);

	// gLogger->logMsg("ObjectController::_handleTransferItem() THIS IS (almost) UNSUPPORTED");

	if(swscanf(dataStr.getUnicode16(),L" %"WidePRIu64 L" %u %f %f %f",&targetContainerId,&linkType,&x,&y,&z) != 5)
	{
		gLogger->logMsg("ObjController::handleTransferItem: Error in parameters");
		return;
	}
	// gLogger->logMsgF("Parameters:  %"PRIu64" %d %.1f %.1f %.1f", MSG_NORMAL, targetContainerId, linkType, x, y, z);
	// gLogger->logMsgF("TargetId = %"PRIu64"", MSG_NORMAL, targetId);

	if(itemObject)
	{
		Item* item = dynamic_cast<Item*>(itemObject);

		// We may want to transfer other things than items...
		// Right now is this code used for "/pickup" of all objects, and a non-item object will crash the server.
		if (item)
		{
			//check if its only temporarily placed
			if(item->getItemFamily() == ItemFamily_Instrument)
			{
				if(item->getPersistantCopy())
				{
					gMessageLib->sendSystemMessage(playerObject,L"You cannot pick this up.");
					return;
				}
			}
		}
		else
		{
			gLogger->logMsg("ObjController::_handleTransferItem: This is not an item");
			return;
		}


		itemObject->mPosition = Anh_Math::Vector3(x,y,z);

		// other transfers, skip for now
		// drop in cell
		CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(targetContainerId));
		if(cell)
		{
			itemObject->setParentId(targetContainerId);
			//gMessageLib->sendContainmentMessage(targetId,targetContainerId,linkType,playerObject);

			playerObject->addKnownObjectSafe(itemObject);

			if(targetContainerId)
			{
				if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(targetContainerId)))
				{
					cell->addObjectSecure(itemObject);
				}
				else
				{
					gLogger->logMsgF("ObjectController::_handleTransferItem: couldn't find cell %"PRIu64"",MSG_HIGH,targetContainerId);
				}
			}
			else
			{
				if(QTRegion* region = gWorldManager->getSI()->getQTRegion(itemObject->mPosition.mX,itemObject->mPosition.mZ))
				{
					itemObject->setSubZoneId((uint32)region->getId());
					region->mTree->addObject(itemObject);
				}
				else
				{
					// we should never get here !
					gLogger->logMsg("ObjectController::_handleTransferItem: could not find zone region in map");
					return;
				}
			}

			// initialize objects in range

			TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);
			//gWorldManager->initPlayersInRange(itemObject);

			PlayerObjectSet*			inRangePlayers	= playerObject->getKnownPlayers();
			PlayerObjectSet::iterator	it				= inRangePlayers->begin();
			while(it != inRangePlayers->end())
			{
				PlayerObject* targetObject = (*it);
				gMessageLib->sendCreateTangible(tangible,targetObject);
				targetObject->addKnownObjectSafe(tangible);
				tangible->addKnownObjectSafe(targetObject);
				++it;
			}
			//mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id=%I64u WHERE item_id=%I64u",itemObject->getParentId() ,itemObject->getId());
			//printf("linktype %u",linkType);
			//gMessageLib->sendContainmentMessage(targetId,targetContainerId,linkType,playerObject);
			gMessageLib->sendDestroyObject(tangible->getId(),playerObject);
			tangible->mPosition = playerObject->mPosition;
			tangible->mDirection = playerObject->mDirection;

			gMessageLib->sendCreateTangible(tangible,playerObject);
			//gMessageLib->sendDataTransformWithParent(tangible);
			playerObject->addKnownObjectSafe(tangible);
			tangible->addKnownObjectSafe(playerObject);

			inventory->removeObject(tangible);

			return;//
		}

		// pick up - check whether target object is a container or an inventory
		cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(itemObject->getParentId()));
		if(cell)
		{
			//stop playing if we pick up the (permanently placed) instrument we are playing
			Item* item = dynamic_cast<Item*>(itemObject);
			if(item)
			{
				if(item->getItemFamily() == ItemFamily_Instrument)
				{
					//is it the one we are playing ?
					if((playerObject->getPerformingState() == PlayerPerformance_Music)&&(gEntertainerManager->gettargetedInstrument(playerObject)==item->getId()))
					gEntertainerManager->stopEntertaining(playerObject);
				}
			}

			itemObject->setParentId(targetContainerId);

			if(TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject))
			{
				//uint64 parentId = itemObject->getParentId();
				cell->removeObject(itemObject);

				//update known players
				PlayerObjectSet*			inRangePlayers	= playerObject->getKnownPlayers();
				PlayerObjectSet::iterator	it				= inRangePlayers->begin();
				while(it != inRangePlayers->end())
				{
					PlayerObject* targetObject = (*it);
					gMessageLib->sendDestroyObject(tangible->getId(),targetObject);
					targetObject->removeKnownObject(tangible);
					++it;
				}
				itemObject->destroyKnownObjects();
				//mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id=%I64u WHERE item_id=%I64u",itemObject->getParentId() ,itemObject->getId());
				gMessageLib->sendDestroyObject(tangible->getId(),playerObject);
				gMessageLib->sendCreateTangible(tangible,playerObject);
				inventory->addObject(tangible);
				//tangible->mPosition = entertainer->mPosition;
			//usedInstrument->mDirection = entertainer->mDirection;
			//gMessageLib->sendDataTransformWithParent(usedInstrument);
			}


				// destroy known objects

			//if (itemObject->getType() ==)
			//itemObject->setInternalAttribute("instrument_placed","0");
			//remove from the sis
			return;//itemObject->setParentId(playerObject->getInventory()->getId());
		}

		switch(itemObject->getType())
		{
			case ObjType_Tangible:
			{
				if(dynamic_cast<TangibleObject*>(itemObject)->getTangibleGroup() == TanGroup_Item && itemObject->hasInternalAttribute("equipped"))
				{
					Item*						item			= dynamic_cast<Item*>(itemObject);
					PlayerObjectSet*			inRangePlayers	= playerObject->getKnownPlayers();
					PlayerObjectSet::iterator	it				= inRangePlayers->begin();

					// unequip it
					if(item->getInternalAttribute<bool>("equipped"))
					{
						inventory->unEquipItem(item);
						return;
					}
					// equip it
					else
					{
						inventory->EquipItem(item);
						return;

					}
				}
			}
			break;

			default:break;
		}
	}
}

//=============================================================================
//
// checks whether we have access to the container containing the item
//

bool ObjectController::checkContainingContainer(uint64 containingContainer)
{
	CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(containingContainer));
	return true;
}

//=============================================================================
//
// checks whether we have a valid and useable targetcontainer
//


bool ObjectController::checkTargetContainer(uint64 targetContainerId, Object* object)
{
	PlayerObject*	playerObject	=	dynamic_cast<PlayerObject*>(mObject);
	Inventory*		inventory		=	dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	ObjectContainer* targetContainer = dynamic_cast<ObjectContainer*>(gWorldManager->getObjectById(targetContainerId));

	//the inventory is *NOT* part of the worldmanagers ObjectMap  
	//our inventory is save to take stuff out
	if(inventory&& (inventory->getId() == targetContainerId))
	{
		//check space
		return true;
	}

	if(playerObject->getId() == targetContainerId)
	{
		//check for equip restrictions!!!!
		return inventory->EquipItemTest(object);
		
	}
	
	if(!targetContainer)
	{
		gLogger->logMsg("ObjController::_handleTransferItemMisc: TargetContainer is NULL :(");
		return false;
	}

	Container* container = dynamic_cast<Container*>(gWorldManager->getObjectById(targetContainerId));
	if (container)
	{
		if (gWorldConfig->isTutorial())
		{
			// We don't allow users to place item in the container.
			// gMessageLib->sendSystemMessage(playerObject,L"","event_perk","chest_can_not_add");
			gMessageLib->sendSystemMessage(playerObject,L"","error_message","remove_only");
			return false;
		}
	}

	//lets begin by getting the containing Object
	if (CellObject* cell = dynamic_cast<CellObject*>(targetContainer))
	{
		//do we have permission to drop the item here ???

	}

	//we also might want to check this for factories hoppers
	TangibleObject* tangibleContainer = dynamic_cast<TangibleObject*>(targetContainer);
	if((tangibleContainer)&&((strcmp(tangibleContainer->getName().getAnsi(),"ingredient_hopper")==0)||(strcmp(tangibleContainer->getName().getAnsi(),"output_hopper")==0)))
	{
		//do we have access rights to the factories hopper?? this would have to be checked asynchronously
	}

	

	return true;

}

//=============================================================================
//
// removes our item from the container containing it
//

bool ObjectController::removeFromContainer(uint64 targetContainerId, uint64 targetId)
{
	PlayerObject*	playerObject	=	dynamic_cast<PlayerObject*>(mObject);
	Object*			itemObject		=	gWorldManager->getObjectById(targetId);
	Inventory*		inventory		=	dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	TangibleObject* targetContainer = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(targetContainerId));

	TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);

	Item* item = dynamic_cast<Item*>(itemObject);

	// its us
	if (tangible->getParentId() == playerObject->getId())
	{
		if ((itemObject->hasInternalAttribute("equipped"))&&(itemObject->getInternalAttribute<bool>("equipped")))
		{
			// unequip it
			return inventory->unEquipItem(itemObject);
			
		}
		//help ... how can that happen an item contained by the player MUST be equipped?
		assert(false);
	}
	
	
	//its our inventory
	if (tangible->getParentId() == inventory->getId())
	{
		//gMessageLib->sendDestroyObject(targetId,playerObject);
		inventory->removeObject(itemObject);
			return true;

	}
	
	//the containerObject is the container used in the tutorial or some random dungeon container
	Container* container = dynamic_cast<Container*>(gWorldManager->getObjectById(tangible->getParentId()));
	if (container)
	{
		container->removeObject(itemObject);
		if (gWorldConfig->isTutorial())
		{
			playerObject->getTutorial()->transferedItemFromContainer(targetId, tangible->getParentId());

			// If object is owned by player (private owned for instancing), we remove the owner from the object.
			// what is this used for ???
			if (itemObject->getPrivateOwner() == playerObject->getId())
			{
				itemObject->setPrivateOwner(0);
			}
		}
		return true;
	}

	//its hard to get a creatures inventory .. it isnt part of the worldObjectMap
	CreatureObject* unknownCreature;
	Inventory*		creatureInventory;

	if (itemObject->getParentId() &&
		(unknownCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(itemObject->getParentId() - 1))) &&
		(creatureInventory = dynamic_cast<Inventory*>(unknownCreature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))) &&
		(creatureInventory->getId() == itemObject->getParentId()))
	{
		gLogger->logMsg("Transfer item from creature inventory to player inventory (looting)");
		// gMessageLib->sendContainmentMessage(targetId,itemObject->getParentId(),-1,playerObject);

		gMessageLib->sendDestroyObject(targetId,playerObject);
		creatureInventory->removeObject(itemObject);
		// gLogger->logMsg("Removed item from a creature inventory");

		ObjectIDList* invObjList = creatureInventory->getObjects();
		if (invObjList->size() == 0)
		{
			// Put this creature in the pool of delayed destruction and remove the corpse from scene.
			gWorldManager->addCreatureObjectForTimedDeletion(creatureInventory->getParentId(), LootedCorpseTimeout);
		}
		
		if (gWorldConfig->isTutorial())
		{
			// TODO: Update tutorial about the loot.
			// playerObject->getTutorial()->transferedItemFromContainer(targetId, sourceId);
		}
		// This ensure that we do not use/store any of the temp id's in the database.
		gObjectFactory->requestNewDefaultItem(inventory, item->getItemFamily(), item->getItemType(), inventory->getId(),99,Anh_Math::Vector3(),"");
		return false;

	}		   


	CellObject* cell;
	if(cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(itemObject->getParentId())))
	{
		gLogger->logMsgF("ObjectController::_handleTransferItemMisc: pick up from cell %"PRIu64"", MSG_NORMAL, itemObject->getParentId());
		
		// Stop playing if we pick up the (permanently placed) instrument we are playing
		if (item && (item->getItemFamily() == ItemFamily_Instrument))
		{
			uint32 instrumentType = item->getItemType();
			if ((instrumentType == ItemType_Nalargon) || (instrumentType == ItemType_omni_box) || (instrumentType == ItemType_nalargon_max_reebo))
			{
				// It's a placeable original instrument.
				// Are we targeting the instrument we actually play on?
				if (playerObject->getActiveInstrumentId() == item->getId())
				{
					gEntertainerManager->stopEntertaining(playerObject);
					// gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Picking up the active instrument", MSG_NORMAL);
				}
			}
		}
		
		// Remove object from cell.
		if (cell->removeObject(itemObject))
		{
			//gMessageLib->sendDestroyObject_InRange(tangible->getId(),playerObject,false);
			//gMessageLib->sendContainmentMessage(targetId,targetContainerId,linkType,playerObject);
	
			PlayerObjectSet* inRangePlayers	= playerObject->getKnownPlayers();
			PlayerObjectSet::iterator it = inRangePlayers->begin();
			while(it != inRangePlayers->end())
			{
				PlayerObject* targetObject = (*it);
				gMessageLib->sendDestroyObject(tangible->getId(),targetObject);
				targetObject->removeKnownObject(tangible);
				tangible->removeKnownObject(targetObject);
				++it;
			}
			return true;
		
		}
		else
		{
			assert(false);
		}

	}


	//some other container ... hopper backpack chest etc
	TangibleObject* containingContainer = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(tangible->getParentId()));
	if(containingContainer&&containingContainer->removeObject(itemObject))

	{
		playerObject->removeKnownObject(tangible);
		tangible->removeKnownObject(playerObject);
		return true;
	
	}
	
	return false;

}

//	Transfer items between player inventories, containers and cells. Also handles transfer from  creature inventories (looting).
//	Also handles trandfer between player inventory and player (equipping items).
//	We don't handle transfer to the world outside.
//

void ObjectController::_handleTransferItemMisc(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//we need to make sure that ONLY equipped items are contained by the player
	//all other items are contained by the inventory!!!!!!!!

	PlayerObject*	playerObject	=	dynamic_cast<PlayerObject*>(mObject);
	Object*			itemObject		=	gWorldManager->getObjectById(targetId);
	Inventory*		inventory		=	dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	string			dataStr;
	uint64			targetContainerId;
	uint32			linkType;
	float			x,y,z;
	CellObject*		cell;

	// gLogger->logMsg("ObjController::_handleTransferItemMisc: Entered");

	//gMessageLib->sendSystemMessage(playerObject,L"","error_message","insufficient_permissions");

	message->getStringUnicode16(dataStr);

	if(swscanf(dataStr.getUnicode16(),L"%"WidePRIu64 L" %u %f %f %f",&targetContainerId,&linkType,&x,&y,&z) != 5)
	{
		gLogger->logMsg("ObjController::_handleTransferItemMisc: Error in parameters");
		return;
	}

	// gLogger->logMsgF("Parameters:  %"PRIu64" %d %.1f %.1f %.1f", MSG_NORMAL, targetContainerId, linkType, x, y, z);
	// gLogger->logMsgF("TargetId = %"PRIu64"", MSG_NORMAL, targetId);
	if (!itemObject)
	{
		gLogger->logMsg("ObjController::_handleTransferItemMisc: No Object to transfer :(");
		//no Object :(
		return;
	}

	TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);
	if(!tangible)
	{
		//no tagible - get out of here
		gLogger->logMsg("ObjController::_handleTransferItemMisc: No tangible to transfer :(");
		return;
	}

	// We may want to transfer other things than items...basically tangibleObjects!
	// resourcecontainers / factory crates
	
	// first check whether its an instrument with persistant copy   - thats a special case!
	Item* item = dynamic_cast<Item*>(itemObject);
	if (item)
	{
		//check if its only temporarily placed
		if(item->getItemFamily() == ItemFamily_Instrument)
		{
			if(item->getPersistantCopy())
			{
				// gMessageLib->sendSystemMessage(playerObject,L"you cannot pick this up");
				// You bet, I can! Remove the temp instrument from the world.

				// Do I have access to this instrument?
				if (item->getOwner() == playerObject->getId())
				{
					playerObject->getController()->destroyObject(targetId);

				}
				return;
			}
		}
	}
	

	// A FYI: When we drop items, we use player pos.
	itemObject->mPosition = Anh_Math::Vector3(x,y,z);

	if (!targetContainerId)
	{
		gLogger->logMsg("ObjController::_handleTransferItemMisc:TargetContainer is 0 :(");
		//return;

	}
	
	//ok how to tackle this ... :
	//basically I want to use ObjectContainer as standard access point for item handling!
	//so far we have different accesses for Objects on the player and for the inventory	and for ContainerObjects and for cells ...

	//lets begin by getting the target Object

	if(!checkTargetContainer(targetContainerId,itemObject))
	{
		gLogger->logMsg("ObjController::_handleTransferItemMisc:TargetContainer is not valid :(");
		return;
	}

	TangibleObject* parentContainer = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(tangible->getParentId()));
	if(!parentContainer)
		parentContainer = inventory;

	if(!checkContainingContainer(tangible->getParentId()))
	{
		gLogger->logMsg("ObjController::_handleTransferItemMisc:ContainingContainer is not valid :(");
		return;

	}
	
	// Remove the object from whatever contains it.
	if(!removeFromContainer(targetContainerId, targetId))
	{
		gLogger->logMsgF("ObjectController::_handleTransferItemMisc: removeFromContainer failed :( this might be caused by looting a corpse though",MSG_NORMAL);
		return;
	}

				
	//now go and move it to wherever it belongs
	cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(targetContainerId));
	if (cell)
	{
		// drop in a cell
		gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Drop into cell %"PRIu64"", MSG_NORMAL, targetContainerId);

		//special case temp instrument
		if (item&&item->getItemFamily() == ItemFamily_Instrument)
		{
			if (playerObject->getPlacedInstrumentId())
			{
				// We do have a placed instrument.
				uint32 instrumentType = item->getItemType();
				if ((instrumentType == ItemType_Nalargon) || (instrumentType == ItemType_omni_box) || (instrumentType == ItemType_nalargon_max_reebo))
				{
					// We are about to drop the real thing, remove any copied instrument.
					// item->setOwner(playerObject->getId();
					playerObject->getController()->destroyObject(playerObject->getPlacedInstrumentId());
		
				}
			}
		}

	
		
		itemObject->mPosition = playerObject->mPosition;
		itemObject->mDirection = playerObject->mDirection;

		gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Cell added item to cell %I64u ", MSG_NORMAL,cell->getId());
		
		//do the db update manually because of the position - unless we get an automated position save in
		itemObject->setParentId(targetContainerId,linkType,playerObject,false); 
		
		ResourceContainer* rc = dynamic_cast<ResourceContainer*>(itemObject);
		if(rc)
			mDatabase->ExecuteSqlAsync(0,0,"UPDATE resource_containers SET parent_id ='%I64u', oY='%f', oZ='%f', oW='%f', x='%f', y='%f', z='%f' WHERE id='%I64u'",itemObject->getParentId(), itemObject->mDirection.mY, itemObject->mDirection.mZ, itemObject->mDirection.mW, itemObject->mPosition.mX, itemObject->mPosition.mY, itemObject->mPosition.mZ, itemObject->getId());
		else
			mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id ='%I64u', oY='%f', oZ='%f', oW='%f', x='%f', y='%f', z='%f' WHERE id='%I64u'",itemObject->getParentId(), itemObject->mDirection.mY, itemObject->mDirection.mZ, itemObject->mDirection.mW, itemObject->mPosition.mX, itemObject->mPosition.mY, itemObject->mPosition.mZ, itemObject->getId());

		// NOTE: It's quite possible to have the player update do this kind of updates manually,
		// but that will have a performance cost we don't ready to take yet.
		
		cell->addObjectSecure(itemObject,playerObject->getKnownPlayers());
		gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Player : %I64u contained in %I64u", MSG_NORMAL,playerObject->getId(),playerObject->getParentId());
		
	}	


	
	if (inventory && (inventory->getId() == targetContainerId))	// Valid player inventory.
	{
		// Add object to OUR inventory.
		itemObject->setParentId(targetContainerId,linkType,playerObject,true);
		inventory->addObjectSecure(itemObject);
		
		return;
		
	}
	
	
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetContainerId));
	if(player)
	{
		if(!item->getInternalAttribute<bool>("equipped"))
		{
			//equip / unequip handles the db side, too
			if(!inventory->EquipItem(item))
			{
				gLogger->logMsgF("ObjectController::_handleTransferItemMisc: Error equipping  %"PRIu64"", MSG_NORMAL, item->getId());
				//readd it to the old parent
				if(parentContainer)
					parentContainer->addObjectSecure(item);
			}
		}
		return;
	}

	//thats the tutorial container - leave them as separate class ?
	Container* container = dynamic_cast<Container*>(gWorldManager->getObjectById(targetContainerId));
	if (container && tangible)
	{
		
		// Add it to the container.
		// We dont have any access validations yet.
		container->addObject(itemObject);	  //just add its already created
		
		//set the new parent, send the contaiment and update the db
		itemObject->setParentId(targetContainerId,linkType,playerObject,true);
		return;
	}

	//some other container ... hopper backpack chest etc
	TangibleObject* receivingContainer = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(targetContainerId));
	if(receivingContainer)
	{
		receivingContainer->addObject(itemObject);
		itemObject->setParentId(receivingContainer->getId(),linkType,playerObject,true);
	}
	

	// gLogger->logMsgF("Object type: %d", MSG_NORMAL, itemObject->getType());
	
}


//=============================================================================
//
// purchase ticket
//

void ObjectController::_handlePurchaseTicket(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(mObject);
	string			dataStr;
	BStringVector	dataElements;
	uint16			elements;

	
	float		purchaseRange = gWorldConfig->getConfiguration("Player_TicketTerminalAccess_Distance",(float)10.0);

	if(playerObject->getPosture() == CreaturePosture_SkillAnimating)
	{
		gMessageLib->sendSystemMessage(playerObject,L"You cannot do that at this time.");
		return;
	}
	

	//however we are able to use the purchaseticket command in starports
	//without having to use a ticketvendor by just giving commandline parameters
	//when we are *near* a ticket vendor

	TravelTerminal* terminal = dynamic_cast<TravelTerminal*> (gWorldManager->getNearestTerminal(playerObject,TanType_TravelTerminal));
	// iterate through the results
	
	if((!terminal)||terminal->mPosition.inRange2D(playerObject->mPosition,purchaseRange))
	{
		gMessageLib->sendSystemMessage(playerObject,L"","travel","too_far");
		return;
	}

	playerObject->setTravelPoint(terminal);

	message->getStringUnicode16(dataStr);

	// Have to convert BEFORE using split, since the conversion done there is removed It will assert().. evil grin...
	// Either do the conversion HERE, or better fix the split so it handles unicoe also.
	dataStr.convert(BSTRType_ANSI);

	elements = dataStr.split(dataElements,' ');

	if(elements < 4)
	{
		gMessageLib->sendSystemMessage(playerObject,L"","travel","route_not_available");
		return;
	}

	// get price and planet ids
	TicketProperties ticketProperties;
	gTravelMapHandler->getTicketInformation(dataElements,&ticketProperties);

	if(!ticketProperties.dstPoint)
	{
		gMessageLib->sendSystemMessage(playerObject,L"","travel","route_not_available");
		return;
	}

	uint8 roundTrip = 0;

	if(elements > 4)
		roundTrip = atoi(dataElements[4].getAnsi());

	if(dataElements[4].getCrc() == BString("single").getCrc())
		roundTrip = 0;


	//how many tickets will it be?
	uint32 amount = 1;
	if(roundTrip)
		amount = 2;

	Inventory*	inventory	= dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	Bank*		bank		= dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

	if(!inventory->checkSlots(static_cast<uint8>(amount)))
	{
		gMessageLib->sendSystemMessage(playerObject,L"","error_message","inv_full");
		return;
	}

	if(roundTrip == 1)
	{
		ticketProperties.price *= 2;
	}

	// update bank or inventory credits
	if(!(inventory->updateCredits(-ticketProperties.price)))
	{
		if(!(bank->updateCredits(-ticketProperties.price)))
		{
			//gMessageLib->sendSystemMessage(entertainer,L"","travel","route_not_available");
			gUIManager->createNewMessageBox(NULL,"ticketPurchaseFailed","The Galactic Travel Commission","You do not have enough money to complete the ticket purchase.",playerObject);
			return;
		}
	}

	if(playerObject->isConnected())
	{
		gMessageLib->sendSystemMessage(playerObject,L"","base_player","prose_pay_acct_success","money/acct_n","travelsystem",L"",ticketProperties.price);

		gObjectFactory->requestNewTravelTicket(inventory,ticketProperties,inventory->getId(),99);

		if(roundTrip == 1)
		{
			uint32 tmpId = ticketProperties.srcPlanetId;
			TravelPoint* tmpPoint = ticketProperties.srcPoint;

			ticketProperties.srcPlanetId = ticketProperties.dstPlanetId;
			ticketProperties.srcPoint = ticketProperties.dstPoint;
			ticketProperties.dstPlanetId = static_cast<uint16>(tmpId);
			ticketProperties.dstPoint = tmpPoint;

			gObjectFactory->requestNewTravelTicket(inventory,ticketProperties,inventory->getId(),99);
		}

		gUIManager->createNewMessageBox(NULL,"handleSUI","The Galactic Travel Commission","@travel:ticket_purchase_complete",playerObject);
	}
}

//======================================================================================================================
//
// get attributes batch
//

void ObjectController::_handleGetAttributesBatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	string			requestStr;
	BStringVector	dataElements;
	BStringVector	dataElements2;
	uint16			elementCount;


	message->getStringUnicode16(requestStr);
	requestStr.convert(BSTRType_ANSI);
	requestStr.getRawData()[requestStr.getLength()] = 0;

	elementCount = requestStr.split(dataElements,' ');

	if(!elementCount)
	{
		gLogger->logMsg("ObjectController::_handleAttributesBatch: Error in requestStr");
		return;
	}

	Message* newMessage;

	for(uint16 i = 0;i < elementCount;i++)
	{

		uint64 itemId	= boost::lexical_cast<uint64>(dataElements[i].getAnsi());
		Object* object	= gWorldManager->getObjectById(itemId);

		//gLogger->logMsgF("ObjectController::_handleAttributesBatch: ID %I64u",MSG_HIGH,itemId);

		if(object == NULL)
		{
			// could be a resource
			Resource* resource = gResourceManager->getResourceById(itemId);

			if(resource != NULL)
			{
				resource->sendAttributes(playerObject);
				continue;
			}

			//could be a schematic!
			Datapad* datapad				= dynamic_cast<Datapad*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
			ManufacturingSchematic* schem	= datapad->getManufacturingSchematicById(itemId);

			if(schem != NULL)
			{
				schem->sendAttributes(playerObject);
				continue;
			}

			MissionObject* mission			= datapad->getMissionById(itemId);
			if(mission != NULL)
			{
				mission->sendAttributes(playerObject);
				continue;
			}

			IntangibleObject* data = datapad->getDataById(itemId);
			if(data != NULL)
			{
				data->sendAttributes(playerObject);
				continue;
			}

			//gLogger->logMsgF("ObjectController::_handleAttributesBatch: get info for %"PRIu64"",MSG_HIGH,itemId);

			// TODO: check our datapad items
			if(playerObject->isConnected())
			{
				// default reply for schematics
				gMessageFactory->StartMessage();
				gMessageFactory->addUint32(opAttributeListMessage);
				gMessageFactory->addUint64(itemId);
				gMessageFactory->addUint32(0);
				//gMessageFactory->addUint16(0);
				//gMessageFactory->addUint32(40);

				newMessage = gMessageFactory->EndMessage();

				(playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(),  CR_Client, 8);
			}

			//gLogger->logMsgF("ObjectController::_handleAttributesBatch: Object not found %"PRIu64"",MSG_HIGH,itemId);

			//finally, when we are crafting this could be the new item, not yet added to the worldmanager??
			if(playerObject->getCraftingSession())
			{
				if(playerObject->getCraftingSession()->getItem()&&playerObject->getCraftingSession()->getItem()->getId() == itemId)
				{
					gLogger->logMsgF("ObjectController::_handleAttributesBatch for crafted Item: ID %I64u",MSG_HIGH,itemId);
					playerObject->getCraftingSession()->getItem()->sendAttributes(playerObject);
				}
			}
		}
		else
		{
			// Tutorial: I (Eru) have to do some hacks here, since I don't know how to get the info of what object the client has selected (by single click) in the Inventory.
			if (gWorldConfig->isTutorial())
			{
				// Let's see if the actual object is the food item "Melon" in our inventory.
				if (dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getId() == object->getParentId())
				{
					// gLogger->logMsg("Selected an object in Inventory!!!");
					//uint64 id = object->getId();

					// Is it an Item?
					Item* item = dynamic_cast<Item*>(object);

					// Check if this item is a food item.
					if (item)
					{
						if (item->getItemFamily() == ItemFamily_Foods)
						{
							// gLogger->logMsg("It's a food item!!!!");
							playerObject->getTutorial()->tutorialResponse("foodSelected");
						}
					}
				}
			}

			object->sendAttributes(playerObject);
		}
	}
}

//=============================================================================
//
// quests
//

void ObjectController::_handleRequestQuestTimersAndCounters(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

}

//=============================================================================
//
// target
//

void ObjectController::_handleTarget(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject* playerObject = (PlayerObject*)mObject;
}
//======================================================================================================================
//
// end burst run
//

void ObjectController::_endBurstRun(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

}

//======================================================================================================================
//
// surrender skill
//

void ObjectController::_handleSurrenderSkill(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player		= dynamic_cast<PlayerObject*>(mObject);
	string			skillStr;

	message->getStringUnicode16(skillStr);
	skillStr.convert(BSTRType_ANSI);

	if(!(skillStr.getLength()))
	{
		gLogger->logMsg("ObjectController::handleSurrenderSkill: no skillname\n");
		return;
	}

	Skill* skill = gSkillManager->getSkillByName(skillStr.getAnsi());

	if(skill == NULL)
	{
		gLogger->logMsgF("ObjectController::handleSurrenderSkill: could not find skill %s",MSG_NORMAL,skillStr.getAnsi());
		return;
	}

	gSkillManager->dropSkill(skill->mId,player);
}

//======================================================================================================================
//
// qualified for skill
//

void ObjectController::_handleClientQualifiedForSkill(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//gLogger->hexDump(message->getData(),message->getSize());
}


//=============================================================================================================================
//
// radial request
//

void ObjectController::handleObjectMenuRequest(Message* message)
{
	//for ever item where we request a radial the client starts by displaying a radial on his own and additionally sends a
	//objectMenuRequest to the server
	//The server then either just resends the radial as send by the client or adds / modifies options on his own
	//this is why sometimes when lag is involved it takes some time for all options to display


	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	message->getUint32(); // unknown
	uint64 requestedObjectId = message->getUint64();
	message->getUint64(); // player id again ?

	Object* requestedObject = gWorldManager->getObjectById(requestedObjectId);

	uint32 itemCount = message->getUint32();

	string extendedDescription;
	MenuItemList menuItemList;

	MenuItem* menuItem;
	for(uint32 i = 0; i < itemCount;i++)
	{
		menuItem = new(MenuItem);

		menuItem->sItem			= message->getUint8();   // item nr
		menuItem->sSubMenu		= message->getUint8();   // submenu flag
		menuItem->sIdentifier	= message->getUint8();   // item identifier
		menuItem->sOption		= message->getUint8();   // extended option
		message->getStringUnicode16(extendedDescription);
		menuItemList.push_back(menuItem);
	}

	uint8 responseNr = message->getUint8();

	// gLogger->logMsgF("ObjController::handleObjectMenuRequest: Entered, responseNr = %u, itemCount = %u", MSG_NORMAL, responseNr, itemCount);

	if(!requestedObject)
	{
		if(playerObject->isConnected())
			gMessageLib->sendEmptyObjectMenuResponse(requestedObjectId,playerObject,responseNr,menuItemList);

		//the list is cleared and items are destroyed in the message lib
		//for the default response
		gLogger->logMsgF("ObjController::handleObjectMenuRequest: Couldn't find object %"PRIu64"",MSG_HIGH,requestedObjectId);
		return;
	}

	requestedObject->prepareCustomRadialMenu(playerObject,static_cast<uint8>(itemCount));

	if (requestedObject->getRadialMenu())
	{
		if(playerObject->isConnected())
		{
			gMessageLib->sendObjectMenuResponse(requestedObject,playerObject,responseNr);
		}
	}
	else
	{
		// putting this for static objects/objects that are not known by the server yet
		// send a default menu,so client stops flooding us with requests

		//empty might just mean that the clients radial is sufficient
		//gLogger->logMsgF("ObjController::handleObjectMenuRequest: Couldn't find object Radial %"PRIu64"",MSG_HIGH,requestedObjectId);

		if(playerObject->isConnected())
		 	gMessageLib->sendEmptyObjectMenuResponse(requestedObjectId,playerObject,responseNr,menuItemList);

		//the list is cleared and items are destroyes in the message lib
		//for the default response
	}

	//are the menu options pointers removed / deleted by boost ?
	//delete(requestedObject->getRadialMenu());
}

//=============================================================================================================================

void ObjectController::handleObjectReady(Object* object,DispatchClient* client)
{
	PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	/*
	if (!playerObject)
	{
		gLogger->logMsg("playerObject == NULL");
	}

	if (!player)
	{
		gLogger->logMsg("player == NULL");
	}

	if (playerObject != player)
	{
		gLogger->logMsg("playerObject != player");
	}
	*/

	// gLogger->logMsgF("ObjectController::handleObjectReady: targetId = %"PRIu64" for %s",MSG_NORMAL,object->getId(), player->getFirstName().getAnsi());


	// Get the container object.
	Container* container = dynamic_cast<Container*>(object);
	if (container)
	{
		// uint32 counter = container->getObjectLoadCounter();
		// gLogger->logMsgF("We have %d objects in the container", MSG_NORMAL, counter);

		ObjectList*	objList = container->getObjects();
		ObjectList::iterator containerObjectIt = objList->begin();

		while(containerObjectIt != objList->end())
		{
			Object* object = (*containerObjectIt);

			if (TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object))
			{
				// reminder: objects are owned by the global map, containers only keeps references
				// send the creates, if we are not owned by any player OR by exactly this player.
				if (playerObject)
				{
					if (!object->getPrivateOwner() || (object->isOwnedBy(playerObject)))
					{
						// could be a resource container, need to check this first, since it inherits from tangible
						if (ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(object))
						{
							gMessageLib->sendCreateResourceContainer(resCont,playerObject);
						}
						// or a tangible
						else
						{
							gMessageLib->sendCreateTangible(tangibleObject,playerObject);
						}
					}
				}
			}
			++containerObjectIt;
		}
		gMessageLib->sendOpenedContainer(object->getId(), player);
	}

}

//======================================================================================================================

void ObjectController::_handleNewbieSelectStartingLocation(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
	// gLogger->hexDump(message->getData(),message->getSize());

	// Find the planet and position.
	if (gWorldConfig->isTutorial())
	{
		string name;
		message->getStringUnicode16(name);

		if (!(name.getLength()))
		{
			return;
		}
		name.convert(BSTRType_ANSI);
		player->getTutorial()->warpToStartingLocation(name);
	}
}




//======================================================================================================================
//
//handles the logout via /logout command
//
void ObjectController::_handleClientLogout(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
	// gLogger->hexDump(message->getData(),message->getSize());
	
	player->togglePlayerCustomFlagOn(PlayerCustomFlag_LogOut);	

	//// we need to kneel
	//player->setPosture(CreaturePosture_Crouched);
	//player->getHam()->updateRegenRates();
	//player->toggleStateOff(CreatureState_SittingOnChair);
	//player->updateMovementProperties();

	//gMessageLib->sendUpdateMovementProperties(player);
	//gMessageLib->sendPostureAndStateUpdate(player);
	//gMessageLib->sendSelfPostureUpdate(player);

	uint32 logout		= gWorldConfig->getConfiguration("Player_LogOut_Time",(uint32)30);
	uint32 logoutSpacer = gWorldConfig->getConfiguration("Player_LogOut_Spacer",(uint32)5);

	if(logoutSpacer > logout)
		logoutSpacer = logout;
	
	if(logoutSpacer < 1)
		logoutSpacer = 1;

	if(logout < logoutSpacer)
		logout = logoutSpacer;

	if(logout > 300)
		logout = 300;

	// schedule execution
	addEvent(new LogOutEvent(Anh_Utils::Clock::getSingleton()->getLocalTime()+((logout-logoutSpacer)*1000),logoutSpacer*1000),logoutSpacer*1000);
	gMessageLib->sendSystemMessage(player,L"","logout","time_left","","",L"",logout);

}


//======================================================================================================================
//
// start burst run
//

void ObjectController::_BurstRun(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	//can we burstrun right now ??
	if(player->checkPlayerCustomFlag(PlayerCustomFlag_BurstRun))
	{
		gMessageLib->sendSystemMessage(player,L"You are already running as hard as you can.");
		return;
	}

	if(player->checkPlayerCustomFlag(PlayerCustomFlag_BurstRunCD))
	{
		gMessageLib->sendSystemMessage(player,L"","combat_effects","burst_run_tired");
		return;
	}

	uint32 actioncost = gWorldConfig->getConfiguration("Player_BurstRun_Action",(uint32)300);
	uint32 healthcost = gWorldConfig->getConfiguration("Player_BurstRun_Health",(uint32)300);
	uint32 mindcost	  = gWorldConfig->getConfiguration("Player_BurstRun_Mind",(uint32)0);

	if(!player->getHam()->checkMainPools(healthcost,actioncost,mindcost))
	{
		gMessageLib->sendSystemMessage(player,L"","combat_effects","burst_run_no");
		return;
	}

	player->getHam()->updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints,-(int32)actioncost,true);
	player->getHam()->updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints,-(int32)healthcost,true);
	player->getHam()->updatePropertyValue(HamBar_Mind,HamProperty_CurrentHitpoints,-(int32)mindcost,true);

	player->setCurrentSpeedModifier(player->getCurrentSpeedModifier()*2);
	gMessageLib->sendUpdateMovementProperties(player);

	uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();

	uint32 br_length		= gWorldConfig->getConfiguration("Player_BurstRun_Time",(uint32)60);
	uint32 br_coolD			= gWorldConfig->getConfiguration("Player_BurstRun_CoolDown",(uint32)600);

	uint32 t = std::min<uint32>(br_length,  br_coolD);

	player->togglePlayerCustomFlagOn(PlayerCustomFlag_BurstRunCD);	
	player->togglePlayerCustomFlagOn(PlayerCustomFlag_BurstRun);	

	// schedule execution
	addEvent(new BurstRunEvent(now+(br_length*1000),now+(br_coolD*1000)),t*1000);
	
	//this sys message is messed up!!!
	//gMessageLib->sendSystemMessage(player,L"","cbt_spam","burstrun_start");
	gMessageLib->sendSystemMessage(player,L"You run as hard as you can!");
	int8 s[256];
	sprintf(s,"%s %s puts on a sudden burst of speed.",player->getFirstName().getAnsi(),player->getLastName().getAnsi());
	BString bs(s);
	bs.convert(BSTRType_Unicode16);
	gMessageLib->sendSystemMessageInRange(player,false,bs.getUnicode16());

}
