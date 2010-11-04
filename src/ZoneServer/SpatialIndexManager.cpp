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

#include "BuildingObject.h"
#include "CellObject.h"
#include "FactoryCrate.h"
#include "FactoryObject.h"
#include "HouseObject.h"
#include "PlayerObject.h"
#include "RegionObject.h"

#include "MountObject.h"
#include "WorldManager.h"
#include "PlayerStructure.h"
#include "ZoneOpcodes.h"
#include "ZoneServer.h"

#include <cassert>

//======================================================================================================================

bool					SpatialIndexManager::mInsFlag    = false;
SpatialIndexManager*	SpatialIndexManager::mSingleton  = NULL;
//======================================================================================================================


SpatialIndexManager*	SpatialIndexManager::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new SpatialIndexManager();
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

void SpatialIndexManager::Shutdown()
{
	delete(mSpatialGrid);
}

SpatialIndexManager::SpatialIndexManager()
{
	mSpatialGrid = new zmap();
	gMessageLib->setGrid(mSpatialGrid);

}

//=================================================
//adds a new object to the grid
//creates the object for all players in range
//if object is a player all objects in range will be created

bool SpatialIndexManager::AddObject(Object *newObject)
{
	//Pesudo
	// 1. Calculate CellID
	// 2. Set CellID
	// 3. Insert object into the cell in the hash table

	//get the cell
	uint32 finalBucket = getGrid()->AddObject(newObject);
	gLogger->log(LogManager::DEBUG,"SpatialIndexManager::AddObject :: Object %I64u added to bucket %u",newObject->getId(), finalBucket);
	
	//any errors ?
	if(finalBucket == 0xffffffff)
	{
		gLogger->log(LogManager::DEBUG,"SpatialIndexManager::AddObject :: add Object %I64u to bucket %u failed because the bucket was invalid",newObject->getId(), finalBucket);
		return false;
	}

	//now create it for everyone around
	//and around for it
	if(newObject->getType() == ObjType_Player)
	{
		PlayerObject* player = dynamic_cast<PlayerObject*>(newObject);
		
		ObjectListType playerList;
		getGrid()->GetViewingRangeCellContents(finalBucket, &playerList,(Bucket_Creatures|Bucket_Objects|Bucket_Players));
		
		for(ObjectListType::iterator i = playerList.begin(); i != playerList.end(); i++)
		{
			if(((*i)->getId() != player->getId()))
			{
				sendCreateObject((*i),player, false);
				registerPlayerToContainer((*i), player);

				if(((*i)->getType() == ObjType_Player) )
				{
					PlayerObject* otherPlayer = dynamic_cast<PlayerObject*>((*i));
					
					sendCreateObject(player,otherPlayer, false);
					registerPlayerToContainer(player, otherPlayer);
				}
			}
		}
	}
	else 
	{
		ObjectListType playerList;
		getGrid()->GetPlayerViewingRangeCellContents(finalBucket, &playerList);
		for(ObjectListType::iterator i = playerList.begin(); i != playerList.end(); i++)
		{
			PlayerObject* otherPlayer = dynamic_cast<PlayerObject*>((*i));
			sendCreateObject(newObject,otherPlayer, false);
		}
	}

	return true;
}

void SpatialIndexManager::UpdateObject(Object *updateObject)
{

	uint32 oldBucket = updateObject->getGridBucket();
	
	//sets the new gridcell, updates subcells
	getGrid()->UpdateObject(updateObject);

	if(updateObject->getGridBucket() != oldBucket)
	{
		//test how much we moved if only one grid proceed normally
		if((updateObject->getGridBucket() == (oldBucket +1))||(updateObject->getGridBucket() == (oldBucket -1))
			||(updateObject->getGridBucket() == (oldBucket + GRIDWIDTH))	||(updateObject->getGridBucket() == (oldBucket - GRIDWIDTH))	
			)
		{
	
			//remove us from the row we left
			UpdateBackCells(updateObject,oldBucket);
			
			//create us for the row in which direction we moved
			UpdateFrontCells(updateObject,oldBucket);
		}
		else //we teleported destroy all and create everything new
		{
			//remove us from everything
			RemoveObject(updateObject,oldBucket);
			
			//and add us freshly to the world
			AddObject(updateObject);
		}
	
	}

		
}

// sends destroys for an equipped object like hair/ a weapon / armor etc
// 
void SpatialIndexManager::SendDestroyEquippedObject(Object *removeObject)
{
	CreatureObject* owner = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(removeObject->getParentId()));
	if(owner)
	{
		uint32 cell = owner->getGridBucket();
		ObjectListType playerList;
		getGrid()->GetPlayerViewingRangeCellContents(cell, &playerList);

		for(ObjectListType::iterator i = playerList.begin(); i != playerList.end(); i++)
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>((*i));
			gMessageLib->sendDestroyObject(removeObject->getId(),player);
		}

	}
	else
	{
		gLogger->log(LogManager::DEBUG,"SpatialIndexManager::SendDestroyEquippedObject :: cannot find %I64u owner (%I64u)",removeObject->getId(),removeObject->getParentId());
	}
}

