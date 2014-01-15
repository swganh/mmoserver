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

#include <cassert>

#include "ZoneServer/GameSystemManagers/Structure Manager/BuildingObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/CellObject.h"
#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/FactoryCrate.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/FactoryObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/HouseObject.h"
#include "ZoneServer/Objects/MountObject.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/PlayerStructure.h"
#include "ZoneServer/GameSystemManagers/Spatial Index Manager/SpatialIndexManager.h"

#include "ZoneServer/GameSystemManagers/Container Manager/Container.h"

#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"


bool					ContainerManager::mInsFlag    = false;
ContainerManager*		ContainerManager::mSingleton  = NULL;


ContainerManager::ContainerManager() {}


ContainerManager* ContainerManager::Init(swganh::database::Database* database) {
    if(!mInsFlag) {
        mSingleton = new ContainerManager();
        mInsFlag = true;
        return mSingleton;
    } else {
        return mSingleton;
    }
}


void	ContainerManager::Shutdown() {}


bool	ContainerManager::transferItem(uint64 targetContainerId, uint64 transferItemId, PlayerObject* player, uint32 linkType)	
{
	
	Object*			itemObject		=	gWorldManager->getObjectById(transferItemId);
	//get our containers
	Object*			newContainer = gWorldManager->getObjectById(targetContainerId);
	Object*			oldContainer = gWorldManager->getObjectById(itemObject->getParentId());
	
	if(!gContainerManager->checkTargetContainer(targetContainerId,itemObject, player))
	{
		DLOG(info) << "ContainerManager::transferItem:TargetContainer is not valid :(";
		return false;
	}

	if(!gContainerManager->checkContainingContainer(itemObject->getParentId(), player->getId()))
	{
		DLOG(info) << "ContainerManager::transferItem:ContainingContainer is not allowing the transfer :(";
		return false;

	}
	
	// Remove the object from whatever contains it.
	if(!gContainerManager->removeFromContainer(targetContainerId, transferItemId, player))
	{
		DLOG(info) << "ContainerManager::transferItem: removeFromContainer failed :( this might be caused by looting a corpse though";
		return false;
	}
	
	//we need to destroy the old radial ... our item now gets a new one
	itemObject->ResetRadialMenu();

	itemObject->setParentId(targetContainerId); 

	//Now update the registered watchers!!
	gContainerManager->updateObjectPlayerRegistrations(newContainer, oldContainer, itemObject, linkType);

	//now go and move it to wherever it belongs
	
	//special case temp instrument
	Item* item = dynamic_cast<Item*>(itemObject);
	if (item&&item->getItemFamily() == ItemFamily_Instrument)
	{
		LOG(error) << "ContainerManager::transferItem transfer instrument : item : " << item->getId();
		LOG(error) << "ContainerManager::transferItem transfer instrument : placedInstrumentId : " << player->getPlacedInstrumentId();
		if (player->getPlacedInstrumentId() == item->getNonPersistantCopy())
		{
			player->getController()->destroyObject(player->getPlacedInstrumentId());
		}
	}
	
	CellObject* cell = dynamic_cast<CellObject*>(newContainer);
	if (cell)
	{
		itemObject->mPosition = player->mPosition;
		
		//do the db update manually because of the position - unless we get an automated position save in
		itemObject->setParentId(targetContainerId); 
		
		cell->addObjectSecure(itemObject);

		gMessageLib->sendDataTransformWithParent053(itemObject);
		itemObject->updateWorldPosition();

		return true;
		
	}	
	
	//if we are about to equip it
	if(newContainer->getId() == player->getId())	
	{
		//equip / unequip handles the db side, too
		if(!player->getEquipManager()->EquipItem(item))
		{
			LOG(warning) << "ContainerManager::_handleTransferItemMisc: Error equipping  " << item->getId();
			//panik!!!!!!
		}
		
		itemObject->setParentIdIncDB(newContainer->getId());
		return true;
	}

	//*****************************************************************
	//All special cases have been handled - now its just our generic ObjectContainer Type
	

	//some other container ... hopper backpack chest inventory etc
	if(newContainer)
	{
		newContainer->addObjectSecure(itemObject);
		itemObject->updateWorldPosition();
		itemObject->setParentIdIncDB(newContainer->getId());
		return true;
	}	

	return true;

}

