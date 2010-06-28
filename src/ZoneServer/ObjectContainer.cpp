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

#include "ObjectContainer.h"
#include "CellObject.h"
#include "Inventory.h"
#include "FactoryObject.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"
#include "WorldManager.h"
#include "CraftingTool.h"

#include <cassert>

//=============================================================================

ObjectContainer::ObjectContainer() 
{
	mCapacity = 0;	
}

//=============================================================================


ObjectContainer::ObjectContainer(uint64 id,uint64 parentId,string model,ObjectType type) 
				:Object(id,parentId,model,ObjType_Tangible)
{
	mCapacity = 0;	
	//mData.reserve(80);

}

//=============================================================================

ObjectContainer::~ObjectContainer()
{
	
	ObjectIDList::iterator	objectIt = mData.begin();

	while(objectIt != mData.end())
	{
	 	Object* object = gWorldManager->getObjectById((*objectIt));
		if(!object)
		{
			gLogger->log(LogManager::DEBUG,"ObjectContainer::remove Object : No Object!!!!");
			assert(false && "ObjectContainer::~ObjectContainer WorldManager unable to find object instance");
			objectIt = removeObject(objectIt);
			
			continue;
		}

		//take care of a crafting tool
		if(CraftingTool* tool = dynamic_cast<CraftingTool*>(object))
		{
			if(tool->getCurrentItem())
			{
				gWorldManager->removeBusyCraftTool(tool);

			}
		}

		gWorldManager->destroyObject(object);

		objectIt = removeObject(objectIt);
	}
}

//=============================================================================
											
bool ObjectContainer::addObjectSecure(Object* Data) 
{ 
	mData.push_back(Data->getId()); 
	if(mCapacity)
	{
		return true;
	}
	else
	{
		gLogger->log(LogManager::DEBUG,"ObjectContainer::addObjectSecure No Capacity!!!!");
		return true;

	}
}

//==============================================================================0
//use only when youre prepared to receive a false result with a not added item
bool ObjectContainer::addObject(Object* Data) 
{ 
	if(mCapacity)
	{
		mData.push_back(Data->getId()); 
		//PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
		return true;
	}
	else
	{
		//PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
		gLogger->log(LogManager::DEBUG,"ObjectContainer::addObject No Capacity!!!!");
		//assert(false);// Another case where....why crash? We can continue just fine.
		//because crashing is fun :)))
		//plus obvioulsly someone uses the code without proper failsafes
		//and its good practice to find these spots :))))
		return false;

		//its not my fault I cant reach snows tc for 2 weeks ...
	}
}

//==========================================================================================0
//adds the Object to the container and creates it for a single player
//
bool ObjectContainer::addObject(Object* Data, PlayerObject* player) 
{ 
	if(!addObject(Data))
		return false;

	if(!player)
	{
		//its still added to the container
		gLogger->log(LogManager::DEBUG,"ObjectContainer::addObject No Capacity!!!!");
		return true;

	}
	
	gMessageLib->sendCreateObject(Data,player,false);
	CraftingTool* tool = dynamic_cast<CraftingTool*>(Data);
	if(tool&&tool->getCurrentItem())
	{
		gMessageLib->sendUpdateTimer(tool,player);
	}
	
	return true;
}

bool ObjectContainer::addObjectSecure(Object* Data, PlayerObject* player) 
{ 	

	if(!player)
	{
		gLogger->log(LogManager::DEBUG,"ObjectContainer::addObject No Capacity!!!!");
		return addObjectSecure(Data);
		
	}
	
	gMessageLib->sendCreateObject(Data,player,false);
	CraftingTool* tool = dynamic_cast<CraftingTool*>(Data);
	if(tool&&tool->getCurrentItem())
	{
		gMessageLib->sendUpdateTimer(tool,player);
	}

	return addObjectSecure(Data);
}

//==========================================================================================0
//adds the Object to the container and creates it for a single player
//
bool ObjectContainer::addObject(Object* Data,PlayerObjectSet*	knownPlayers) 
{ 
	if(!addObject(Data))
		return false;

	if(!knownPlayers||(!knownPlayers->size()))
	{
		//its still added to the container
		gLogger->log(LogManager::DEBUG,"ObjectContainer::addObject No Capacity!!!!");
		return true;

	}
	
	PlayerObjectSet::iterator	playerIt		= knownPlayers->begin();
	CraftingTool* tool = dynamic_cast<CraftingTool*>(Data);

	while(playerIt != knownPlayers->end())
	{
		PlayerObject* player = (*playerIt);
		gMessageLib->sendCreateObject(Data,player,false);
	
		if(tool&&tool->getCurrentItem())
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
			gMessageLib->sendUpdateTimer(tool,player);
		}

		playerIt++;
	}


	return true;
	
}