void SpatialIndexManager::RemoveRegion(RegionObject *removeObject)
{
	getGrid()->RemoveSubCell(removeObject->subCellId);
}

void SpatialIndexManager::addRegion(RegionObject *region)
{
	//add the adequate subcells to the grid
	getGrid()->AddSubCell(region->mPosition.x,region->mPosition.z,region->getHeight(), region->getWidth(),region);
	
	//add the region as Object to the grid so we are able to search for nearby regions should it become necessary
	AddObject(region);
	
}

RegionObject* SpatialIndexManager::getRegion(uint32 id)
{
	return getGrid()->getSubCell(id);
	
}

//*********************************************************************
//a simple Object can only be in the grid *or* in the cell
//it can be equipped however by a creature / player
void SpatialIndexManager::RemoveObjectFromWorld(Object *removeObject)
{

	//were in a cell - get us out
	if(removeObject->getParentId())
	{
		Object* container = gWorldManager->getObjectById(removeObject->getParentId());
		if(container)
		{
			if(CreatureObject* owner = dynamic_cast<CreatureObject*>(container))
			{
				// remove from creatures slotmap
				owner->getEquipManager()->removeEquippedObject(removeObject);

				// send out the new equiplist
				gMessageLib->sendEquippedListUpdate_InRange(owner);				

				//destroy for players in the grid
				gSpatialIndexManager->SendDestroyEquippedObject(removeObject);
				
				//Bailout - the reson we use SendDestroyEquippedObject(object); instead of the (faster) destroyObjectToRegisteredPlayers
				//is that creatures do not get registered to players as containers (yet) - that might be an idea to change
				return;

			}

			destroyObjectToRegisteredPlayers(container,removeObject->getId());
		}
		
		//remove the object out of the container
		container->removeObject(removeObject);
		
		//no need to remove a tangible(!) from the grid if it was in a cell
		return;
	}

	//remove it out of the grid
	RemoveObject(removeObject, removeObject->getGridBucket());
}
 
//*********************************************************************
//a Player or creature is ALWAYS in the grid and possibly in a cell
void SpatialIndexManager::RemoveObjectFromWorld(PlayerObject *removePlayer)
{
	//remove us from the grid
	RemoveObject(removePlayer, removePlayer->getGridBucket());

	//remove us out of the cell
	if(removePlayer->getParentId() == 0)
	{
		return;
	}
	
	CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(removePlayer->getParentId()));
	if(cell)
	{
		if(BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId())))
		{
			this->unRegisterPlayerFromBuilding(building,removePlayer);
		}

		cell->removeObject(removePlayer);

	}
	else
	{
		gLogger->log(LogManager::DEBUG,"SpatialIndexManager::RemovePlayerFromWorld: couldn't find cell %"PRIu64"",removePlayer->getParentId());
	}


}

void SpatialIndexManager::RemoveObjectFromWorld(CreatureObject *removeCreature)
{
	//remove us from the grid
	RemoveObject(removeCreature, removeCreature->getGridBucket());

	//remove us out of the cell
	Object* container = gWorldManager->getObjectById(removeCreature->getParentId());
	if(container)
	{
		destroyObjectToRegisteredPlayers(container,removeCreature->getId());

		//remove the object out of the container
		container->removeObject(removeCreature);
	}


}