bool	ContainerManager::removeFromContainer(uint64 targetId, uint64 objectId, PlayerObject* player)
{

	Object*			itemObject		=	gWorldManager->getObjectById(objectId);
	Inventory*		inventory		=	dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	TangibleObject* targetContainer = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(targetId));

	TangibleObject* tangible = dynamic_cast<TangibleObject*>(itemObject);

	Item* item = dynamic_cast<Item*>(itemObject);

	// its us
	if (tangible->getParentId() == player->getId())
	{
		// unequip it
		return player->getEquipManager()->unEquipItem(itemObject);
		
	}
	
	
	//creature inventories are a special case - their items are temporary!!! we cannot loot them directly
	CreatureObject* unknownCreature;
	Inventory*		creatureInventory;


	if (itemObject->getParentId() &&
		(unknownCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(itemObject->getParentId() - INVENTORY_OFFSET))) &&
		(creatureInventory = dynamic_cast<Inventory*>(unknownCreature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))) &&
		(creatureInventory->getId() == itemObject->getParentId()) && (creatureInventory->getId() != inventory->getId()))
	{
		
		if(!creatureInventory->removeObject(itemObject))
		{
			LOG(warning) << "ObjectController::removeFromContainer: Internal Error could not remove  " <<  itemObject->getId() << " from creature inventory "  << creatureInventory->getId();
			return false;
		}


		// we destroy the item in this case as its a temporary!! 
		// we do not want to clog the db with unlooted items
		gContainerManager->destroyObjectToRegisteredPlayers(creatureInventory, tangible->getId());

		ObjectIDList* invObjList = creatureInventory->getObjects();
		if (invObjList->size() == 0)
		{
			// Put this creature in the pool of delayed destruction and remove the corpse from scene.
			gWorldManager->addCreatureObjectForTimedDeletion(creatureInventory->getParentId(), LootedCorpseTimeout);
		}
		
		//if (gWorldConfig->isTutorial())
		//{
			// TODO: Update tutorial about the loot.
			//playerObject->getTutorial()->transferedItemFromContainer(targetId, creatureInventory->getId());
		//}

		//bail out here and request the item over the db - as the item in the NPC has a temporary id and we dont want that in the db
		// This ensure that we do not use/store any of the temp id's in the database.
        gObjectFactory->requestNewDefaultItem(inventory, item->getItemFamily(), item->getItemType(), inventory->getId(), 99, glm::vec3(), "");
		return false;

	}		   

	
	// Stop playing if we pick up the (permanently placed) instrument we are playing
	if (item && (item->getItemFamily() == ItemFamily_Instrument))
	{
		// It's a placeable original instrument.
		// Are we targeting the instrument we actually play on?
		if (player->getActiveInstrumentId() == item->getId())
		{
			gEntertainerManager->stopEntertaining(player);
		}	
	}

	//we *cannot* remove static tangibles like the structureterminal!!!!
	if(tangible->getStatic())
	{
		return false;
	}

	//some other container ... hopper backpack chest etc
	Object* containingContainer = dynamic_cast<Object*>(gWorldManager->getObjectById(tangible->getParentId()));
	if(containingContainer && containingContainer->removeObject(itemObject))
	{
		return true;
	}
	
	return false;
}


bool ContainerManager::checkContainingContainer(uint64 containingContainer, uint64 playerId)
{
	Object* container = gWorldManager->getObjectById(containingContainer);
	if(!container)	{
		return false;
	}

	//it might be our inventory or the inventory of a creature were looting
	//PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
	if(containingContainer == (playerId + INVENTORY_OFFSET))
	{
		//its our inventory ... - return true
		return true;
	}

	if(containingContainer == playerId)	{
		//its us
		return true;
	}

	uint64 ownerId = gSpatialIndexManager->getObjectMainParent(container);

	Object* owner = dynamic_cast<Object*>(gWorldManager->getObjectById(ownerId));

	if(BuildingObject* building = dynamic_cast<BuildingObject*>(owner))	{
		if(building->hasAdminRights(playerId))		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
			uint64 playerOwnerId = gSpatialIndexManager->getObjectMainParent(player);
			//are we in the same building?
			if(playerOwnerId == building->getId())	{
				//still get in a range check ???
				return true;
			}
		}
		return false;
	}
	
	PlayerObject* player = dynamic_cast<PlayerObject*>(owner);
	if(player)
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
	if(CreatureObject* creature = dynamic_cast<CreatureObject*>(owner))
	{
	}

	
	return true;
}



