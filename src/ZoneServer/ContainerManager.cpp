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

#include "ZoneServer/BuildingObject.h"
#include "ZoneServer/CellObject.h"
#include "ZoneServer/ContainerManager.h"
#include "ZoneServer/FactoryCrate.h"
#include "ZoneServer/FactoryObject.h"
#include "ZoneServer/HouseObject.h"
#include "ZoneServer/MountObject.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/PlayerStructure.h"
#include "ZoneServer/SpatialIndexManager.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"


bool					ContainerManager::mInsFlag    = false;
ContainerManager*		ContainerManager::mSingleton  = NULL;


ContainerManager::ContainerManager() {}


ContainerManager* ContainerManager::Init(Database* database) {
    if(!mInsFlag) {
        mSingleton = new ContainerManager();
        mInsFlag = true;
        return mSingleton;
    } else {
        return mSingleton;
    }
}


void ContainerManager::Shutdown() {}


void ContainerManager::unRegisterPlayerFromContainer(Object* container, PlayerObject* const player) const {
    
	//limit logging for now
	if(container->getType() == ObjType_Player)	{
		DLOG(INFO) << "ContainerManager::unRegisterPlayerFromContainer :: unregister player " << player->getId() << " from (player)" << container->getId();
	}

    //bail out in case were not registerted
    if(!container->unRegisterWatcher(player))	{
		if(container->getType() == ObjType_Player)	{
			DLOG(INFO) << "ContainerManager::unRegisterPlayerFromContainer :: unregister player " << player->getId() << " from" << container->getId() << "failed!!!";
		}
        return;
    }

	//send destroys only for containeritems NOT for items in the si - thats the SI managers Job
	if(container->getParentId() != 0)	{
		gMessageLib->sendDestroyObject(container->getId(), player );
	}

    //buildings are a different kind of animal all together
    //as cells need to be handled in a different way
    if(container->getType() == ObjType_Building ) {
        BuildingObject* building = static_cast<BuildingObject*>(container);
        if (!building) {
            assert(false && "Object says it's a building when it's not!");
            return;
        }

        CellObjectList* cell_list = building->getCellList();

        std::for_each(cell_list->begin(), cell_list->end(), [=] (CellObject* cell) {
            unRegisterPlayerFromContainer(cell, player);
            gMessageLib->sendDestroyObject(cell->getId(), player);
        });
    } else {
        ObjectIDList* content_list = container->getObjects();

        std::for_each(content_list->begin(), content_list->end(), [=] (uint64_t object_id) {
            Object* object = gWorldManager->getObjectById(object_id);
            if (object) {
                unRegisterPlayerFromContainer(object, player);
            }
        });
    }

	if(container->getType() == ObjType_Player)	{
		PlayerObject* containerPlayer = static_cast<PlayerObject*>(container);
		if(!player->unRegisterWatcher(containerPlayer))		{
			assert(false);
		}
	}
	else
    if(!player->unRegisterWatcher(container))	{
		DLOG(INFO) << "ContainerManager::unRegisterPlayerFromContainer :: unregister container " << container->getId()  << " from  player " << player->getId() << "failed" ;
		assert(false);
	}
}



void ContainerManager::SendDestroyEquippedObject(Object* object) {
    CreatureObject* owner = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(object->getParentId()));
    if(!owner) {
        DLOG(INFO) << "SpatialIndexManager::SendDestroyEquippedObject :: cannot find " << object->getId() <<" owner :" << object->getParentId();
        return;
    }

    sendToRegisteredWatchers(owner, [object] (PlayerObject* const player) {
        gMessageLib->sendDestroyObject(object->getId(), player);
    });
}


void ContainerManager::registerPlayerToContainer(Object* container, PlayerObject* const player) const {
    if (!container->registerWatcher(player))	{
        DLOG(INFO) << "SpatialIndexManager::registerPlayerToContainer :: Container " << container->getId() << " already known to player" << player->getId();
        return;
    }

	if(!player->registerWatcher(container))	{
		DLOG(INFO) << "SpatialIndexManager::registerPlayerToContainer :: player " << player->getId() << " already known to container" << container->getId();
		assert(false);
	}

    ObjectIDList* content_list = container->getObjects();

    std::for_each(content_list->begin(), content_list->end(), [=] (uint64_t object_id) {
        Object* object = gWorldManager->getObjectById(object_id);
        if(object && object->getType() == ObjType_Tangible) {
            gSpatialIndexManager->sendCreateObject(object, player, false);
            registerPlayerToContainer(object, player);
        }
    });
}


void ContainerManager::registerPlayerToStaticContainer(Object* container, PlayerObject* const player, bool player_create) const {

    //are we sure the player doesnt know the container already ???
    if (container->checkStatics(player) && (player_create == false)) {
        DLOG(INFO) << "SpatialIndexManager::registerPlayerToStaticContainer :: Container " << container->getId() << " already known to player" << player->getId();
        return;
    }

    if(container->getType() == ObjType_Player) {
        DLOG(INFO) << "SpatialIndexManager::registerPlayerToContainer :: registered player (container) " << container->getId() <<" to player %I64u" << player->getId();
        return;
    }

    container->registerStatic(player);
    player-> registerStatic(container);

    ObjectIDList* content_list = container->getObjects();

    std::for_each(content_list->begin(), content_list->end(), [=] (uint64_t object_id) {
        Object* object = gWorldManager->getObjectById(object_id);
        if(object && object->getType() == ObjType_Tangible) {
            TangibleObject* tangible = dynamic_cast<TangibleObject*>(object);
            gSpatialIndexManager->sendCreateTangible(tangible, player, false);
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

    DLOG(INFO) << "SpatialIndexManager::updateObjectPlayerRegistrations :: newContainer " << newContainer->getId() << " oldContainer " << oldContainer->getId() << " - update old player set";

    //destroy for old Players that are NOT on the new list
    sendToRegisteredWatchers(oldContainer, [=] (PlayerObject* const recipient) {
        //if the watcher is still known just update the contaiment
        if(newContainer->checkRegisteredWatchers(recipient)) 	{
            DLOG(INFO) << "SpatialIndexManager::updateObjectPlayerRegistrations :: player " << recipient->getId() << " still known - update containment";
            gMessageLib->sendContainmentMessage(object->getId(), newContainer->getId(), containment, recipient);
        } else {
            //if the watcher is not known anymore destroy us and our content
            DLOG(INFO) << "SpatialIndexManager::updateObjectPlayerRegistrations :: player " << recipient->getId() << " will be unknown - remove";
            unRegisterPlayerFromContainer(object,recipient);

            //destroy it for the registered Players
            gMessageLib->sendDestroyObject(object->getId(),recipient);
        }
    });

    //create for all players on the new list who are NOT on the old list
    sendToRegisteredWatchers(newContainer, [=] (PlayerObject* const recipient) {
        if (!oldContainer->checkRegisteredWatchers(recipient)) {
            DLOG(INFO) << "SpatialIndexManager::updateObjectPlayerRegistrations :: player" << recipient->getId() << " added - create";
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