//===========================================================================
// this will remove an object from the grid
// if the object is a container with listeners, the listeners will be unregistered
void SpatialIndexManager::RemoveObject(Object *removeObject, uint32 gridCell)
{
	
	getGrid()->RemoveObject(removeObject);

	//now destroy it for everyone around as well as around for it
	
	ObjectListType playerList;
	//getGrid()->GetPlayerViewingRangeCellContents(gridCell, &playerList);//cell means gridcell here
	getGrid()->GetViewingRangeCellContents(gridCell, &playerList,(Bucket_Creatures|Bucket_Objects|Bucket_Players));
	
	PlayerObject* removePlayer = dynamic_cast<PlayerObject*>(removeObject);

	for(ObjectListType::iterator i = playerList.begin(); i != playerList.end(); i++)
	{
		PlayerObject* otherPlayer = dynamic_cast<PlayerObject*>((*i));

		//if we are a player unregister us from everything around
		if(removePlayer)
		{
			if((*i)->getId() != removePlayer->getId())
				unRegisterPlayerFromContainer((*i), removePlayer);	
		}
		else

		//is the object a container?? do we need to despawn the content and unregister it ?
		//just dont unregister us for ourselves or our equipment - we likely only travel
		if(otherPlayer)
		{
			if(removeObject->checkRegisteredWatchers(otherPlayer) )
			{
				unRegisterPlayerFromContainer(removeObject, otherPlayer);	
			}

			gMessageLib->sendDestroyObject(removeObject->getId(),otherPlayer);
		}
	}

	PlayerObjectSet* knownPlayers = removeObject->getRegisteredWatchers();
	PlayerObjectSet::iterator it = knownPlayers->begin();
		
	while(it != knownPlayers->end())
	{
		
		//the only registration a player is still supposed to have at this point is himself and his inventory
		PlayerObject* player = dynamic_cast<PlayerObject*>(*it);
		if(player->getId() != removeObject->getId())
		{
			//we shouldnt get here
			assert(false);
			//unRegisterPlayerFromContainer invalidates the knownObject / knownPlayer iterator
			unRegisterPlayerFromContainer(removeObject, player);	
			gMessageLib->sendDestroyObject(removeObject->getId(),player);
			it = knownPlayers->begin();
		}
		else
			it++;
	}

	ObjectSet* knownObjects = removeObject->getRegisteredContainers();
	ObjectSet::iterator objectIt = knownObjects->begin();
	PlayerObject* player = dynamic_cast<PlayerObject*> (removeObject);

	// no need to unregister stuff from stuff
	if(!player)
	{
		return;
	}

	//the only registration a player is still supposed to have at this point is himself and his inventory and equipped stuff
	while(objectIt != knownObjects->end())
	{
		
		//create it for the registered Players
		
		Object* object = dynamic_cast<Object*>(*objectIt);
		if(object)
		{
			//unRegisterPlayerFromContainer invalidates the knownObject / knownPlayer iterator
			unRegisterPlayerFromContainer(object, player);	
			gMessageLib->sendDestroyObject(object->getId(),player);
			objectIt = knownObjects->begin();
		}
		else	
			objectIt++;
	}
}

// when a player leaves a structure we need to either delete all items in the structure directly
// or cache them until the structure gets deleted

void SpatialIndexManager::removePlayerFromStructure(PlayerObject* player, CellObject* cell)
{
	BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));
	if(building)
	{
		//for now delete the content directly
		removeStructureItemsForPlayer(player, building);
	}
	
}

// removes all structures items
void SpatialIndexManager::removeStructureItemsForPlayer(PlayerObject* player, BuildingObject* building)
{
	ObjectList cellObjects		= building->getAllCellChilds();
	ObjectList::iterator objIt	= cellObjects.begin();

	while(objIt != cellObjects.end())
	{
		Object* object = (*objIt);

		if(PlayerObject* otherPlayer = dynamic_cast<PlayerObject*>(object))
		{
			//do nothing players and creatures are always in the grid

		}
		else
		if(CreatureObject* pet = dynamic_cast<CreatureObject*>(object))
		{
			//do nothing players and creatures are always in the grid

		}
		else
		{
			if((object) && (object->checkRegisteredWatchers(player)))
			{
				unRegisterPlayerFromContainer(object,player);	
			}

			//destroy item for the player
			gMessageLib->sendDestroyObject(object->getId(),player);

			
		}
		objIt++;
	}


}


// removes an item from a structure
void SpatialIndexManager::removeObjectFromBuilding(Object* object, BuildingObject* building)
{

	ObjectList cellObjects		= building->getAllCellChilds();
	ObjectList::iterator objIt	= cellObjects.begin();

	while(objIt != cellObjects.end())
	{
		Object* obj = (*objIt);

		if(object == obj)
		{

			// get player list
			ObjectList cellPlayers			= building->getAllCellChilds();
			ObjectList::iterator playerIt	= cellPlayers.begin();

			while(playerIt != cellPlayers.end())
			{
				PlayerObject* player = dynamic_cast<PlayerObject*>(*playerIt);
				
				if(player)
				{
					if((object) && (object->checkRegisteredWatchers(player)))
					{
						unRegisterPlayerFromContainer(object,player);	
					}

					//destroy item for the player
					gMessageLib->sendDestroyObject(object->getId(),player);
				}

				playerIt++;
			}

			cellObjects.erase(objIt++);
			return;
			
		}

		objIt++;
	}


}

//============================================================================
//destroy an object if its not us
void SpatialIndexManager::CheckObjectIterationForDestruction(Object* toBeTested, Object* toBeUpdated)
{
	PlayerObject* updatedPlayer = dynamic_cast<PlayerObject*>(toBeUpdated);

	if(updatedPlayer && (toBeTested->getId() != updatedPlayer->getId()))
	{
		if(toBeTested->checkRegisteredWatchers(updatedPlayer))
		{
			unRegisterPlayerFromContainer(toBeTested,updatedPlayer);	
		}

		//we (updateObject) got out of range of the following (*i) objects
		//destroy them for us
		gMessageLib->sendDestroyObject(toBeTested->getId(),updatedPlayer);
	}

	//if its a player, destroy us for him
	PlayerObject* them = dynamic_cast<PlayerObject*> (toBeTested);
	if(them)
	{
		
		unRegisterPlayerFromContainer(toBeUpdated,them);	
		

		gMessageLib->sendDestroyObject(toBeUpdated->getId(),them);
			
	}
}