//========================================================================================================0
//ads an object for all surrounding players - use for SI objects

void ObjectContainer::addObjectSecure(Object* object, PlayerObjectSet* inRangePlayers, PlayerObject* player)
{
	addObjectSecure(object);

	PlayerObjectSet::iterator it = inRangePlayers->begin();
	while(it != inRangePlayers->end())
	{
		PlayerObject* targetObject = (*it);
		gMessageLib->sendCreateObject(object,targetObject);
		
		targetObject->addKnownObjectSafe(object);
		object->addKnownObjectSafe(targetObject);
		++it;
	}
	if(player)
	{
		gMessageLib->sendCreateObject(object,player);
		player->addKnownObjectSafe(object);
		object->addKnownObjectSafe(player);
	}
	return;
}

//=============================================================================

Object* ObjectContainer::getObjectById(uint64 id)
{
	ObjectIDList::iterator it = mData.begin();

	while(it != mData.end())
	{
		if((*it) == id) 
			return(gWorldManager->getObjectById((*it))); 

		++it;
	}
	gLogger->log(LogManager::DEBUG,"ObjectContainer::getDataById Data %I64u not found", id);
	return NULL;
}

//=============================================================================
//just removes it out of the container - the object gets not deleted in the worldmanager
//
bool ObjectContainer::removeObject(Object* data)
{
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == data->getId())
		{
			it = mData.erase(it);
			return true;
		}
		++it;
	}
	gLogger->log(LogManager::DEBUG,"ObjectContainer::removeDataByPointer Data %I64u not found", data->getId());
//	assert(false);
	return false;
}

//=============================================================================
// removes it out of the container and destroys it in the worldmanager
//
bool ObjectContainer::deleteObject(Object* data)
{
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == data->getId())
		{
			it = mData.erase(it);
			gWorldManager->destroyObject(data);
			return true;
		}
		++it;
	}
	gLogger->log(LogManager::DEBUG,"ObjectContainer::removeDataByPointer Data %I64u not found", data->getId());
	return false;
}


//=============================================================================

bool ObjectContainer::removeObject(uint64 id)
{
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == id)
		{
			it = mData.erase(it);
			return true;
		}
		++it;
	}
	gLogger->log(LogManager::DEBUG,"ObjectContainer::removeDataById  %I64u not found", id);
	return false;
}

bool ObjectContainer::removeObject(uint64 id, PlayerObject* player)
{
	if(!removeObject(id))
		return false;
	gMessageLib->sendDestroyObject(id,player);
	
	return true;
}

bool ObjectContainer::removeObject(Object* Data, PlayerObject* player)
{
	if(!removeObject(Data->getId()))
		return false;
	gMessageLib->sendDestroyObject(Data->getId(),player);
	return true;
}

bool ObjectContainer::removeObject(uint64 id, PlayerObjectSet*	knownPlayers)
{
	if(!removeObject(id))
		return false;

	PlayerObjectSet::iterator	playerIt		= knownPlayers->begin();

	while(playerIt != knownPlayers->end())
	{
		PlayerObject* player = (*playerIt);
		gMessageLib->sendDestroyObject(id,player);
	
		playerIt++;
	}
	return true;
}

bool ObjectContainer::removeObject(Object* Data, PlayerObjectSet*	knownPlayers)
{
	if(!removeObject(Data->getId()))
		return false;
		
	PlayerObjectSet::iterator	playerIt		= knownPlayers->begin();

	while(playerIt != knownPlayers->end())
	{
		PlayerObject* player = (*playerIt);
		gMessageLib->sendDestroyObject(Data->getId(),player);
	
		playerIt++;
	}
	
	return true;
}

ObjectIDList::iterator ObjectContainer::removeObject(ObjectIDList::iterator it, PlayerObjectSet*	knownPlayers)
{
	PlayerObjectSet::iterator	playerIt		= knownPlayers->begin();

	while(playerIt != knownPlayers->end())
	{
		PlayerObject* player = (*playerIt);
		gMessageLib->sendDestroyObject((*it),player);

		playerIt++;
	}

	it = mData.erase(it);

	return it;
}

ObjectIDList::iterator ObjectContainer::removeObject(ObjectIDList::iterator it, PlayerObject* player)
{
	gMessageLib->sendDestroyObject((*it),player);
	it = mData.erase(it);
	return it;
}


//=============================================================================

ObjectIDList::iterator ObjectContainer::removeObject(ObjectIDList::iterator it)
{
	it = mData.erase(it);
return it;
}

//=============================================================================
// the item is obviously a container that gets to hold the item we just created
// we need to find out who needs updates on the containers content - these are the players on the 
// knownObjects list of the container
// please note that the inventory and the datapad handle their Own ObjectReady functions!!!!

