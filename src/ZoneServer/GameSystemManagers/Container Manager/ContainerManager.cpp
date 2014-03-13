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

#include "ZoneServer\Objects\permissions\container_permissions_interface.h"

#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "ZoneServer\Services\equipment\equipment_service.h"


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
	
	if(!gContainerManager->checkTargetContainer(targetContainerId,itemObject, player))	{
		DLOG(info) << "ContainerManager::transferItem:TargetContainer is not valid :(";
		return false;
	}

	
	if (!oldContainer->GetPermissions()->canRemove(oldContainer, player->GetCreature(), itemObject))	{
		DLOG(info) << "ContainerManager::transferItem: GetPermissions ContainingContainer is not allowing the transfer :(";
		return false;
	}

	if (!newContainer->GetPermissions()->canInsert(newContainer, player->GetCreature(), itemObject))	{
		DLOG(info) << "ContainerManager::transferItem: GetPermissions ContainingContainer is not allowing the transfer :(";
		return false;
	}

	//we *cannot* remove static tangibles like the structureterminal!!!!
	if(itemObject->getStatic())	{
		return false;
	}

	//we need to destroy the old radial ... our item now gets a new one
	itemObject->ResetRadialMenu();

	//itemObject->setParentId(targetContainerId); 

	int32 arrangement_id = newContainer->GetAppropriateArrangementId(itemObject);
	LOG(info) << "ContainerManager::transferItem arrangement id : " << arrangement_id;
	

	oldContainer->TransferObject(player, itemObject, newContainer, player->mPosition);

		

	return true;

}