//=================================================================================
//destroy objects out of our range when moving for ONE GRID
//do not use when teleporting
//
void SpatialIndexManager::UpdateBackCells(Object* updateObject, uint32 oldCell)
{
	
	uint32 newCell = updateObject->getGridBucket();

	//ZMAP Northbound! 
	if((oldCell + GRIDWIDTH) == newCell)
	{
		ObjectListType FinalList;
		ObjectListType::iterator it;
		
		getGrid()->GetGridContentsListRow(oldCell - (GRIDWIDTH*VIEWRANGE), &FinalList, 0xffffffff);

		for(ObjectListType::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);			
		}	

		return;
	}

	//ZMAP Southbound! 
	else if((oldCell - GRIDWIDTH) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;
		
		getGrid()->GetGridContentsListRow(oldCell + (GRIDWIDTH*VIEWRANGE), &FinalList, 0xffffffff);

		for(ObjectListType::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);			
		}

		return;
	}

	//ZMAP Westbound! 
	else if((oldCell - 1) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetGridContentsListRow(oldCell + VIEWRANGE, &FinalList, 0xffffffff);
		
		for(ObjectListType::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);
		}

		return;
	}

	//Eastbound! 
	else if((oldCell + 1) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetGridContentsListColumn(oldCell - VIEWRANGE, &FinalList, 0xffffffff);
		

		for(ObjectListType::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);
		}

		return;
	}

	// NorthEastbound
	else if((oldCell + (GRIDWIDTH+1)) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetCellContents(oldCell - (GRIDWIDTH+1)*VIEWRANGE, &FinalList, 0xffffffff);

		getGrid()->GetGridContentsListColumnUp(oldCell - ((GRIDWIDTH+1)*VIEWRANGE) +GRIDWIDTH, &FinalList, 0xffffffff);

		getGrid()->GetGridContentsListRowRight(oldCell - ((GRIDWIDTH+1)*VIEWRANGE) + 1, &FinalList, 0xffffffff);//

		for(ObjectListType::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);
		}

		return;
	}

	// NorthWestbound -> up left
	else if((oldCell + GRIDWIDTH-1) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		//so we need to delete down right (Southeast)
		getGrid()->GetCellContents(oldCell - ((GRIDWIDTH-1)*VIEWRANGE), &FinalList, 0xffffffff);

		getGrid()->GetGridContentsListColumnUp(oldCell - ((GRIDWIDTH-1)*VIEWRANGE) +GRIDWIDTH, &FinalList, 0xffffffff);

		getGrid()->GetGridContentsListRowLeft(oldCell - ((GRIDWIDTH-1)*VIEWRANGE)-1, &FinalList, 0xffffffff);//

		for(ObjectListType::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);	
		}

		return;
	}

		// SouthWestbound	  -> down left
	else if((oldCell - (GRIDWIDTH+1)) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		//so we need to delete up right (Northeast)
		getGrid()->GetCellContents(oldCell + (GRIDWIDTH+1)*VIEWRANGE, &FinalList, 0xffffffff);

		//get the column down 
		getGrid()->GetGridContentsListColumnDown(oldCell + ((GRIDWIDTH+1)*VIEWRANGE) - GRIDWIDTH, &FinalList, 0xffffffff);

		//get the row 
		getGrid()->GetGridContentsListRowLeft(oldCell + ((GRIDWIDTH+1)*VIEWRANGE) -1, &FinalList, 0xffffffff);//

		for(ObjectListType::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);
		}

		return;
	}

		// SouthEastbound	-> down right
	else if((oldCell - (GRIDWIDTH-1)) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		//so we need to delete up left (Northwest)
		getGrid()->GetCellContents(oldCell + ((GRIDWIDTH-1)*VIEWRANGE), &FinalList, 0xffffffff);

		getGrid()->GetGridContentsListColumnDown(oldCell + ((GRIDWIDTH-1)*VIEWRANGE) -GRIDWIDTH, &FinalList, 0xffffffff);

		getGrid()->GetGridContentsListRowRight(oldCell + ((GRIDWIDTH-1)*VIEWRANGE) +1, &FinalList, 0xffffffff);//

		for(ObjectListType::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);
		}

		return;
	}
	
}