void ObjectContainer::handleObjectReady(Object* object,DispatchClient* client)
{

	TangibleObject* tO = dynamic_cast<TangibleObject*>(object);
	if(!tO)
	{
		gLogger->log(LogManager::DEBUG,"ObjectContainer::handleObjectReady :No tangible ????");
		return;
	}
	// reminder: objects are owned by the global map, our item (container) only keeps references

	gWorldManager->addObject(object,true);

	CraftingTool* tool = dynamic_cast<CraftingTool*>(object);

	//==========================
	//update the world - who is looking into our container ?

	// find the main containing object
	// thats an inventory, a cell or a factory - they are the object registered in the SI 
	// and can tell us who we need to update
	uint64 mainParent = getObjectMainParent(object);

	if(!mainParent)
	{
		gLogger->log(LogManager::DEBUG,"ObjectContainer::handleObjectReady :No main parent ???? Object ID %I64u", object->getId());
		return;

	}

	//TODO: handle error messages with full containers

	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mainParent));
	
	if(player)
	{
		//add it to our container list
		this->addObject(object,player);
		return;
	}

	// no need to check the type again - getObjectMainParent() did that already
	// ás its not an inventory this leaves us with factory or cell as mainparent
	Object* ParentObject = dynamic_cast<Object*>(gWorldManager->getObjectById(mainParent));	
	if(ParentObject)
	{
		PlayerObjectSet*			knownPlayers	= ParentObject->getKnownPlayers();
		this->addObject(object,knownPlayers);
		return;
	
	}

	// send the creates to everyone on our containers knownObjectslist
	// please note that only makes sense for containers in the SI - containers in the inventory need to
	// be handled differently!!! (IE only send to player (= mainparent) like done above
	
}

//============================================================================================================
// the idea is that the container holding our new item might be held by a container, too
// should this happen, we need to find the main container to determin what kind of creates to send to our player/s
// we will iterate through the parentObjects until the parent is either a player (item has been equipped) or in the inventory or )
// or a cell or a factory
uint64 ObjectContainer::getObjectMainParent(Object* object)
{

	uint64 parentID = object->getParentId();

	// hack ourselves a player - it is not possible to get an inventory otherwise because
	// inventories are not part of the WorldObjectMap ... which really gets cumbersome
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(parentID-1));
	if(!player)
	{
		// the backpack might have been equipped ?
		//   this way we have of course a player directly
		PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(parentID));
		if(!player)
		{
			CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(parentID));
			if(!cell)
			{
				CellObject* cell = dynamic_cast<CellObject*>(object);
				if(cell)
				{
					return parentID;
				}
				FactoryObject* factory = dynamic_cast<FactoryObject*>(gWorldManager->getObjectById(parentID));
				if(!factory)
				{
					Object* ob = dynamic_cast<Object*>(gWorldManager->getObjectById(parentID));
					if(!ob)
					{
						return 0;
					}
					parentID = getObjectMainParent(ob);
				}
			}
			else
			{
				//return the house
				return cell->getParentId();
			}
		}
	}
	else
	{
		//its in the inventory
		return parentID;
		//enum is INVENTORY_OFFSET
		//Inventory is parent ID +1 - we cannot find inventories in the worldObjectMap but we can find players there
		//so we have to go this way
		//before changing this we need to settle the dispute what objects are part of the world objectmap and need to discuss objectownership
		//Eru is right in saying that we cant have two object owners (well we can but obviously we shouldnt)
	}

	return parentID;
}

//=============================================================================================
// gets a headcount of all tangible (!!!) Objects in the container 
// including those contained in containers
uint16 ObjectContainer::getHeadCount()
{
	uint16 count = 0;

	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		//do NOT count static tangibles like the playerStructureTerminal
		TangibleObject* to = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(*it));
		if(to && (!to->getStatic()))
		{
		 	count += to->getHeadCount();
			count += 1; //implememt items counting more than 1 at one time	
		}
		++it;
	}
	return count;

}
bool ObjectContainer::checkCapacity(uint8 amount, PlayerObject* player)
{
	uint16 contentCount = getHeadCount();
	if(player&&(mCapacity-contentCount < amount))
	{
		gMessageLib->sendSystemMessage(player,L"","container_error_message","container3");
	}

	return((mCapacity-contentCount) >= amount);
}



bool ObjectContainer::checkForObject(Object* object)
{
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == object->getId())
		{
			return(true);
		}
		++it;
	}
	return(false);
}

void ObjectContainer::createContent(PlayerObject* player)
{
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		TangibleObject* to = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(*it));
		if(to && (!player->checkKnownObjects(to)))
		{
			gMessageLib->sendCreateObject(to,player);
			player->addKnownObjectSafe(to);
			to->addKnownObjectSafe(player);
		}
		++it;
	}
}