bool ContainerManager::checkTargetContainer(uint64 targetContainerId, Object* object, PlayerObject* player)
{
	PlayerObject*		targetPlayer	=	dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetContainerId));
	
	TangibleObject* tangibleItem = dynamic_cast<TangibleObject*>(object);
	
	//if its a backpack etc we want to know how many items are in it!
	uint32 objectSize = tangibleItem->getHeadCount();

	//********************
	//If we want to equip it we need to check for equip restrictions
	if(targetPlayer)	{
		return targetPlayer->getEquipManager()->CheckEquipable(object);		
	}
	
	//*****************************
	//ok everything else is an Object
	Object* targetContainer = gWorldManager->getObjectById(targetContainerId);
	
	//sanity check - 
	if(!targetContainer)	{
		DLOG(info) << "ContainerManager::_handleTransferItemMisc: TargetContainer is NULL :(";
		return false;
	}

	//====================================================================00
	//check access permissions first

	bool access = false;
	bool fit	 = false;


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
	//get the mainOwner of the container - thats a building or a player or an inventory or the main cell (0) for items in the main cell
	uint64 ownerId = gSpatialIndexManager->getObjectMainParent(targetContainer);
	
	Object* objectOwner = dynamic_cast<Object*>(gWorldManager->getObjectById(ownerId));

	if(BuildingObject* building = dynamic_cast<BuildingObject*>(objectOwner))
	{
		if(!building->hasAdminRights(player->getId()))
		{
			//You do not have permission to access that container. 
			gMessageLib->SendSystemMessage(L"",player,"container_error_message","container08");
			return false;
		}
		
		access = true;
		//do we have enough room ?
		if(!building->checkCapacity(objectSize))
		{
			//This building is full. 
			gMessageLib->SendSystemMessage(L"",player,"container_error_message","container03");
			return false;
		}

		if(!tangibleContainer)   //its probably a cell
			return true;

		if(!targetContainer->checkCapacity(objectSize,player))	{
			return false;
		}
			
		return false;			
	}

	//**********************************
	//this is our inventory - we are allowed to put stuff in there - but is there still enough place ?
	Inventory*		inventory		=	dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	if(inventory&& (inventory->getId() == ownerId))
	{
		//make sure its our inventory!!!!!!
		access = ((inventory->getId()- INVENTORY_OFFSET) == player->getId());
		if(!access)
		{
			//You do not have permission to access that container. 
			gMessageLib->SendSystemMessage(L"",player,"container_error_message","container08");
			return false;
		}
		
		//check space in inventory
		if(!inventory->checkCapacity(objectSize,player))
			return false;
		
	}	
	
	//if this is a tangible container (backpack, satchel) we want to make sure,
	//that we do not put another backpack in it.
	//in other words, the contained container MUST be smaller than the containing container

	//**********************
	//check capacity - return false if full
	//we wont get here if its an inventory
	if(tangibleContainer && (!tangibleContainer->checkCapacity(objectSize,player))) //automatically sends errormsg to player
	{
		return false;
	}

	uint32 containingContainersize =  tangibleContainer->getCapacity();
	uint32 containedContainersize =  tangibleItem->getCapacity();

	//we can only add smaller containers inside other containers
	if(containedContainersize >= containingContainersize)
	{
		//This item is too bulky to fit inside this container.
		gMessageLib->SendSystemMessage(L"",player,"container_error_message","container12");
		return false;
	}


	return true;
}


void ContainerManager::unRegisterPlayerFromContainer(Object* container, PlayerObject* const player) const {
    
	//limit logging for now
	if(container->getType() == ObjType_Player)	{
		DLOG(info) << "ContainerManager::unRegisterPlayerFromContainer :: unregister player " << player->getId() << " from (player)" << container->getId();
	}

    //bail out in case were not registered
    if(!container->unRegisterWatcher(player))	
	{
		DLOG(info) << "ContainerManager::unRegisterPlayerFromContainer :: cannot unregister player " << player->getId() << " from " << container->getId();
        return;
    }

	//send destroys only for containeritems NOT for items in the si - thats the SI managers Job
	//plus were not supposed to destroy buildings before we destroy the cells
	if(container->getParentId() != 0)	{
		gMessageLib->sendDestroyObject(container->getId(), player );
	}

    //buildings are a different kind of beast alltogether as cells need to be handled in a different way
	//as we were registered  to the building were privy to the cells contents
    if(container->getType() == ObjType_Building ) {
        BuildingObject* building = static_cast<BuildingObject*>(container);

        CellObjectList* cell_list = building->getCellList();

        std::for_each(cell_list->begin(), cell_list->end(), [=] (CellObject* cell) {
			//TODO check permission before continuing
            unRegisterPlayerFromContainer(cell, player);
        });
    } else {
        ObjectIDList* content_list = container->getObjects();

        std::for_each(content_list->begin(), content_list->end(), [=] (uint64_t object_id) {
            Object* object = gWorldManager->getObjectById(object_id);
			//TODO check permission before continuing
            if (object) {
                unRegisterPlayerFromContainer(object, player);
            }
        });
    }

	if(container->getType() == ObjType_Player)	{
		PlayerObject* containerPlayer = static_cast<PlayerObject*>(container);
		if(!player->unRegisterWatcher(containerPlayer))		{
			DLOG(info) << "ContainerManager::unRegisterPlayerFromContainer :: unregister player " << player->getId() << " from other player " << container->getId() << "failed!!!";
		}
	}
	
}



