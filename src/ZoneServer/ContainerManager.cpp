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

#include "SpatialIndexManager.h"

#include "ContainerManager.h"
#include "BuildingObject.h"
#include "CellObject.h"
#include "FactoryCrate.h"


#include "FactoryObject.h"
#include "HouseObject.h"
#include "PlayerObject.h"


#include "MountObject.h"
#include "WorldManager.h"
#include "PlayerStructure.h"
#include "ZoneOpcodes.h"
#include "ZoneServer.h"

#include <cassert>

//======================================================================================================================

bool					ContainerManager::mInsFlag    = false;
ContainerManager*		ContainerManager::mSingleton  = NULL;
//======================================================================================================================


ContainerManager*	ContainerManager::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new ContainerManager();
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

void ContainerManager::Shutdown()
{
	
}

ContainerManager::ContainerManager()
{

}


//===============================================================================================================================================================================
// UnRegisters a container to a player
// a container can be a backpack placed in a cell (or a cell itself) - unregistering it means that the content is not known to us anylonger
// the container itself might (but must not necessarily) still be known afterwards
// we will only destroy the contents for the client however, not the container and unregister the player
// unRegisterPlayerFromContainer *invalidates* the knownObject / knownPlayer iterator
void ContainerManager::unRegisterPlayerFromContainer(Object* container,PlayerObject* player)
{
	//are we sure the player doesnt know the container already ???
	if(!container->checkRegisteredWatchers(player))
	{

		//gLogger->log(LogManager::DEBUG,"SpatialIndexManager::UnRegisterPlayerToContainer :: Container %I64u not known to player %I64u",container->getId(),player->getId());
		return;	
								
	}

	//buildings are a different kind of animal all together
	//as cells need to be handled in a different way
	if(BuildingObject* building = dynamic_cast<BuildingObject*>(container))
	{
		container->unRegisterWatcher(player);
		player->unRegisterWatcher(container);

		CellObjectList::iterator cellIt = building->getCellList()->begin();

		while(cellIt != building->getCellList()->end())
		{
			unRegisterPlayerFromContainer((*cellIt),player);
			++cellIt;
		}

		//still destroy cells for the player
		CellObjectList* cellList = building->getCellList();
		CellObjectList::iterator it = cellList->begin();

		while(it != cellList->end())
		{
			gMessageLib->sendDestroyObject((*it)->getId(),player);
			it++;
		}

		gMessageLib->sendDestroyObject(container->getId(),player);
		return;
	}

	container->unRegisterWatcher(player);
	player->unRegisterWatcher(container);

	gMessageLib->sendDestroyObject(container->getId(),player);

	ObjectIDList*			contentList		= container->getObjects();
	ObjectIDList::iterator	it				= contentList->begin();
	
	//its important we do not destroy cells when unregistering a building!!
	while(it != contentList->end())
	{
		Object* tO = dynamic_cast<Object*>(gWorldManager->getObjectById((*it)));
		if(tO)
		{
			unRegisterPlayerFromContainer(tO,player);
		}
		
		it++;
	}
	
}

// sends destroys for an equipped object like hair/ a weapon / armor etc
// 
void ContainerManager::SendDestroyEquippedObject(Object *removeObject)
{
	CreatureObject* owner = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(removeObject->getParentId()));
	if(!owner)
	{
		gLogger->log(LogManager::DEBUG,"SpatialIndexManager::SendDestroyEquippedObject :: cannot find %I64u owner (%I64u)",removeObject->getId(),removeObject->getParentId());
		return;
	}
		
	sendToRegisteredPlayers(owner,[removeObject](PlayerObject* player)
	{
		gMessageLib->sendDestroyObject(removeObject->getId(),player);
	}
	);

}

//=================================================================================================================================================
// registers a containers content as known to a player
// a container can be a backpack placed in a cell or a cell itself
// registering the container just means that we know the content
// there are containers we register to automatically (like players)
// containers we register to when we enter them (buildings)
// and containers we register to when we look inside them (containers in the maincell)
// to register to a container we need to have the proper permissions

void ContainerManager::registerPlayerToContainer(Object* container,PlayerObject* player)
{

	//are we sure the player doesnt know the container already ???
	if(container->checkRegisteredWatchers(player))
	{

		gLogger->log(LogManager::DEBUG,"SpatialIndexManager::registerPlayerToContainer :: Container %I64u already known to player %I64u",container->getId(),player->getId());
		return;	
								
	}

	if(PlayerObject* tO = dynamic_cast<PlayerObject*>(container))
	{
		gLogger->log(LogManager::DEBUG,"SpatialIndexManager::registerPlayerToContainer :: registered player (container) %I64u to player %I64u",container->getId(),player->getId());
	}

	container->registerWatcher(player);
	player-> registerWatcher(container);


	ObjectIDList*			contentList		= container->getObjects();
	ObjectIDList::iterator	it				= contentList->begin();

	while(it != contentList->end())
	{
		TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById((*it)));
		if(tO)
		{
			gSpatialIndexManager->sendCreateObject(tO,player,false);
			registerPlayerToContainer(tO,player);
		}
		
		it++;
	}
	
}

