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
#include "ContainerManager.h"
#include "CellObject.h"
#include "Inventory.h"
#include "FactoryObject.h"
#include "BuildingObject.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"
#include "WorldManager.h"
#include "SpatialIndexManager.h"
#include "CraftingTool.h"

#include <cassert>

//=============================================================================

ObjectContainer::ObjectContainer()
{
    mCapacity = 0;
}

//=============================================================================


ObjectContainer::ObjectContainer(uint64 id,uint64 parentId,BString model,ObjectType type)
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
			DLOG(INFO) << "ObjectContainer::remove Object : No Object!!!!";
			assert(false && "ObjectContainer::~ObjectContainer WorldManager unable to find object instance");
			objectIt = removeObject(objectIt);
			
			continue;
		}

		if(object->getId() == 2533274790395904)
		{
			DLOG(INFO) << "ObjectContainer::remove Object 2533274790395904";
		}

		//take care of a crafting tool
		if(CraftingTool* tool = dynamic_cast<CraftingTool*>(object))
		{
			if(tool->getCurrentItem())
			{
				gWorldManager->removeBusyCraftTool(tool);

			}
		}

		//just deinitialization/removal out of mainObjectMap .. NO removal out of si/cells
		gWorldManager->destroyObject(object);

		objectIt = removeObject(objectIt);
	}
}

//=============================================================================
//use for cells - players must enter them of course - it might be prudent to separate											
//players from items though
bool ObjectContainer::addObjectSecure(Object* data) 
{ 
	
	mData.push_back(data->getId()); 

	if(mCapacity)
	{
		return true;
	}
	else
	{
		DLOG(INFO) << "ObjectContainer::addObjectSecure No Capacity!!!!";
		return true;

	}
}

//===============================================
//use only when youre prepared to receive a false result with a not added item
//returns false when the item couldnt be added (container full)

bool ObjectContainer::addObject(Object* data) 
{ 
	if(mCapacity)
	{
		mData.push_back(data->getId()); 
		return true;
	}
	else
	{
		DLOG(WARNING) << "ObjectContainer::addObject No Capacity left for container " << this->getId();			
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
			gWorldManager->destroyObject(data);
			return true;
		}
		++it;
	}
	DLOG(WARNING) << "ObjectContainer::removeDataByPointer Data " << data->getId() << " not found";
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
	DLOG(WARNING) << "ObjectContainer::removeDataById  " << id <<" not found";
	return false;
}

//=============================================================================

ObjectIDList::iterator ObjectContainer::removeObject(ObjectIDList::iterator it)
{
	it = mData.erase(it);
return it;
}

//=============================================================================
// *this* is obviously a container that gets to hold the item we just created
// we need to create this item to registered players
// please note that the inventory and the datapad handle their own ObjectReady functions!!!!

void ObjectContainer::handleObjectReady(Object* object,DispatchClient* client)
{
	
	//==========================
	// reminder: objects are owned by the global map, our item (container) only keeps references
	gWorldManager->addObject(object,true);

	//add it to the spatialIndex, too
	gSpatialIndexManager->createInWorld(object);

	this->addObject(object);
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
		//Inventory is parent ID +1 - we cannot find inventories in the worldObjectMap but we can find players there
		//so we have to go this way
		//before changing this we need to settle the dispute what objects are part of the world objectmap and need to discuss objectownership
		//Eru is right in saying that we cant have two object owners (well we can but obviously we shouldnt)
	}

	return parentID;
}
//deprecated
/*uint64 ObjectContainer::getObjectMainParent2(Object* object)
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
*/

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
        gMessageLib->SendSystemMessage(::common::OutOfBand("container_error_message", "container3"), player);
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