void ContainerManager::SendDestroyEquippedObject(Object* object) {
    CreatureObject* owner = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(object->getParentId()));
    if(!owner) {
        DLOG(info) << "SpatialIndexManager::SendDestroyEquippedObject :: cannot find " << object->getId() <<" owner :" << object->getParentId();
        return;
    }

    sendToRegisteredWatchers(owner, [object] (PlayerObject* const player) {
        gMessageLib->sendDestroyObject(object->getId(), player);
    });
}


void ContainerManager::registerPlayerToContainer(Object* container, PlayerObject* const player) const {
    DLOG(info) << "SpatialIndexManager::registerPlayerToContainer :: Try register player " << player->getId() << "to container " << container->getId();
	if (!container->registerWatcher(player))	{
        DLOG(info) << "SpatialIndexManager::registerPlayerToContainer :: Container " << container->getId() << " already known to player" << player->getId();
		return;
    }
	DLOG(info) << "SpatialIndexManager::registerPlayerToContainer :: player " << player->getId() << " succesfully registered to container " << container->getId();
    ObjectIDList* content_list = container->getObjects();
	
	//iterate through the containers children and create them for the player
    std::for_each(content_list->begin(), content_list->end(), [=] (uint64_t object_id) {
        Object* object = gWorldManager->getObjectById(object_id);
        if(object && object->getType() == ObjType_Tangible) {
            gSpatialIndexManager->sendCreateObject(object, player, false);
            registerPlayerToContainer(object, player);
        }
    });
}



void ContainerManager::registerPlayerToBuilding(BuildingObject* building, PlayerObject* player) {
    if (!building) {
        assert(false && "SpatialIndexManager::registerPlayerToBuilding no building");
        return;
    }

    if (!player) {
        assert(false && "SpatialIndexManager::registerPlayerToBuilding invalid player");
        return;
    }

    //cells are not subcontainers that get autoregistered on registering a building!!
    //they already are registered, as a building cannot be created without cells without crashing the client
    //the buildings contents however we want to create once we enter the building

    registerPlayerToContainer(building,player);

    CellObjectList* cell_list = building->getCellList();

    std::for_each(cell_list->begin(), cell_list->end(), [=] (CellObject* cell) {
        registerPlayerToContainer(cell, player);
    });
}


void ContainerManager::unRegisterPlayerFromBuilding(BuildingObject* building,PlayerObject* player) {
    if(!building) {
        assert(false && "SpatialIndexManager::unRegisterPlayerFromBuilding no building");
        return;
    }

    if (!player) {
        assert(false && "SpatialIndexManager::unRegisterPlayerFromBuilding invalid player");
        return;
    }

    //cells are not subcontainers that get autoregistered on registering a building!!
    //they already are registered, as a building cannot be created without cells without crashing the client
    //the buildings contents however we want to create once we enter the building

    unRegisterPlayerFromContainer(building, player);

    CellObjectList* cell_list = building->getCellList();

    std::for_each(cell_list->begin(), cell_list->end(), [=] (CellObject* cell) {
        unRegisterPlayerFromContainer(cell, player);
    });
}


void ContainerManager::createObjectToRegisteredPlayers(Object* container,Object* object) {
    //creatures players entering a cell get here, too
    //they are however already members of the grid so just ignore them
    if(object->getType() == ObjType_Creature) {
        return;
    }

    sendToRegisteredWatchers(container, [this, object] (PlayerObject* const recipient) {
        gSpatialIndexManager->sendCreateObject(object, recipient, false);

        //the registered object likely is a container in itself
        registerPlayerToContainer(object, recipient);
    });
}