//=============================================================================================
//collect Objects in the new cells
void SpatialIndexManager::UpdateFrontCells(Object* updateObject, uint32 oldCell)
{

	uint32 newCell = updateObject->getGridBucket();

	//ZMAP Northbound!
	if((oldCell + GRIDWIDTH) == newCell)
	{
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetGridContentsListRow((updateObject->getGridBucket() + (GRIDWIDTH*VIEWRANGE)) + GRIDWIDTH, &FinalList, 0xffffffff);
		
		ObjectCreationIteration(&FinalList,updateObject);
		
		return;
	}

	//ZMAP Southbound! 
	else if((oldCell - GRIDWIDTH) == newCell)
	{
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetGridContentsListRow((updateObject->getGridBucket() - (GRIDWIDTH*VIEWRANGE)) - GRIDWIDTH, &FinalList, 0xffffffff);
		
		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

			//ZMAP Eastbound! 
	else if((oldCell + 1) == newCell)
	{
	
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetGridContentsListColumn((updateObject->getGridBucket() + VIEWRANGE) + 1, &FinalList, 0xffffffff);

		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

		//ZMAP Westbound! 
	else if((oldCell - 1) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetGridContentsListColumn((updateObject->getGridBucket() - VIEWRANGE) - 1, &FinalList, 0xffffffff);

		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

	// NorthEastbound
	else if((oldCell + (GRIDWIDTH+1)) == newCell)
	{
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetCellContents((updateObject->getGridBucket() + ((GRIDWIDTH+1)*VIEWRANGE)) + (GRIDWIDTH+1), &FinalList, 0xffffffff);//

		getGrid()->GetGridContentsListColumnDown((updateObject->getGridBucket() + ((GRIDWIDTH+1)*VIEWRANGE)) - GRIDWIDTH, &FinalList, 0xffffffff);//
		
		getGrid()->GetGridContentsListRowLeft((updateObject->getGridBucket() + ((GRIDWIDTH+1)*VIEWRANGE)) - 1, &FinalList, 0xffffffff);//
		
		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

	// NorthWestbound
	else if((oldCell + (GRIDWIDTH-1)) == newCell)
	{
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetCellContents((updateObject->getGridBucket() + ((GRIDWIDTH-1)*VIEWRANGE)) + (GRIDWIDTH-1), &FinalList, 0xffffffff);//
		
		getGrid()->GetGridContentsListColumnDown((updateObject->getGridBucket() + ((GRIDWIDTH-1)*VIEWRANGE))  + (GRIDWIDTH-1) - GRIDWIDTH, &FinalList, 0xffffffff);//

		getGrid()->GetGridContentsListRowRight((updateObject->getGridBucket() + ((GRIDWIDTH-1)*VIEWRANGE))  + (GRIDWIDTH-1) + 1, &FinalList, 0xffffffff);//
	
		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

		// SouthWestbound
	else if((oldCell - (GRIDWIDTH+1)) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetCellContents((updateObject->getGridBucket() - (GRIDWIDTH+1)*VIEWRANGE) - (GRIDWIDTH+1), &FinalList, 0xffffffff);

		getGrid()->GetGridContentsListColumnUp((updateObject->getGridBucket() - ((GRIDWIDTH+1)*VIEWRANGE)) - (GRIDWIDTH+1) + GRIDWIDTH, &FinalList, 0xffffffff);

		getGrid()->GetGridContentsListRowRight((updateObject->getGridBucket() + ((GRIDWIDTH+1)*VIEWRANGE)) - (GRIDWIDTH+1) + 1, &FinalList, 0xffffffff);//

		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

		// SouthEastbound
	else if((oldCell - (GRIDWIDTH-1)) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetCellContents((updateObject->getGridBucket() - (GRIDWIDTH-1)*VIEWRANGE) - (GRIDWIDTH-1), &FinalList, 0xffffffff);

		getGrid()->GetGridContentsListColumnUp((updateObject->getGridBucket() - ((GRIDWIDTH-1)*VIEWRANGE)) - (GRIDWIDTH-1) + GRIDWIDTH, &FinalList, 0xffffffff);

		getGrid()->GetGridContentsListRowLeft((updateObject->getGridBucket() + ((GRIDWIDTH-1)*VIEWRANGE)) - (GRIDWIDTH-1) - 1, &FinalList, 0xffffffff);//

		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}
}

void SpatialIndexManager::ObjectCreationIteration(std::list<Object*>* FinalList, Object* updateObject)
{
	//at some point we need to throttle ObjectCreates!!!
	//one possibility would be to only send one grid at a time and keep track of up / unup dated Grids

	for(std::list<Object*>::iterator i = FinalList->begin(); i != FinalList->end(); i++)
	{
		CheckObjectIterationForCreation((*i),updateObject);
	}
}

void SpatialIndexManager::CheckObjectIterationForCreation(Object* toBeTested, Object* updatedObject)
{
	PlayerObject* updatedPlayer = dynamic_cast<PlayerObject*>(updatedObject);

	if(toBeTested->getId() != updatedObject->getId())
	{
		//we are a player and need to create the following object for us
		if(updatedPlayer)
		{
			sendCreateObject(toBeTested,updatedPlayer,false);
			registerPlayerToContainer(toBeTested, updatedPlayer);
			
			//if it is a house we need to register the cells for watching *only* when we enter
		}
				
		PlayerObject* testedPlayer = dynamic_cast<PlayerObject*> (toBeTested);
		if(testedPlayer)
		{
			sendCreateObject(updatedObject,testedPlayer,false);

			//if we and it are players we need to register each other for watching our equipment
			if(updatedPlayer)
			{
				registerPlayerToContainer(testedPlayer, updatedPlayer);
				registerPlayerToContainer(updatedPlayer, testedPlayer);
			}
		}

		//is it a player register him so we get to see his equipment
	}
}

//======================================================================================================================
//
// creates a creature in the world 
// this means to find whether it is in the si or in a container (cell)
// then add it to the container and update the listeners
// 

//we need to listen to ourselves when the equiplist/inventory is updated
//please note the pecularities as items on the equiplist are not part of the objectcontainer

void SpatialIndexManager::createInWorld(CreatureObject* creature)
{
	//are we in the SI ???
	if(creature->getParentId() == 0)
	{
		//just create in the SI - it will keep track of nearby players
		this->AddObject(creature);
		return;
	}

	//then very likely in a cell
	CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(creature->getParentId()));
	if(cell)
	{
		BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));
		
		if(building)
		{
			//add the Creature to the cell
			cell->addObjectSecure(creature);

			//add it to the world
			this->AddObject(creature);
			
		}
	}
}


void SpatialIndexManager::createInWorld(PlayerObject* player)
{

	//are we in the SI ???
	if(player->getParentId() == 0)
	{
		//just create in the SI - it will keep track of nearby players
		this->AddObject(player);
		return;
	}

	//then very likely in a cell
	CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
	if(cell)
	{
		BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));
		
		if(building)
		{
			//add the Creature to the cell
			cell->addObjectSecure(player);

			//add it to the world
			this->AddObject(player);
				
			//iterate through all the cells and add the player as listener
			
			CellObjectList::iterator cellIt = building->getCellList()->begin();

			while(cellIt != building->getCellList()->end())
			{
				this->registerPlayerToContainer((*cellIt),player);
				++cellIt;
			}			
		}
	}
}


//======================================================================================================================
//
// creates an object in the world 
// this means to find whether it is in the si or in a container
// then add it to the container and update the listeners
// 

void SpatialIndexManager::createInWorld(Object* object)
{


	uint32 baseCell = 0xffffffff;

	//get parent object determine who to send creates for

	if(object->getParentId() == 0)
	{
		//just create in the SI - it will keep track of nearby players
		this->AddObject(object);
		return;
	}

	Object* parent = gWorldManager->getObjectById(object->getParentId());

	//start with equipped items
	PlayerObject* player = dynamic_cast<PlayerObject*>(parent);
	if(player)
	{
		//add to equiplist manually yet we dont use the objectcontainer for equipped items yet
		player->getEquipManager()->addEquippedObject(object);
		//createObjectToRegisteredPlayers(player, object);// no ... ! create it only for the owner
		sendCreateObject(object,player,false);
		updateEquipListToRegisteredPlayers(player);
		return;
	}

	//please note that individual cells have players as listeners, not the entire building
	//items in cells
	CellObject* cell = dynamic_cast<CellObject*>(parent);
	if(cell)
	{
		createObjectToRegisteredPlayers(parent, object);
		return;
	}	
	
	if(parent)
	{
		//items in containers
		createObjectToRegisteredPlayers(parent, object);
		return;
	}	

	//need to implement container watch list in containerObject
}

//======================================================================================================================
// when creating a player and the player is in a cell we need to create all the cells contents for the player
// cellcontent is *NOT* in the grid

// interisting point to find out 
// will we destroy all building content always on leaving or will we have (like containers) a list with players
// knowing the content and destroy it when getting out of range ??

void SpatialIndexManager::initObjectsInRange(PlayerObject* playerObject)
{
	
	CellObject*			cell ;

	//if we are in a cell create the buildings content
	//we just moved inside

	//what about caves ?
	if((playerObject->getParentId() != 0) && (cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(playerObject->getParentId()))))
	{
		
		BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));
		if(building)
		{
			ObjectList cellChilds = building->getAllCellChilds();
			ObjectList::iterator cellChildsIt = cellChilds.begin();

			while(cellChildsIt != cellChilds.end())
			{
		
				//create the tangibles now
				TangibleObject* cellTangible = dynamic_cast<TangibleObject*>(*cellChildsIt);
				if(cellTangible)
				{
					sendCreateObject(cellTangible, playerObject, true);
				}

				++cellChildsIt;
			}
	
		}
			
	}

}

