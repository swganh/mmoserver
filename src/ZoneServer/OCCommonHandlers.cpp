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
#include "BuildingObject.h"
#include "TravelMapHandler.h"
#include "TravelTerminal.h"
#include "TreasuryManager.h"
#include "Tutorial.h"
#include "UIManager.h"
//#include "Wearable.h"
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

#include <cassert>

//=============================================================================

void ObjectController::_handleBoardTransport(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);

	ObjectSet		inRangeObjects;
	float			boardingRange	= 25.0;

	if(playerObject->getPosture() == CreaturePosture_SkillAnimating)
	{
		gMessageLib->sendSystemMessage(playerObject,L"", "error_message", "wrong_state");
		return;
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

	mSI->getObjectsInRange(playerObject,&inRangeObjects,ObjType_Creature | ObjType_NPC, boardingRange);

	// iterate through the results
	ObjectSet::iterator it = inRangeObjects.begin();

	while(it != inRangeObjects.end())
	{
		if(Shuttle* shuttle = dynamic_cast<Shuttle*>(*it))
		{
			// in range check
			if(playerObject->getParentId() !=  shuttle->getParentId())
			{
				gMessageLib->sendSystemMessage(playerObject,L"","travel","boarding_too_far");
				return;
			}

			if (!shuttle->availableInPort())
			{
				gMessageLib->sendSystemMessage(playerObject,L"","travel","shuttle_not_available");
				return;
			}

			shuttle->useShuttle(playerObject);

			return;
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

	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	Object*			itemObject		= gWorldManager->getObjectById(targetId);

	if (itemObject)
	{
		if(glm::distance(playerObject->getWorldPosition(), itemObject->getWorldPosition()) > 10)
		{
			gMessageLib->sendSystemMessage(playerObject, L"", "system_msg", "out_of_range");
			return;
		}

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
				// Request container contents.
				gContainerFactory->requestObject(this,targetId,TanGroup_Container,0,playerObject->getClient());
				aContainer = true;
			}

			//this might be a backpack
			//or a chest - it needs to have a capacity to be a container!
			if (tangObj->getCapacity())
			{
				//checkContainingContainer checks the permission
				if(checkContainingContainer(tangObj->getId(),playerObject->getId()))
				{
					aContainer = true;
					
					//create containers content for us	
					tangObj->createContent(playerObject);
				}
			}
		}
		//its not a Container* Object however in theory it still can be a backpack for example
		if (!aContainer)
		{
			// STF: container_error_message Key: container8 does not seem to be working, using this custom string temperary.
			gMessageLib->sendSystemMessage(playerObject, L"You do not have permission to access that container.");
		}
		else
		{
            gMessageLib->sendOpenedContainer(targetId, playerObject);
		}
	}
	else
	{
		gLogger->log(LogManager::NOTICE,"ObjectController::_handleOpenContainer: INVALID Object id %"PRIu64"",targetId);
	}
}

void ObjectController::_handleCloseContainer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	//Object*			itemObject		= gWorldManager->getObjectById(targetId);

	if (gWorldConfig->isTutorial())
	{
		playerObject->getTutorial()->containerClose(targetId);
	}

	// gMessageLib->sendOpenedContainer(targetId, playerObject);
}


//=============================================================================
//
// transfer item
//