void ContainerManager::registerPlayerToStaticContainer(Object* container,PlayerObject* player)
{

	//are we sure the player doesnt know the container already ???
	if(container->checkRegisteredWatchers(player))
	{

		gLogger->log(LogManager::DEBUG,"SpatialIndexManager::registerPlayerToContainer :: Container %I64u already known to player %I64u",container->getId(),player->getId());
		return;	
								
	}

	if(PlayerObject* tO = dynamic_cast<PlayerObject*>(container))
	{
		gLogger->log(LogManager::DEBUG,"SpatialIndexManager::registerPlayerToContainer :: registered player (container) %I64u to player %I64u",container->getId(),player->getId());
	}

	container->registerStatic(player);
	player-> registerStatic(container);


	ObjectIDList*			contentList		= container->getObjects();
	ObjectIDList::iterator	it				= contentList->begin();

	while(it != contentList->end())
	{
		TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById((*it)));
		if(tO)
		{
			gSpatialIndexManager->sendCreateObject(tO,player,false);
			registerPlayerToContainer(tO,player);
		}
		
		it++;
	}
	
}
//===============================================================================================================================================================================
//registering a player to a building is different, as the cells of a building must be known at all time to a player (as long as we know the building)
//the cellcontent is not loaded on building creation
//cellcontent gets loaded once we enter (registered to) the building 

void ContainerManager::registerPlayerToBuilding(BuildingObject* building,PlayerObject* player)
{
	//iterate through all the cells and add and register their content
	
	//BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));

	if(!building)
	{
		assert(false && "SpatialIndexManager::registerPlayerToBuilding no building");
		return;
	}
	
	//cells are not subcontainers that get autoregistered on registering a building!!
	//they already are registered, as a building cannot be created without cells without crashing the client
	//the buildings contents however we want to create once we enter the building

	registerPlayerToContainer(building,player);

	//iterate through all the cells and add the player as listener
	if(player)
	{		
		CellObjectList::iterator cellIt = building->getCellList()->begin();

		while(cellIt != building->getCellList()->end())
		{
			registerPlayerToContainer((*cellIt),player);
			++cellIt;
		}
	}

}

//===============================================================================================================================================================================
//unregistering a player from a building is different, as the cells of a building must be known at all time to a player (as long as we know the building)
//but we register cells only when the player enters the building
//even if the cellscontent can be destroyed unless of course we go out of range and the building is destroyed
void ContainerManager::unRegisterPlayerFromBuilding(BuildingObject* building,PlayerObject* player)
{
	if(!building)
	{
		assert(false && "SpatialIndexManager::unRegisterPlayerFromBuilding no building");
		return;
	}
	
	//cells are not subcontainers that get autoregistered on registering a building!!
	//they already are registered, as a building cannot be created without cells without crashing the client
	//the buildings contents however we want to create once we enter the building

	unRegisterPlayerFromContainer(building,player);

	//iterate through all the cells and add the player as listener
	if(player)
	{		
		CellObjectList::iterator cellIt = building->getCellList()->begin();

		while(cellIt != building->getCellList()->end())
		{
			unRegisterPlayerFromContainer((*cellIt),player);
			++cellIt;
		}
	}
	
}



void ContainerManager::createObjectToRegisteredPlayers(Object* container,Object* object)
{
	//creatures players entering a cell get here, too
	//they are however already members of the grid so just ignore them
	if(CreatureObject* creature = dynamic_cast<CreatureObject*>(object))
	{
		return;
	}

	sendToRegisteredPlayers(container,[object, this] (PlayerObject* recipient) 
		{
			gSpatialIndexManager->sendCreateObject(object,recipient,false);
		
			//the registered object likely is a container in itself
			registerPlayerToContainer(object,recipient);
		}
	);
	
}

//=======================================================================
// player / creature equipped something - update all registered listeners with the changed equiplist
void ContainerManager::updateEquipListToRegisteredPlayers(PlayerObject* player)
{

	sendToRegisteredPlayers(player,[player](PlayerObject* recepient)
		{
			gMessageLib->sendEquippedListUpdate(player, recepient);
		}
	);
}