//==============================================================================================================
//
// get the Objects in range of the player ie everything in the Object Bucket
// NO players, NO creatures, NO regions
//
void SpatialIndexManager::getObjectsInRange(const Object* const object,ObjectSet* resultSet,uint32 objTypes,float range, bool cellContent)
{
	
	ObjectList 	resultList;
	
	Object*			tmpObject;
	ObjectType		tmpType;
	uint64			objectId = object->getId();

	uint32			CustomRange;

	//see that we do not query more grids than necessary
	CustomRange = (uint32) range / (MAPWIDTH/GRIDWIDTH);
	//CustomRange++;
	if(CustomRange > VIEWRANGE)
	{
		CustomRange = VIEWRANGE;
	}

	if(CustomRange < 2)
	{
		CustomRange = 2;
	}



	glm::vec3   position;

	position = object->getWorldPosition();
	getGrid()->GetCustomRangeCellContents(getGrid()->getCellId(position.x, position.z),CustomRange, &resultList,Bucket_Objects);

	// filter needed objects
	ObjectList::iterator it = resultList.begin();
	while(it != resultList.end())
	{
		// check if its us
		if((*it) != object) 
		{
			tmpObject = (*it);
			tmpType = tmpObject->getType();
		
			// add it
			if((tmpType & objTypes) == static_cast<uint32>(tmpType))
			{
				float rangeTest = glm::distance(object->getWorldPosition(),tmpObject->getWorldPosition());
				if(rangeTest <= range)
				{
					resultSet->insert(tmpObject);
				}
			}
					
			// if its a building, add objects of our types it contains if wished
			if((tmpType == ObjType_Building)&&cellContent)
			{
				// gLogger->log(LogManager::DEBUG,"Found a building");

				ObjectList cellChilds = (dynamic_cast<BuildingObject*>(tmpObject))->getAllCellChilds();
				ObjectList::iterator cellChildsIt = cellChilds.begin();

				while(cellChildsIt != cellChilds.end())
				{
					Object* cellChild = (*cellChildsIt);
					
					tmpType = cellChild->getType();

					if((tmpType & objTypes) == static_cast<uint32>(tmpType))
					{
						TangibleObject* tangible = dynamic_cast<TangibleObject*>(cellChild);
						glm::vec3   positionObject = cellChild->getWorldPosition();
						
						float rangeTest = glm::distance(positionObject, position);
						if(rangeTest <= range)
						{
							resultSet->insert(cellChild);
						}
					}

					++cellChildsIt;
									
				}
			}

		}
		
		++it;

	}
	
}