void ContainerManager::updateEquipListToRegisteredPlayers(PlayerObject* const player) {
    sendToRegisteredWatchers(player, [player] (PlayerObject* const recepient) {
        gMessageLib->sendEquippedListUpdate(player, recepient);
    });
}


void ContainerManager::sendToRegisteredPlayers(Object* container, std::function<void (PlayerObject* const player)> callback) {
    PlayerObjectSet* registered_watchers	= container->getRegisteredWatchers();

    std::for_each(registered_watchers->begin(), registered_watchers->end(), [=] (PlayerObject* player) {
        callback(player);
    });
}


void ContainerManager::sendToRegisteredWatchers(Object* container, std::function<void (PlayerObject* const player)> callback)
{
    sendToRegisteredPlayers(container, callback);

    PlayerObjectSet* registered_watchers = container->getRegisteredStaticWatchers();

    std::for_each(registered_watchers->begin(), registered_watchers->end(), [=] (PlayerObject* player) {
        callback(player);
    });
}


void ContainerManager::sendToGroupedRegisteredPlayers(PlayerObject* const container, std::function<void ( PlayerObject* const player)> callback, bool self) {
    if(self && container)	{
        callback(container);
    }

    if(container->getGroupId() == 0) {
        return;
    }

    PlayerObjectSet* in_range_players = container->getRegisteredWatchers();

    std::for_each(in_range_players->begin(), in_range_players->end(), [=] (PlayerObject* player) {
        if(player && (player->getGroupId() == container->getGroupId()))	{
            callback(player);
        }
    });
}


void ContainerManager::destroyObjectToRegisteredPlayers(Object* container,uint64 object, bool destroyForSelf) {
    Object* destroyObject = gWorldManager->getObjectById(object);

    sendToRegisteredWatchers(container, [=] (PlayerObject* const recipient) {
        if((recipient->getId() != container->getId()) || (destroyForSelf == true)) {
            if(destroyObject) {
                unRegisterPlayerFromContainer(destroyObject,recipient);
            }

            //destroy it for the registered Players
            gMessageLib->sendDestroyObject(object,recipient);
        }
    });
}


void ContainerManager::updateObjectPlayerRegistrations(Object* newContainer, Object* oldContainer, Object* object, uint32 containment) {

    DLOG(info) << "SpatialIndexManager::updateObjectPlayerRegistrations :: newContainer " << newContainer->getId() << " oldContainer " << oldContainer->getId() << " - update old player set";

    //destroy for old Players that are NOT on the new list
    sendToRegisteredWatchers(oldContainer, [=] (PlayerObject* const recipient) {
        //if the watcher is still known just update the contaiment
        if(newContainer->checkRegisteredWatchers(recipient)) 	{
            DLOG(info) << "SpatialIndexManager::updateObjectPlayerRegistrations :: player " << recipient->getId() << " still known - update containment";
            gMessageLib->sendContainmentMessage(object->getId(), newContainer->getId(), containment, recipient);
        } else {
            //if the watcher is not known anymore destroy us and our content
            DLOG(info) << "SpatialIndexManager::updateObjectPlayerRegistrations :: player " << recipient->getId() << " will be unknown - remove";
            unRegisterPlayerFromContainer(object,recipient);

            //destroy it for the registered Players
            gMessageLib->sendDestroyObject(object->getId(),recipient);
        }
    });

    //create for all players on the new list who are NOT on the old list
    sendToRegisteredWatchers(newContainer, [=] (PlayerObject* const recipient) {
        if (!oldContainer->checkRegisteredWatchers(recipient)) {
            DLOG(info) << "SpatialIndexManager::updateObjectPlayerRegistrations :: player" << recipient->getId() << " added - create";
            gSpatialIndexManager->sendCreateObject(object, recipient, false);

            //the registered object likely is a container in itself
            registerPlayerToContainer(object,recipient);
        }
    });
}


void ContainerManager::deleteObject(Object* object, Object* parent) {
    destroyObjectToRegisteredPlayers(parent,object->getId());
    parent->removeObject(object);
    gObjectFactory->deleteObjectFromDB(object);
    gWorldManager->destroyObject(object);
}


void ContainerManager::removeObject(Object* object, Object* parent) {
    destroyObjectToRegisteredPlayers(parent,object->getId());
    parent->removeObject(object);
}