// sends given function to all of the containers registered watchers
void ContainerManager::sendToRegisteredPlayers(Object* container, std::function<void (PlayerObject* player)> callback)
{

	PlayerObjectSet* knownPlayers = container->getRegisteredWatchers();
	PlayerObjectSet::iterator it = knownPlayers->begin();
		
	while(it != knownPlayers->end())
	{
		//create it for the registered Players
		PlayerObject* player = dynamic_cast<PlayerObject*>(*it);
		if(player)
		{
			callback(player);
		}
	
		it++;
	}

	knownPlayers = container->getRegisteredStaticWatchers();
	it = knownPlayers->begin();

	while(it != knownPlayers->end())
	{
		//create it for the registered Players
		PlayerObject* player = dynamic_cast<PlayerObject*>(*it);
		if(player)
		{
			callback(player);
		}
	
		it++;
	}
}

// sends given function to all of the containers registered watchers
void ContainerManager::sendToGroupedRegisteredPlayers(CreatureObject* container, std::function<void (PlayerObject* player)> callback, bool self)
{
	PlayerObject* ourSelve;
	if(self && (ourSelve = dynamic_cast<PlayerObject*>(container)))
	{
		callback(ourSelve);
	}

	if(container->getGroupId() == 0)
	{
		return;
	}

	PlayerObjectSet* knownPlayers = container->getRegisteredWatchers();
	PlayerObjectSet::iterator it = knownPlayers->begin();
		
	while(it != knownPlayers->end())
	{
		//create it for the registered Players
		PlayerObject* player = dynamic_cast<PlayerObject*>(*it);
		if(player && (player->getGroupId() == container->getGroupId()))
		{
			callback(player);
		}
	
		it++;
	}
}

//=======================================================================
// we remove an Object for all the players registered to *container*
// if destroyForSelf is false we will not destroy it for the player who is the recipient (used for transfers when the player remains registered: ie in the inventory)
//
void ContainerManager::destroyObjectToRegisteredPlayers(Object* container,uint64 object, bool destroyForSelf)
{
	sendToRegisteredPlayers(container,[container, object, this, destroyForSelf] (PlayerObject* recipient) 
		{
			if((recipient->getId() != container->getId()) || (destroyForSelf == true))
			{
				Object* destroyObject = gWorldManager->getObjectById(object);
				if(destroyObject)
					this->unRegisterPlayerFromContainer(destroyObject,recipient);

				//destroy it for the registered Players
				gMessageLib->sendDestroyObject(object,recipient);
			}
		}
	);

	//the destroyed object can be a container in itself
	//if so destroy it for the parent containers known players
	
}

//=======================================================================
// we remove an Object for all the players registered to *container*
// if destroyForSelf is false we will not destroy it for the player who is the recipient (used for transfers when the player remains registered: ie in the inventory)
//
void ContainerManager::updateObjectPlayerRegistrations(Object* newContainer, Object* oldContainer, Object* object, uint32 containment)
{

	gLogger->log(LogManager::DEBUG,"SpatialIndexManager::updateObjectPlayerRegistrations :: newContainer %I64u oldContainer %I64u - update old player set",newContainer->getId(),oldContainer->getId());
	//destroy for old Players that are NOT on the new list
	sendToRegisteredPlayers(oldContainer,[oldContainer, object, this, newContainer, containment] (PlayerObject* recipient) 
		{
			//if the watcher is still known just update the contaiment
			if(newContainer->checkRegisteredWatchers(recipient))
			{
				gLogger->log(LogManager::DEBUG,"SpatialIndexManager::updateObjectPlayerRegistrations :: player %I64u still known - update containment",recipient->getId());
				gMessageLib->sendContainmentMessage(object->getId(),newContainer->getId(),containment,recipient);
			}
			else//if the watcher is not known anymore destroy us and our content
			{
				gLogger->log(LogManager::DEBUG,"SpatialIndexManager::updateObjectPlayerRegistrations :: player %I64u will be unknown - remove",recipient->getId());
				this->unRegisterPlayerFromContainer(object,recipient);

				//destroy it for the registered Players
				gMessageLib->sendDestroyObject(object->getId(),recipient);
			}
		}
	);

	//create for all players on the new list who are NOT on the old list
	sendToRegisteredPlayers(newContainer,[object, this, oldContainer] (PlayerObject* recipient) 
		{
			if(!oldContainer->checkRegisteredWatchers(recipient))
			{
				gLogger->log(LogManager::DEBUG,"SpatialIndexManager::updateObjectPlayerRegistrations :: player %I64u added - create",recipient->getId());
				gSpatialIndexManager->sendCreateObject(object,recipient,false);
		
				//the registered object likely is a container in itself
				registerPlayerToContainer(object,recipient);
			}
		}
	);
	
	
}