bool ContainerManager::checkTargetContainer(uint64 targetContainerId, Object* object, PlayerObject* player)
{
	CreatureObject*		targetPlayer	=	dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetContainerId));
	
	TangibleObject*		tangibleItem	=	dynamic_cast<TangibleObject*>(object);
	
	Object*				targetContainer =	gWorldManager->getObjectById(targetContainerId);

	//if its a backpack etc we want to know how many items are in it!
	uint32 objectSize = tangibleItem->getHeadCount();


	
	
	//sanity check - 
	if(!targetContainer)	{
		DLOG(info) << "ContainerManager::_handleTransferItemMisc: TargetContainer is NULL :(";
		return false;
	}
	return true;

	if(!targetContainer->checkCapacity(objectSize,player))	{
		return false;
	}
			
	return true;			

	/*
	//====================================================================00
	//check access permissions first

	bool access = false;
	bool fit	 = false;


	//====================================================================================
	//get the mainOwner of the container - thats a building or a player or an inventory or the main cell (0) for items in the main cell
	uint64 ownerId = gSpatialIndexManager->getObjectMainParent(targetContainer);
	
	Object* objectOwner = dynamic_cast<Object*>(gWorldManager->getObjectById(ownerId));

	if(BuildingObject* building = dynamic_cast<BuildingObject*>(objectOwner))
	{
		if(!building->hasAdminRights(player->getId()))
		{
			//You do not have permission to access that container. 
			gMessageLib->SendSystemMessage(std::u16string(),player,"container_error_message","container08");
			return false;
		}
		
		access = true;
		//do we have enough room ?
		if(!building->checkCapacity(objectSize))
		{
			//This building is full. 
			gMessageLib->SendSystemMessage(std::u16string(),player,"container_error_message","container03");
			return false;
		}
		if(!targetContainer->checkCapacity(objectSize,player))	{
			return false;
		}
			
		return false;			
	}

	//**********************************
	//this is our inventory - we are allowed to put stuff in there - but is there still enough place ?
	auto inventory = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService")->GetEquippedObject(player->GetCreature(), "inventory");
	
	if(inventory&& (inventory->getId() == ownerId))
	{
		//make sure its our inventory!!!!!!
		access = ((inventory->getId()- INVENTORY_OFFSET) == player->GetCreature()->getId());
		if(!access)
		{
			//You do not have permission to access that container. 
			gMessageLib->SendSystemMessage(std::u16string(),player,"container_error_message","container08");
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
		gMessageLib->SendSystemMessage(std::u16string(),player,"container_error_message","container12");
		return false;
	}

	*/
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

	if(container->getId() < 5100273716)	{
		DLOG(info) << "ContainerManager::unRegisterPlayerFromContainer :: cannot unregister statics!!!!!!!! ( " << player->getId() << " from " << container->getId() << " )";
        return;
	}

	//send destroys only for containeritems NOT for items in the si - thats the SI managers Job
	//plus were not supposed to destroy buildings before we destroy the cells
	if(container->getParentId() != 0)	{
		gMessageLib->sendDestroyObject(container->getId(), player );
	}


	container->ViewObjects(player, 0, false, [&] (Object* object) {
		unRegisterPlayerFromContainer(object, player);
	});

    //buildings are a different kind of beast alltogether as cells need to be handled in a different way
	//as we were registered  to the building were privy to the cells contents
    if(container->getType() == ObjType_Building ) {
		DLOG(info) << "ContainerManager::unRegisterPlayerFromContainer :: unregister Building!!!!!!!! ( " << player->getId() << " from " << container->getId() << " )";
	}            

	if(container->getObjectType() == SWG_PLAYER)	{
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


void ContainerManager::initializePlayerToContainer(Object* container, PlayerObject* const player) const {

	//DLOG(info) << "SpatialIndexManager::initializePlayerToContainer :: player " << player->getId() << " succesfully initialized to container " << container->getId();
    
	container->ViewObjects(player, 0, false, [&] (Object* object) {
		if(object && (object->getId() >= 5100273716)) {
            gSpatialIndexManager->sendCreateObject(object, player, false);
			
			// check the permission
			if(!object->GetPermissions()->canView(object, player))	{
				DLOG(info) << "SpatialIndexManager::initializePlayerToContainer :: player " << player->getId() << " got no permission to initialize " << object->getId();
				return;
			}
            initializePlayerToContainer(object, player);
        }
	
	});
	
}

void ContainerManager::registerPlayerToContainer(Object* container, PlayerObject* const player) const {
    //DLOG(info) << "SpatialIndexManager::registerPlayerToContainer :: Try register player " << player->getId() << "to container " << container->getId();
	if (!container->registerWatcher(player))	{
        DLOG(info) << "SpatialIndexManager::registerPlayerToContainer :: Container " << container->getId() << " already known to player" << player->getId();
		return;
    }
	//DLOG(info) << "SpatialIndexManager::registerPlayerToContainer :: player " << player->getId() << " succesfully registered to container " << container->getId();
    
	container->ViewObjects(player, 0, false, [&] (Object* object) {
        if(object && (object->getId() >= 5100273716)) {
            gSpatialIndexManager->sendCreateObject(object, player, false);

			// check the permission
			if(!object->GetPermissions()->canView(object, player))	{
				DLOG(info) << "SpatialIndexManager::initializePlayerToContainer :: player " << player->getId() << " got no permission to register to " << object->getId();
				return;
			}

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

	//DLOG(info) << "SpatialIndexManager::registerPlayerToBuilding :: player " << player->getId() << " succesfully registered to container " << building->getId();

    //cells are subcontainers that get autoregistered on registering a building
    //as a building cannot be created without cells without crashing the client
    //the buildings contents however we want to create once we enter the building, as in playercities the traffic would explode otherwise

    registerPlayerToContainer(building,player);

    building->ViewObjects(player, 0, false, [&] (Object* object) {    
        registerPlayerToContainer(object, player);
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

    building->ViewObjects(player, 0, false, [&] (Object* object) {    
        unRegisterPlayerFromContainer(object, player);
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


void ContainerManager::sendToRegisteredPlayers(Object* container, std::function<void (PlayerObject* const player)> callback) {
    PlayerObjectSet* registered_watchers	= container->getRegisteredWatchers();

    std::for_each(registered_watchers->begin(), registered_watchers->end(), [=] (PlayerObject* player) {
        callback(player);
    });
}


void ContainerManager::sendToRegisteredWatchers(Object* container, std::function<void (PlayerObject* const player)> callback)
{
    sendToRegisteredPlayers(container, callback);

}


void ContainerManager::sendToGroupedRegisteredPlayers(PlayerObject* container, std::function<void ( PlayerObject* const player)> callback, bool self) {
    if(self && container)	{
        callback(container);
    }

    if(container->GetCreature()->getGroupId() == 0) {
        return;
    }

    PlayerObjectSet* in_range_players = container->getRegisteredWatchers();

    std::for_each(in_range_players->begin(), in_range_players->end(), [=] (PlayerObject* player) {
        if(player && (player->GetCreature()->getGroupId() == container->GetCreature()->getGroupId()))	{
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
    parent->RemoveObject(nullptr, object);
    gObjectFactory->deleteObjectFromDB(object);
    gWorldManager->destroyObject(object);
}


void ContainerManager::removeObject(Object* object, Object* parent) {
    destroyObjectToRegisteredPlayers(parent,object->getId());
    parent->RemoveObject(nullptr, object);
}