void SpatialIndexManager::getPlayersInRange(const Object* const object,PlayerObjectSet* resultSet, bool cellContent)
{
	//please note that players are always in the grid, even if in a cell!!!!
	
	ObjectList 		resultList;
	
	Object*			tmpObject;

	uint64			objectId = object->getId();

	glm::vec3   position;

	// we in world space, outside -> inside , outside -> outside checking
	if(!object->getParentId())
	{
	
		position = object->mPosition;
	}
	else
	{
		position = object->getWorldPosition(); 
	}

	getGrid()->GetPlayerViewingRangeCellContents(getGrid()->getCellId(position.x, position.z), &resultList);
	
	// filter needed objects
	ObjectList::iterator it = resultList.begin();
	while(it != resultList.end())
	{
		// check if its us
		if((*it) != object) 
		{
			tmpObject = (*it);

			if((!tmpObject->getParentId())||(cellContent == true))
			{
				PlayerObject* player = dynamic_cast<PlayerObject*>(tmpObject);
				resultSet->insert(player);
			}

			
		}

		++it;
	}
}


void SpatialIndexManager::sendToPlayersInRange(const Object* const object, bool cellContent, std::function<void (PlayerObject* player)> callback)
{
	
	PlayerObjectSet		inRangePlayers;
	gSpatialIndexManager->getPlayersInRange(object,&inRangePlayers,cellContent);

	PlayerObjectSet::const_iterator	it				= inRangePlayers.begin();

	while(it != inRangePlayers.end())
	{
		PlayerObject* targetObject = (*it);

		if(targetObject->isConnected())
		{
			callback(targetObject);
		}

		++it;
	}


}

//=================================================================================================================================================
// registers a containers content as known to a player
// a container can be a backpack placed in a cell or a cell itself
// registering the container just means that we know the content
// there are containers we register to automatically (like players)
// containers we register to when we enter them (buildings)
// and containers we register to when we look inside them (containers in the maincell)
// to register to a container we need to have the proper permissions

void SpatialIndexManager::registerPlayerToContainer(Object* container,PlayerObject* player)
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
			sendCreateObject(tO,player,false);
			registerPlayerToContainer(tO,player);
		}
		
		it++;
	}
	
}