void ObjectController::_handleTransferItem(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{


	CellObject*		cell;

	PlayerObject*	playerObject	=	dynamic_cast<PlayerObject*>(mObject);
	Object*			itemObject		=	gWorldManager->getObjectById(targetId);
	Inventory*		inventory		=	dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	string			dataStr;
	uint64			targetContainerId;
	uint32			linkType;
	float			x,y,z;


	gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItem: called item %I64u",itemObject->getId());

	message->getStringUnicode16(dataStr);

	if(swscanf(dataStr.getUnicode16(),L" %"WidePRIu64 L" %u %f %f %f",&targetContainerId,&linkType,&x,&y,&z) != 5)
	{
		gLogger->log(LogManager::DEBUG,"ObjController::handleTransferItem: Error in parameters");
		return;
	}




	if (!itemObject)
	{
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc: No Object to transfer :(");
		//no Object :(
		return;
	}

	TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);
	if(!tangible)
	{
		//no tagible - get out of here
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc: No tangible to transfer :(");
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
    itemObject->mPosition = glm::vec3(x,y,z);

	if (!targetContainerId)
	{
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc:TargetContainer is 0 :(");
		//return;

	}
	
	//ok how to tackle this ... :
	//basically I want to use ObjectContainer as standard access point for item handling!


	//lets begin by getting the target Object
	//when the target Object approves the transfer we may proceed

	if(!checkTargetContainer(targetContainerId,itemObject))
	{
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc:TargetContainer is not valid :(");
		return;
	}

	gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc:TargetContainer has approved :)");
	
	// get ourselves the target container 
	// please note THIS IS ONLY SUCCESFUL FOR TANGIBLE OBJECT BASED CONTAINERS -> no cells
	TangibleObject* parentContainer = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(tangible->getParentId()));
	if(!parentContainer)
		parentContainer = inventory;

	if(!checkContainingContainer(tangible->getParentId(), playerObject->getId()))
	{
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc:ContainingContainer is not allowing the transfer :(");
		return;

	}
	
	// Remove the object from whatever contains it.
	if(!removeFromContainer(targetContainerId, targetId))
	{
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: remove item %I64u FromContainer %I64u failed :( this ",itemObject->getId(),targetContainerId);
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: This rightly happens when looting corpses as the item gets then regularly created");
		//this we might need to revise somehow we do not always want to loot standard db items, do we ?

		return;
	}
					
	//now go and move it to wherever it belongs
	cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(targetContainerId));
	if (cell)
	{
		// drop in a cell
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: Drop into cell %"PRIu64"", targetContainerId);

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

		gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: Cell added item to cell %I64u ",cell->getId());
		
		//do the db update manually because of the position - unless we get an automated position save in
		itemObject->setParentId(targetContainerId,linkType,playerObject,false); 
		itemObject->updateWorldPosition();
		
		/*ResourceContainer* rc = dynamic_cast<ResourceContainer*>(itemObject);

		if(rc)
			mDatabase->ExecuteSqlAsync(0,0,"UPDATE resource_containers SET parent_id ='%I64u', oY='%f', oZ='%f', oW='%f', x='%f', y='%f', z='%f' WHERE id='%I64u'",itemObject->getParentId(), itemObject->mDirection.y, itemObject->mDirection.z, itemObject->mDirection.w, itemObject->mPosition.x, itemObject->mPosition.y, itemObject->mPosition.z, itemObject->getId());
		else
			mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id ='%I64u', oY='%f', oZ='%f', oW='%f', x='%f', y='%f', z='%f' WHERE id='%I64u'",itemObject->getParentId(), itemObject->mDirection.y, itemObject->mDirection.z, itemObject->mDirection.w, itemObject->mPosition.x, itemObject->mPosition.y, itemObject->mPosition.z, itemObject->getId());
		  */
		
		cell->addObjectSecure(itemObject,playerObject->getKnownPlayers());
		
		//do this manually - we need to destroy the object and create it freshly for it to display properly 
		//to the owner
		playerObject->addKnownObjectSafe(itemObject);
		itemObject->addKnownObjectSafe(playerObject);
		
		gMessageLib->sendDestroyObject(itemObject->getId(),playerObject);
		gMessageLib->sendCreateObject(itemObject,playerObject);
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: Player : %I64u contained in %I64u",playerObject->getId(),playerObject->getParentId());
		
	}	


	
	if (inventory && (inventory->getId() == targetContainerId))	// Valid player inventory.
	{
		// Add object to OUR inventory.
		tangible->destroyKnownObjects();
		gMessageLib->sendCreateObject(itemObject,playerObject);

		itemObject->setParentId(targetContainerId,linkType,playerObject,true);
		inventory->addObjectSecure(itemObject);
		
		return;
		
	}
	
	
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetContainerId));
	if(player)
	{
		//equip / unequip handles the db side, too
		if(!player->getEquipManager()->EquipItem(item))
		{
			//readd it to the old parent
			if(parentContainer)
				parentContainer->addObjectSecure(item);
		}
		return;
	}

	//thats the tutorial container - leave them as separate class ?
	Container* container = dynamic_cast<Container*>(gWorldManager->getObjectById(targetContainerId));
	if (container && tangible)
	{
		
		// Add it to the container.
		// We dont have any access validations yet.
		container->addObjectSecure(itemObject);	  //just add its already created
		
		//set the new parent, send the contaiment and update the db
		itemObject->setParentId(targetContainerId,linkType,playerObject,true);
		return;
	}

	//some other container ... hopper backpack chest etc
	TangibleObject* receivingContainer = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(targetContainerId));
	if(receivingContainer)
	{
		receivingContainer->addObjectSecure(itemObject);
		itemObject->setParentId(receivingContainer->getId(),linkType,playerObject,true);
	}
	
	
}

//=============================================================================
//
// checks whether we have access to the container containing the item
//

bool ObjectController::checkContainingContainer(uint64 containingContainer, uint64 playerId)
{
	ObjectContainer* container = dynamic_cast<ObjectContainer*>(gWorldManager->getObjectById(containingContainer));
	
	if(!container)
	{
		//it might be our inventory or the inventory of a creature were looting
		//PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
		if(containingContainer == (playerId+1))
		{
			//its our inventory ... - return true
			return true;
		}

		if(containingContainer == playerId)
		{
			//its us
			return true;
		}

		return false;

	}

	uint64 ownerId = container->getObjectMainParent(container);

	Object* object = dynamic_cast<Object*>(gWorldManager->getObjectById(ownerId));

	//it might be the inventory
	if(!object)
	{
		//Hack ourselves an inventory .... - its not part of the world ObjectMap
		if((ownerId-1) == playerId)
		{
			object = gWorldManager->getObjectById(playerId);
		}
	}

	if(BuildingObject* building = dynamic_cast<BuildingObject*>(object))
	{
		if(building->hasAdminRights(playerId) || gWorldConfig->isTutorial())
		{
			return true;
		}
		return false;
	}

	if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(ownerId)))
	{
		if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId())))
		{
			if(building->hasAdminRights(playerId))
			{
				//now test whether we are in the same building
				PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
				if(CellObject* playercell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId())))
				{
					if(BuildingObject* playerparent = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(playercell->getParentId())))
					{
						//still get in a range check ???
						return true;
					}
				}
			}
		}
		return false;
	}

	if(PlayerObject* player = dynamic_cast<PlayerObject*>(object))
	{
		if(player->getId() == playerId)
		{
			return true;
		}
		else
			return false;
	}

	//todo handle factory hoppers

	//todo handle loot permissions
	if(CreatureObject* creature = dynamic_cast<CreatureObject*>(object))
	{
	}

	
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
	
	TangibleObject* tangibleItem = dynamic_cast<TangibleObject*>(object);
	
	//if its a backpack etc we want to know how many items are in it!
	uint32 objectSize = tangibleItem->getHeadCount();

	//********************
	//this is a special case as we are equipping the item
	//so handle it separately 
	if(playerObject->getId() == targetContainerId)
	{
		//check for equip restrictions!!!!
		//we cant drop that check - further down we assume that the transfer is accepted
		// a failing equip will just loose us our item in the receiving container and crash the server in the end
		return playerObject->getEquipManager()->CheckEquipable(object);		
	}
	
	//*****************************
	//ok everything else is a tangible Object
	ObjectContainer* targetContainer = dynamic_cast<ObjectContainer*>(gWorldManager->getObjectById(targetContainerId));
	
	//sanity check - 
	if(!targetContainer)
	{
		//inventory is NOT part of the main ObjectMap - everything else should be in there
		if(inventory && (inventory->getId() != targetContainerId))
		{
			return false;
		}
		if(inventory)
			targetContainer = dynamic_cast<TangibleObject*>(inventory);
		else
		{
			gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc: TargetContainer is NULL and not an inventory :(");
			return false;
		}
		
	}

	//====================================================================00
	//check access permissions first

	bool access = false;
	bool fit	 = false;

	//********************
	//The tutorial Container is a special case
	//so handle it separately
	Container* container = dynamic_cast<Container*>(targetContainer );
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

	//********************
	//Factory Outputhopper is retrieve only
	//access has been granted through the UI already
	TangibleObject* tangibleContainer = dynamic_cast<TangibleObject*>(targetContainer);
	if((tangibleContainer)&&(strcmp(tangibleContainer->getName().getAnsi(),"ingredient_hopper")==0))
	{
		//do we have access rights to the factories hopper?? this would have to be checked asynchronously
		//for now we can only access the hoppers UI through the client and checking our permission so its proven already
		//a hacker might in theory exploit this, though factories items should only be in memory when someone accesses the hopper

		access = true;
	}
	
	//====================================================================================
	//get the mainOwner of the container - thats a building or a player or an inventory
	//
	
	uint64 ownerId = container->getObjectMainParent(targetContainer);
	
	Object* objectOwner = dynamic_cast<Object*>(gWorldManager->getObjectById(ownerId));

	if(BuildingObject* building = dynamic_cast<BuildingObject*>(objectOwner))
	{
		if(building->hasAdminRights(playerObject->getId()))
		{
			access = true;
			//do we have enough room ?
			if(building->checkCapacity(objectSize))
			{
				//*****************************
				//if it is the House wé dont need to check a containers capacity further down
				if(!tangibleContainer)   //mainly as the container might not exist if its placed in the house directly
					return true;

				fit = true;
			}
			else
			{
				//This container is full. 
				gMessageLib->sendSystemMessage(playerObject,L"","container_error_message","container03");
				return false;
			}
			
		}
		else
		{
			//You do not have permission to access that container. 
			gMessageLib->sendSystemMessage(playerObject,L"","container_error_message","container08");
			return false;
		}

		
	}

	//**********************************
	//the inventory is *NOT* part of the worldmanagers ObjectMap  
	//this is our inventory - we are allowed to put stuff in there - but is there still enough place ?
	if(inventory&& (inventory->getId() == ownerId))
	{
		//make sure its our inventory!!!!!!
		access = ((inventory->getId()-1) == playerObject->getId());
		if(!access)
		{
			//You do not have permission to access that container. 
			gMessageLib->sendSystemMessage(playerObject,L"","container_error_message","container08");
			return false;
		}
		
		//check space in inventory
		fit = inventory->checkCapacity(1,playerObject,true);
		if(!fit)
		{
			//This container is full. 
			gMessageLib->sendSystemMessage(playerObject,L"","container_error_message","container03");
			return false;
		}
	}	
	
	//if this is a tangible container (backpack, satchel) we want to make sure,
	//that we do not put another backpack in it.
	//in other words, the contained container MUST be smaller than the containing container

	//**********************
	//check capacity - return false if full
	//we wont get here if its an inventory
	if(tangibleContainer && (!tangibleContainer->checkCapacity(objectSize,playerObject))) //automatically sends errormsg to player
	{
		return false;
	}

	uint32 containingContainersize =  tangibleContainer->getCapacity();
	uint32 containedContainersize =  tangibleItem->getCapacity();

	//we can only add smaller containers inside other containers
	if(containedContainersize >= containingContainersize)
	{
		//This item is too bulky to fit inside this container.
		gMessageLib->sendSystemMessage(playerObject,L"","container_error_message","container12");
		return false;
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
		// unequip it
		return playerObject->getEquipManager()->unEquipItem(itemObject);
		
	}
	
	
	//its our inventory
	if (tangible->getParentId() == inventory->getId())
	{
		//gMessageLib->sendDestroyObject(targetId,playerObject);
		
		if(!inventory->removeObject(itemObject))
		{
			gLogger->log(LogManager::DEBUG,"ObjectController::removeFromContainer: Internal Error could not remove  %"PRIu64" from %I64u", itemObject->getId(),inventory->getId());
			return false;
		}
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
		gLogger->log(LogManager::DEBUG,"Transfer item from creature inventory to player inventory (looting)");
		// gMessageLib->sendContainmentMessage(targetId,itemObject->getParentId(),-1,playerObject);

		gMessageLib->sendDestroyObject(targetId,playerObject);
		
		if(!creatureInventory->removeObject(itemObject))
		{
			gLogger->log(LogManager::DEBUG,"ObjectController::removeFromContainer: Internal Error could not remove  %"PRIu64" from creature inventory %I64u", itemObject->getId(),creatureInventory->getId());
			return false;
		}

		ObjectIDList* invObjList = creatureInventory->getObjects();
		if (invObjList->size() == 0)
		{
			// Put this creature in the pool of delayed destruction and remove the corpse from scene.
			gWorldManager->addCreatureObjectForTimedDeletion(creatureInventory->getParentId(), LootedCorpseTimeout);
		}
		
		if (gWorldConfig->isTutorial())
		{
			// TODO: Update tutorial about the loot.
			 //playerObject->getTutorial()->transferedItemFromContainer(targetId, sourceId);
		}
		// This ensure that we do not use/store any of the temp id's in the database.
        gObjectFactory->requestNewDefaultItem(inventory, item->getItemFamily(), item->getItemType(), inventory->getId(), 99, glm::vec3(), "");
		return false;

	}		   


	CellObject* cell;
	if(cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(itemObject->getParentId())))
	{
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: pick up from cell %"PRIu64"", itemObject->getParentId());
		
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
				}
			}
		}
		
		//we *cannot* remove static tangibles like the structureterminal!!!!
		if(tangible->getStatic())
		{
			return false;
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
			assert(false && "ObjectController::removeFromContainer unable to remove itemObject");
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

	gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc: Entered");

	//gMessageLib->sendSystemMessage(playerObject,L"","error_message","insufficient_permissions");

	message->getStringUnicode16(dataStr);

	if(swscanf(dataStr.getUnicode16(),L"%"WidePRIu64 L" %u %f %f %f",&targetContainerId,&linkType,&x,&y,&z) != 5)
	{
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc: Error in parameters");
		return;
	}

	if (!itemObject)
	{
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc: No Object to transfer :(");
		return;
	}

	TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);
	if(!tangible)
	{
		//no tagible - get out of here
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc: No tangible to transfer :(");
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
    itemObject->mPosition = glm::vec3(x,y,z);

	if (!targetContainerId)
	{
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc:TargetContainer is 0 :(");
		//return;

	}
	
	//ok how to tackle this ... :
	//basically I want to use ObjectContainer as standard access point for item handling!
	//so far we have different accesses for Objects on the player and for the inventory	and for ContainerObjects and for cells ...

	//lets begin by getting the target Object

	if(!checkTargetContainer(targetContainerId,itemObject))
	{
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc:TargetContainer is not valid :(");
		return;
	}

	TangibleObject* parentContainer = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(tangible->getParentId()));
	if(!parentContainer)
		parentContainer = inventory;

	if(!checkContainingContainer(tangible->getParentId(), playerObject->getId()))
	{
		gLogger->log(LogManager::DEBUG,"ObjController::_handleTransferItemMisc:ContainingContainer is not allowing the transfer :(");
		return;

	}
	
	// Remove the object from whatever contains it.
	if(!removeFromContainer(targetContainerId, targetId))
	{
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: removeFromContainer failed :( this might be caused by looting a corpse though");
		return;
	}

	
	//we need to destroy the old radial ... our item now gets a new one
	//delete(itemObject->getRadialMenu());
	itemObject->ResetRadialMenu();


	//now go and move it to wherever it belongs
	cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(targetContainerId));
	if (cell)
	{
		// drop in a cell
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: Drop into cell %"PRIu64"",  targetContainerId);

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

		gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: Cell added item to cell %I64u ", cell->getId());
		
		//do the db update manually because of the position - unless we get an automated position save in
		itemObject->setParentId(targetContainerId,linkType,playerObject,false); 
		
		ResourceContainer* rc = dynamic_cast<ResourceContainer*>(itemObject);
		if(rc)
			mDatabase->ExecuteSqlAsync(0,0,"UPDATE resource_containers SET parent_id ='%I64u', oX='%f', oY='%f', oZ='%f', oW='%f', x='%f', y='%f', z='%f' WHERE id='%I64u'",itemObject->getParentId(), itemObject->mDirection.x, itemObject->mDirection.y, itemObject->mDirection.z, itemObject->mDirection.w, itemObject->mPosition.x, itemObject->mPosition.y, itemObject->mPosition.z, itemObject->getId());
		else
			mDatabase->ExecuteSqlAsync(0,0,"UPDATE items SET parent_id ='%I64u', oX='%f', oY='%f', oZ='%f', oW='%f', x='%f', y='%f', z='%f' WHERE id='%I64u'",itemObject->getParentId(), itemObject->mDirection.x, itemObject->mDirection.y, itemObject->mDirection.z, itemObject->mDirection.w, itemObject->mPosition.x, itemObject->mPosition.y, itemObject->mPosition.z, itemObject->getId());

		//take wm function at one point
		cell->addObjectSecure(itemObject,playerObject->getKnownPlayers());
		playerObject->addKnownObjectSafe(itemObject);
		itemObject->addKnownObjectSafe(playerObject);
		
		gMessageLib->sendDestroyObject(itemObject->getId(),playerObject);
		gMessageLib->sendCreateObject(itemObject,playerObject);
		
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: Player : %I64u contained in %I64u", playerObject->getId(),playerObject->getParentId());
		
	}	


	
	if (inventory && (inventory->getId() == targetContainerId))	// Valid player inventory.
	{
		// Add object to OUR inventory.

		itemObject->destroyKnownObjects();
		gMessageLib->sendCreateObject(itemObject,playerObject);
		
		itemObject->setParentId(targetContainerId,linkType,playerObject,true);
		inventory->addObjectSecure(itemObject);
		
		return;
		
	}
	
	
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetContainerId));
	if(player)
	{
		//equip / unequip handles the db side, too
		if(!player->getEquipManager()->EquipItem(item))
		{
			gLogger->log(LogManager::DEBUG,"ObjectController::_handleTransferItemMisc: Error equipping  %"PRIu64"",  item->getId());
			//panik!!!!!!
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
		receivingContainer->addObjectSecure(itemObject);
		itemObject->setParentId(receivingContainer->getId(),linkType,playerObject,true);
	}	
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

	
	float		purchaseRange = gWorldConfig->getConfiguration<float>("Player_TicketTerminalAccess_Distance",(float)10.0);

	if(playerObject->getPosture() == CreaturePosture_SkillAnimating)
	{
		gMessageLib->sendSystemMessage(playerObject,L"", "error_message", "wrong_state");
		return;
	}
	

	//however we are able to use the purchaseticket command in starports
	//without having to use a ticketvendor by just giving commandline parameters
	//when we are *near* a ticket vendor

	TravelTerminal* terminal = dynamic_cast<TravelTerminal*> (gWorldManager->getNearestTerminal(playerObject,TanType_TravelTerminal));
	// iterate through the results
	
    if((!terminal)|| (glm::distance(terminal->mPosition, playerObject->mPosition) > purchaseRange))
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
		gLogger->log(LogManager::DEBUG,"ObjectController::_handleAttributesBatch: Error in requestStr");
		return;
	}

	Message* newMessage;

	for(uint16 i = 0;i < elementCount;i++)
	{

		uint64 itemId	= boost::lexical_cast<uint64>(dataElements[i].getAnsi());
		Object* object	= gWorldManager->getObjectById(itemId);

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
			Datapad* datapad			= playerObject->getDataPad();
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

			//finally, when we are crafting this could be the new item, not yet added to the worldmanager??
			if(playerObject->getCraftingSession())
			{
				if(playerObject->getCraftingSession()->getItem()&&playerObject->getCraftingSession()->getItem()->getId() == itemId)
				{
					gLogger->log(LogManager::DEBUG,"ObjectController::_handleAttributesBatch for crafted Item: ID %I64u",itemId);
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
					//uint64 id = object->getId();

					// Is it an Item?
					Item* item = dynamic_cast<Item*>(object);

					// Check if this item is a food item.
					if (item)
					{
						if (item->getItemFamily() == ItemFamily_Foods)
						{
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
		gLogger->log(LogManager::DEBUG,"ObjectController::handleSurrenderSkill: no skillname\n");
		return;
	}

	Skill* skill = gSkillManager->getSkillByName(skillStr.getAnsi());

	if(skill == NULL)
	{
		gLogger->log(LogManager::DEBUG,"ObjectController::handleSurrenderSkill: could not find skill %s",skillStr.getAnsi());
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

	if(!requestedObject)
	{
		if(playerObject->isConnected())
			gMessageLib->sendEmptyObjectMenuResponse(requestedObjectId,playerObject,responseNr,menuItemList);

		//the list is cleared and items are destroyed in the message lib
		//for the default response
		gLogger->log(LogManager::DEBUG,"ObjController::handleObjectMenuRequest: Couldn't find object %"PRIu64"",requestedObjectId);
		return;
	}

	requestedObject->setMenuList(&menuItemList);



	//are we an item dropped in a structure awaiting to be moved or picked u`p?
    //just implement this virtual function for items as we need just one central point instead
	//of the same code over and over for all items

	CellObject* itemCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(requestedObject->getParentId()));

	Item* item = dynamic_cast<Item*>(requestedObject);
	ResourceContainer* rC = dynamic_cast<ResourceContainer*>(requestedObject);
	TangibleObject* tO = dynamic_cast<TangibleObject*>(requestedObject);

	//only display that menu when *we* and the item are in the same structure
	if((rC || item) && itemCell && (!tO->getStatic()))
	{
		CellObject* playerCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(playerObject->getParentId()));
		if(playerCell && (playerCell->getParentId() == itemCell->getParentId()))
		{
			PlayerStructure* pS = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(playerCell->getParentId()));
			if(pS)
				requestedObject->prepareCustomRadialMenuInCell(playerObject,static_cast<uint8>(itemCount));
		}
	}
	/*
	if(rc && requestedObject->getParentId())
	{
		if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(requestedObject->getParentId())))
		{
			requestedObject->prepareCustomRadialMenuInCell(playerObject,static_cast<uint8>(itemCount));
		}
	}
	*/
	//delete the radials after every use or provide every object with set rules when to delete it ?

	if(!requestedObject->getRadialMenu())
		requestedObject->prepareCustomRadialMenu(playerObject,static_cast<uint8>(itemCount));

	if (requestedObject->getRadialMenu())
	{
		if(playerObject->isConnected())
		{
			gMessageLib->sendObjectMenuResponse(requestedObject,playerObject,responseNr);
		}
		//they only reset if the objects virtual function does it
		//by default it stays
		requestedObject->ResetRadialMenu();
		//the radial menu is supposed to be an intelligent pointer deleting itself when no reference is left
		//however during runtime the item always references the radialmenu that was generated for it on the first call.
		//when the circumstances of the item change we need to delete the pointer and thus force it to generate a new radial
	}
	else
	{
		// putting this for static objects/objects that are not known by the server yet
		// send a default menu,so client stops flooding us with requests

		//empty might just mean that the clients radial is sufficient

		if(playerObject->isConnected())
		 	gMessageLib->sendEmptyObjectMenuResponse(requestedObjectId,playerObject,responseNr,menuItemList);

		//the list is cleared and items are destroyes in the message lib
		//for the default response
	}


	//we need to clear that if the messagelib wont clear it
	//still want to use it for the player radials at some point
	for(MenuItemList::iterator it=menuItemList.begin(); it != menuItemList.end();it++)
			delete (*it);

	menuItemList.clear();
	
}

//=============================================================================================================================

void ObjectController::handleObjectReady(Object* object,DispatchClient* client)
{
	PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	// Get the container object.
	Container* container = dynamic_cast<Container*>(object);
	if (container)
	{
		// uint32 counter = container->getObjectLoadCounter();

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

	uint32 logout		= gWorldConfig->getConfiguration<uint32>("Player_LogOut_Time",(uint32)30);
	uint32 logoutSpacer = gWorldConfig->getConfiguration<uint32>("Player_LogOut_Spacer",(uint32)5);

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
		gMessageLib->sendSystemMessage(player,L"","combat_effects","burst_run_wait");
		return;
	}

	uint32 actioncost = gWorldConfig->getConfiguration<uint32>("Player_BurstRun_Action",(uint32)300);
	uint32 healthcost = gWorldConfig->getConfiguration<uint32>("Player_BurstRun_Health",(uint32)300);
	uint32 mindcost	  = gWorldConfig->getConfiguration<uint32>("Player_BurstRun_Mind",(uint32)0);

	if(!player->getHam()->checkMainPools(healthcost,actioncost,mindcost))
	{
		gMessageLib->sendSystemMessage(player,L"You cannot burst run right now."); // the stf doesn't work!
		return;
	}

	player->getHam()->updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints,-(int32)actioncost,true);
	player->getHam()->updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints,-(int32)healthcost,true);
	player->getHam()->updatePropertyValue(HamBar_Mind,HamProperty_CurrentHitpoints,-(int32)mindcost,true);

	player->setCurrentSpeedModifier(player->getCurrentSpeedModifier()*2);
	gMessageLib->sendUpdateMovementProperties(player);

	uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();

	uint32 br_length		= gWorldConfig->getConfiguration<uint32>("Player_BurstRun_Time",(uint32)60);
	uint32 br_coolD			= gWorldConfig->getConfiguration<uint32>("Player_BurstRun_CoolDown",(uint32)600);

	uint32 t = std::min<uint32>(br_length,  br_coolD);

	player->togglePlayerCustomFlagOn(PlayerCustomFlag_BurstRunCD);	
	player->togglePlayerCustomFlagOn(PlayerCustomFlag_BurstRun);	

	// schedule execution
	addEvent(new BurstRunEvent(now+(br_length*1000),now+(br_coolD*1000)),t*1000);
	
	//Send the burst run system message to the player
	gMessageLib->sendSystemMessage(player,L"You run as hard as you can!");
	
	//Now send the burst run combat spam message to InRange
	int8 s[256];
	sprintf(s,"%s %s puts on a sudden burst of speed.",player->getFirstName().getAnsi(),player->getLastName().getAnsi());
	BString bs(s);
	bs.convert(BSTRType_Unicode16);
	gMessageLib->sendCombatSpam(player,player,0,"","",0,0,bs.getUnicode16());

}
