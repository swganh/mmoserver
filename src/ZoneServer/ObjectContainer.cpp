/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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
		PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
		//gMessageLib->sendSystemMessage(player,L"","base_player","too_many_waypoints");
		return false;

	}
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
	gLogger->logMsgF("ObjectContainer::getDataById Data %I64u not found",MSG_HIGH, id);
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
	gLogger->logMsgF("ObjectContainer::removeDataByPointer Data %I64u not found",MSG_HIGH, data->getId());
	return false;
}

//=============================================================================
// removes it out of the container and destroys it in the worldmanager
// get in removal from db and zone, too ?
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
	gLogger->logMsgF("ObjectContainer::removeDataByPointer Data %I64u not found",MSG_HIGH, data->getId());
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
	gLogger->logMsgF("ObjectContainer::removeDataById  %I64u not found",MSG_HIGH, id);
	return false;
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
		gLogger->logMsgF("ObjectContainer::handleObjectReady :No tangible ????", MSG_NORMAL);
		return;
	}
	// reminder: objects are owned by the global map, our item (container) only keeps references

	gWorldManager->addObject(object,true);

	//add it to our container list
	this->addObject(object);

	CraftingTool* tool = dynamic_cast<CraftingTool*>(object);

	//==========================
	//update the world - who is looking into our container ?

	// find the main containing object
	// thats an inventory, a cell or a factory - they are the object registered in the SI 
	// and can tell us who we need to update
	uint64 mainParent = getObjectMainParent(object);

	if(!mainParent)
	{
		gLogger->logMsgF("ObjectContainer::handleObjectReady :No main parent ???? Object ID %I64u", MSG_NORMAL, object->getId());
		return;

	}
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mainParent));
	
	if(player)
	{
		//send the update to the player only
		if(player)
		{
			gMessageLib->sendCreateObject(object,player,false);
			if(tool&&tool->getCurrentItem())
			{
				gMessageLib->sendUpdateTimer(tool,player);
			}
		}

		return;
	}

	// no need to check the type again - getObjectMainParent() did that already
	// ás its not an inventory this leaves us with factory or cell
	Object* ParentObject = dynamic_cast<Object*>(gWorldManager->getObjectById(mainParent));	
	if(ParentObject)
	{
		PlayerObjectSet*			knownPlayers	= ParentObject->getKnownPlayers();
		PlayerObjectSet::iterator	playerIt		= knownPlayers->begin();
		
		while(playerIt != knownPlayers->end())
		{
			PlayerObject* player = (*playerIt);
			gMessageLib->sendCreateObject(object,player,false);
		
			if(tool&&tool->getCurrentItem())
			{
				PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));
				gMessageLib->sendUpdateTimer(tool,player);
			}

			player++;
		}


		return;
	}

	// send the creates to everyone on our containers knownObjectslist
	// please note that only makes sense for containers in the SI - containers in the inventory need to
	// be handled differently!!!
	
	// find out whether we need to try and find a parent
	// valid parents for
}

//============================================================================================================
// the idea is that the container holding our new item might be held by a container, too
// should this happen, we need to find the main container to determin what kind of creates to send to our player/s
// we will iterate through the parentObjects until the parent is either a player (item has been in the inventory)
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
		}
	}
	else
	{
		return parentID-1;
		//Inventory is parent ID +1 - we cannot find inventories in the worldObjectMap but we can find players there
		//so we have to go this way
		//before changing this we need to settle the dispute what objects are part of the world objectmap and need to discuss objectownership
		//Eru is right in saying that we cant have two object owners (well we can but obviously we shouldnt)
	}

	return parentID;
}