//===============================================================================================================================================================================
//registering a player to a building is different, as the cells of a building must be known at all time to a player (as long as we know the building)
//the cellcontent is not loaded on building creation
//cellcontent gets loaded once we enter (registered to) the building 

void SpatialIndexManager::registerPlayerToBuilding(BuildingObject* building,PlayerObject* player)
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
void SpatialIndexManager::unRegisterPlayerFromBuilding(BuildingObject* building,PlayerObject* player)
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

//===============================================================================================================================================================================
// UnRegisters a container to a player
// a container can be a backpack placed in a cell (or a cell itself) - unregistering it means that the content is not known to us anylonger
// the container itself might (but must not necessarily) still be known afterwards
// we will only destroy the contents for the client however, not the container and unregister the player
// unRegisterPlayerFromContainer *invalidates* the knownObject / knownPlayer iterator
void SpatialIndexManager::unRegisterPlayerFromContainer(Object* container,PlayerObject* player)
{
	//are we sure the player doesnt know the container already ???
	if(!container->checkRegisteredWatchers(player))
	{

		gLogger->log(LogManager::DEBUG,"SpatialIndexManager::UnRegisterPlayerToContainer :: Container %I64u not known to player %I64u",container->getId(),player->getId());
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
		TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById((*it)));
		if(!tO)
		{
			//assert(false && "SpatialIndexManager::registerPlayerToContainer ::unable to find tangible object (container content)");
		}

		unRegisterPlayerFromContainer(tO,player);
		it++;
	}
	
}

void SpatialIndexManager::createObjectToRegisteredPlayers(Object* container,Object* object)
{
	//creatures players entering a cell get here, too
	//they are however already members of the grid so just ignore them
	if(CreatureObject* creature = dynamic_cast<CreatureObject*>(object))
	{
		return;
	}

	gSpatialIndexManager->sendToRegisteredPlayers(container,[object, this] (PlayerObject* recipient) 
		{
			sendCreateObject(object,recipient,false);
		
			//the registered object likely is a container in itself
			registerPlayerToContainer(object,recipient);
		}
	);
	
}

//=======================================================================
// player / creature equipped something - update all registered listeners with the changed equiplist
void SpatialIndexManager::updateEquipListToRegisteredPlayers(PlayerObject* player)
{

	sendToRegisteredPlayers(player,[player](PlayerObject* recepient)
		{
			gMessageLib->sendEquippedListUpdate(player, recepient);
		}
	);
}


// sends given function to all of the containers registered watchers
void SpatialIndexManager::sendToRegisteredPlayers(Object* container, std::function<void (PlayerObject* player)> callback)
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
}

// sends given function to all of the containers registered watchers
void SpatialIndexManager::sendToGroupedRegisteredPlayers(CreatureObject* container, std::function<void (PlayerObject* player)> callback, bool self)
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
void SpatialIndexManager::destroyObjectToRegisteredPlayers(Object* container,uint64 object, bool destroyForSelf)
{
	gSpatialIndexManager->sendToRegisteredPlayers(container,[container, object, this, destroyForSelf] (PlayerObject* recipient) 
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
void SpatialIndexManager::updateObjectPlayerRegistrations(Object* newContainer, Object* oldContainer, Object* object, uint32 containment)
{

	gLogger->log(LogManager::DEBUG,"SpatialIndexManager::updateObjectPlayerRegistrations :: newContainer %I64u oldContainer %I64u - update old player set",newContainer->getId(),oldContainer->getId());
	//destroy for old Players that are NOT on the new list
	gSpatialIndexManager->sendToRegisteredPlayers(oldContainer,[oldContainer, object, this, newContainer, containment] (PlayerObject* recipient) 
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
	gSpatialIndexManager->sendToRegisteredPlayers(newContainer,[object, this, oldContainer] (PlayerObject* recipient) 
		{
			if(!oldContainer->checkRegisteredWatchers(recipient))
			{
				gLogger->log(LogManager::DEBUG,"SpatialIndexManager::updateObjectPlayerRegistrations :: player %I64u added - create",recipient->getId());
				sendCreateObject(object,recipient,false);
		
				//the registered object likely is a container in itself
				registerPlayerToContainer(object,recipient);
			}
		}
	);
	
	
}

//============================================================================================================
// the idea is that the container holding our new item might be held by a container, too
// should this happen, we need to find the main container to determin what kind of creates to send to our player/s
// we will iterate through the parentObjects until the parent is either a player (item has been equipped) or in the inventory or )
// or a cell or a factory
uint64 SpatialIndexManager::getObjectMainParent(Object* object)
{

	uint64 parentID = object->getParentId();

	// hack ourselves a player - it is not possible to get an inventory otherwise because
	// inventories are not part of the WorldObjectMap ... which really gets cumbersom (resolved with newest trunc)

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