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
	
	uint32 finalBucket = getGrid()->AddObject(newObject);

	//DLOG(INFO) << "SpatialIndexManager::AddObject :: Object " << newObject->getId() << " added to bucket " <<  finalBucket;
	
	//any errors ?
	if(finalBucket == 0xffffffff)
	{
		DLOG(INFO) << "SpatialIndexManager::AddObject :: Object " << newObject->getId() << " could not be added to the bucket because the bucket was invalid " <<  finalBucket;
		return false;
	}

	//get all Players in range
	//and register as necessary
	
	ObjectListType playerList;
	getGrid()->GetPlayerViewingRangeCellContents(finalBucket, &playerList);
	
	for(ObjectListType::iterator i = playerList.begin(); i != playerList.end(); i++)
	{
		PlayerObject* otherPlayer = dynamic_cast<PlayerObject*>((*i));
		sendCreateObject(newObject,otherPlayer, false);
			
		if((newObject->getType() == ObjType_Creature) || (newObject->getType() == ObjType_NPC))
		{			
			//DLOG(INFO) << "SpatialIndexManager::AddObject:: register : " << newObject->getId() << " to player ; " << otherPlayer->getId();
			gContainerManager->registerPlayerToContainer(newObject, otherPlayer);
		}
	}

	return true;
}

bool SpatialIndexManager::AddObject(PlayerObject *player)
{
	//Pesudo
	// 1. Calculate CellID
	// 2. Set CellID
	// 3. Insert object into the cell in the hash table

	uint32 finalBucket = getGrid()->AddObject(player);
	
	DLOG(INFO) << "SpatialIndexManager::AddObject :: Object " << player->getId() << " added to bucket " <<  finalBucket;
	
	//any errors ?
	if(finalBucket == 0xffffffff)
	{
		DLOG(INFO) << "SpatialIndexManager::AddObject :: Object " << player->getId() << " could not be added to the bucket because the bucket was invalid " <<  finalBucket;
		return false;
	}

	//now create it for everyone around
	//and around for it
		
	ObjectListType playerList;
	getGrid()->GetViewingRangeCellContents(finalBucket, &playerList,(Bucket_Creatures|Bucket_Objects|Bucket_Players));
		
	for(ObjectListType::iterator i = playerList.begin(); i != playerList.end(); i++)
	{
		//we just added ourselves to the grid - dont send a create to ourselves
		if(((*i)->getId() == player->getId()))
		{
			continue;
		}
		sendCreateObject((*i), player, false);

		if(((*i)->getType() == ObjType_Creature) || ((*i)->getType() == ObjType_NPC))
		{
			gContainerManager->registerPlayerToContainer((*i), player);
			continue;
		}
		if((*i)->getType() == ObjType_Player) 
		{
			PlayerObject* otherPlayer = dynamic_cast<PlayerObject*>(*i);
			sendCreateObject(player, otherPlayer, false);

			gContainerManager->registerPlayerToContainer(otherPlayer, player);
		}

	}
	return true;
}

void SpatialIndexManager::UpdateObject(Object *updateObject)
{

	uint32 oldBucket = updateObject->getGridBucket();
	
	uint32 newBucket = getGrid()->getCellId(updateObject->getWorldPosition().x, updateObject->getWorldPosition().z);
	
	//now  process the spatial index update
	if(newBucket != oldBucket)
	{
		//DLOG(INFO) << "ContainerManager::UpdateObject :: " << updateObject->getId() <<"normal movement from bucket" << oldBucket << " to bucket" << updateObject->getGridBucket();
		
		//test how much we moved if only one grid proceed normally
		if(	(newBucket == (oldBucket +1))			  || (newBucket == (oldBucket -1))				||
			(newBucket == (oldBucket + GRIDWIDTH))	  || (newBucket == (oldBucket - GRIDWIDTH))		||
			(newBucket == (oldBucket + GRIDWIDTH +1)) || (newBucket == (oldBucket + GRIDWIDTH -1))	||
			(newBucket == (oldBucket - GRIDWIDTH +1)) || (newBucket == (oldBucket - GRIDWIDTH -1))
			)
		{
			/*
			PlayerObject* player = dynamic_cast<PlayerObject*>(updateObject);
			if(player)
			{
				DLOG(INFO) << "ContainerManager::UpdateObject :: " << updateObject->getId() <<"moved from bucket" << oldBucket << " to bucket" << updateObject->getGridBucket();
			}
	*/

			//sets the new gridcell, updates subcells
			getGrid()->UpdateObject(updateObject);

			//remove us from the row we left
			UpdateBackCells(updateObject,oldBucket);
			
			//create us for the row in which direction we moved
			UpdateFrontCells(updateObject,oldBucket);
		}
		else //we teleported destroy all and create everything new
		{
			//DLOG(INFO) << "ContainerManager::UpdateObject :: " << updateObject->getId() <<"teleportation from bucket" << oldBucket << " to bucket" << updateObject->getGridBucket();

			//remove us from everything
			RemoveObject(updateObject);

			//sets the new gridcell, updates subcells
			//getGrid()->UpdateObject(updateObject);
			getGrid()->AddObject(updateObject);
			
			//and add us freshly to the world
			AddObject(updateObject);
		}
	
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
	DLOG(INFO) << "SpatialIndexManager::RemoveObjectFromWorld:: : " << removeObject->getId();
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
				//gContainerManager->SendDestroyEquippedObject(removeObject);

				//gContainerManager->destroyObjectToRegisteredPlayers(container, removeObject);
				
				//Bailout - the reason we use SendDestroyEquippedObject(object); instead of the (faster) destroyObjectToRegisteredPlayers
				//is that creatures do not get registered to players as containers (yet) - that might be an idea to change
				//return;

			}

			gContainerManager->destroyObjectToRegisteredPlayers(container,removeObject->getId());

			//remove the object out of the container
			container->removeObject(removeObject);
		}	
		
		//no need to remove a tangible(!) from the grid if it was in a cell
		return;
	}

	//remove it out of the grid
	RemoveObject(removeObject);
}
 
//*********************************************************************
//a Player or creature is ALWAYS in the grid and possibly in a cell
void SpatialIndexManager::RemoveObjectFromWorld(PlayerObject *removePlayer)
{
	//DLOG(INFO) << "SpatialIndexManager::RemoveObjectFromWorld:: : " << removePlayer->getId();

	//remove us from the grid
	RemoveObject(removePlayer);

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
			gContainerManager->unRegisterPlayerFromBuilding(building,removePlayer);
		}

		cell->removeObject(removePlayer);

	}
	else
	{
		DLOG(INFO) << "SpatialIndexManager::RemovePlayerFromWorld: couldn't find cell " << removePlayer->getParentId();
	}


}

void SpatialIndexManager::RemoveObjectFromWorld(CreatureObject *removeCreature)
{
	//DLOG(INFO) << "SpatialIndexManager::RemoveObjectFromWorld:: : " << removeCreature->getId();

	//remove us from the grid
	RemoveObject(removeCreature);

	//remove us out of the cell
	Object* container = gWorldManager->getObjectById(removeCreature->getParentId());
	if(container)
	{
		gContainerManager->destroyObjectToRegisteredPlayers(container,removeCreature->getId());

		//remove the object out of the container
		container->removeObject(removeCreature);
	}


}


//===========================================================================
// this will remove an object from the grid
// if the object is a container with listeners, the listeners will be unregistered
// SpatialIndexManager::UpdateObject calls us with updateGrid = false in case we teleported
// we have updated the grid in that case already
void SpatialIndexManager::RemoveObject(Object *removeObject)
{

	uint32 bucket = removeObject->getGridBucket();

	DLOG(INFO) << "SpatialIndexManager::RemoveObject:: : " << removeObject->getId() << "out of Bucket : " << bucket;

	//remove out of grid lists
	getGrid()->RemoveObject(removeObject);
	
	//get the Objects and unregister as necessary
	ObjectListType playerList;
	//getGrid()->GetPlayerViewingRangeCellContents(gridCell, &playerList);//cell means gridcell here
	getGrid()->GetViewingRangeCellContents(bucket, &playerList,(Bucket_Creatures|Bucket_Objects|Bucket_Players));
	
	PlayerObject* removePlayer = dynamic_cast<PlayerObject*>(removeObject);

	for(ObjectListType::iterator i = playerList.begin(); i != playerList.end(); i++)
	{
		PlayerObject* otherPlayer = dynamic_cast<PlayerObject*>((*i));

		//if we are a player unregister us from everything around
		if(removePlayer)
		{
			if((*i)->getId() != removePlayer->getId())
			{
				gContainerManager->unRegisterPlayerFromContainer((*i), removePlayer);	
				gMessageLib->sendDestroyObject((*i)->getId(), removePlayer);

				if(otherPlayer)
				{
					gMessageLib->sendDestroyObject(removeObject->getId(),otherPlayer);
				}

			}
		}
		else

		//is the object a container?? do we need to despawn the content and unregister it ?
		//just dont unregister us for ourselves or our equipment - we likely only travel
		if(otherPlayer)
		{
			//DLOG(INFO) << "SpatialIndexManager::RemoveObject:: try unregister : " << removeObject->getId() << " from player ; " << otherPlayer->getId();
			
			if(removeObject->checkRegisteredWatchers(otherPlayer) )
			{
				DLOG(INFO) << "SpatialIndexManager::RemoveObject:: unregister : " << removeObject->getId() << " from player ; " << otherPlayer->getId();
				gContainerManager->unRegisterPlayerFromContainer(removeObject, otherPlayer);	
			}
			else
			{
				DLOG(INFO) << "SpatialIndexManager::RemoveObject:: failed unregister : " << removeObject->getId() << " from player ; " << otherPlayer->getId() << " not registered";
			}

			gMessageLib->sendDestroyObject(removeObject->getId(),otherPlayer);
		}
	}

	PlayerObjectSet* knownPlayers	= removeObject->getRegisteredWatchers();
	PlayerObjectSet::iterator it	= knownPlayers->begin();
		
	while(it != knownPlayers->end())
	{
		
		//the only registration a player is still supposed to have at this point is himself and his inventory
		PlayerObject* const player = dynamic_cast<PlayerObject*>(*it);
		if(player->getId() != removeObject->getId())
		{
			//we shouldnt get here
			assert(false);
			//unRegisterPlayerFromContainer invalidates the knownObject / knownPlayer iterator
			gContainerManager->unRegisterPlayerFromContainer(removeObject, player);	
			gMessageLib->sendDestroyObject(removeObject->getId(),player);
			it = knownPlayers->begin();
		}
		else
			it++;
	}

	// no need to unregister the content of knownObjects, as thats (hopefully) just the content of our inventory
	
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
	DLOG(INFO) << "SpatialIndexManager::removeStructureItemsForPlayer:: : " << player->getId();

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
				gContainerManager->unRegisterPlayerFromContainer(object,player);	
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
	DLOG(INFO) << "SpatialIndexManager::removeObjectFromBuilding:: : " << object->getId();

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
						gContainerManager->unRegisterPlayerFromContainer(object,player);	
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
// unregister containers
void SpatialIndexManager::CheckObjectIterationForDestruction(Object* toBeTested, Object* toBeUpdated)
{
	//DLOG(INFO) << "SpatialIndexManager::CheckObjectIterationForDestruction (Object) :: check : " <<toBeTested->getId() << " to be removed from " << toBeUpdated->getId();

	//if its a player, destroy us for him
	if(toBeTested->getType() == ObjType_Player)
	{
		PlayerObject* testedPlayer = dynamic_cast<PlayerObject*> (toBeTested);
		gMessageLib->sendDestroyObject(toBeUpdated->getId(),testedPlayer);		
		gContainerManager->unRegisterPlayerFromContainer(toBeUpdated,testedPlayer);	
	}
}


void SpatialIndexManager::CheckObjectIterationForDestruction(Object* toBeTested, PlayerObject* updatedPlayer)
{
	//DLOG(INFO) << "SpatialIndexManager::CheckObjectIterationForDestruction (Player) :: check : " <<toBeTested->getId() << " to be removed from " << toBeUpdated->getId();

	if((toBeTested->getType() == ObjType_Creature || toBeTested->getType() == ObjType_NPC) && toBeTested->checkRegisteredWatchers(updatedPlayer))
	{
		gContainerManager->unRegisterPlayerFromContainer(toBeTested,updatedPlayer);	
	}

	//we (updateObject) got out of range of the following (*i) objects
	//destroy them for us
	gMessageLib->sendDestroyObject(toBeTested->getId(),updatedPlayer);

	//if its a player, destroy us for him
	if(toBeTested->getType() == ObjType_Player)
	{
		PlayerObject* testedPlayer = dynamic_cast<PlayerObject*> (toBeTested);
		gMessageLib->sendDestroyObject(updatedPlayer->getId(),testedPlayer);		
		gContainerManager->unRegisterPlayerFromContainer(updatedPlayer,testedPlayer);	
	}
}


//=================================================================================
//destroy objects out of our range when moving for ONE GRID
//do not use when teleporting
//

void SpatialIndexManager::UpdateBackCells(Object* updateObject, uint32 oldCell)
{

	uint32 newCell = updateObject->getGridBucket();

	//Update all for players
	uint32 queryType = Bucket_Creatures | Bucket_Objects | Bucket_Players;

	//npcs/ creatures are only interested in updating players
	if(updateObject->getType() != ObjType_Player)
	{
		queryType = Bucket_Players;
	}

	//ZMAP Northbound! 
	if((oldCell + GRIDWIDTH) == newCell)
	{
		ObjectListType FinalList;
		ObjectListType::iterator it;
		
		getGrid()->GetGridContentsListRow(oldCell - (GRIDWIDTH*VIEWRANGE), &FinalList, queryType);

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
		
		getGrid()->GetGridContentsListRow(oldCell + (GRIDWIDTH*VIEWRANGE), &FinalList, queryType);

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

		getGrid()->GetGridContentsListRow(oldCell + VIEWRANGE, &FinalList, queryType);
		
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

		getGrid()->GetGridContentsListColumn(oldCell - VIEWRANGE, &FinalList, queryType);
		

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

		getGrid()->GetCellContents(oldCell - (GRIDWIDTH+1)*VIEWRANGE, &FinalList, queryType);

		getGrid()->GetGridContentsListColumnUp(oldCell - ((GRIDWIDTH+1)*VIEWRANGE) +GRIDWIDTH, &FinalList, queryType);

		getGrid()->GetGridContentsListRowRight(oldCell - ((GRIDWIDTH+1)*VIEWRANGE) + 1, &FinalList, queryType);//

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
		getGrid()->GetCellContents(oldCell - ((GRIDWIDTH-1)*VIEWRANGE), &FinalList, queryType);

		getGrid()->GetGridContentsListColumnUp(oldCell - ((GRIDWIDTH-1)*VIEWRANGE) +GRIDWIDTH, &FinalList, queryType);

		getGrid()->GetGridContentsListRowLeft(oldCell - ((GRIDWIDTH-1)*VIEWRANGE)-1, &FinalList, queryType);//

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
		getGrid()->GetCellContents(oldCell + (GRIDWIDTH+1)*VIEWRANGE, &FinalList, queryType);

		//get the column down 
		getGrid()->GetGridContentsListColumnDown(oldCell + ((GRIDWIDTH+1)*VIEWRANGE) - GRIDWIDTH, &FinalList, queryType);

		//get the row 
		getGrid()->GetGridContentsListRowLeft(oldCell + ((GRIDWIDTH+1)*VIEWRANGE) -1, &FinalList, queryType);//

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
		getGrid()->GetCellContents(oldCell + ((GRIDWIDTH-1)*VIEWRANGE), &FinalList, queryType);

		getGrid()->GetGridContentsListColumnDown(oldCell + ((GRIDWIDTH-1)*VIEWRANGE) -GRIDWIDTH, &FinalList, queryType);

		getGrid()->GetGridContentsListRowRight(oldCell + ((GRIDWIDTH-1)*VIEWRANGE) +1, &FinalList, queryType);//

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

	//Update all for players
	uint32 queryType = Bucket_Creatures | Bucket_Objects | Bucket_Players;

	//npcs/ creatures are only interested in updating players
	if(updateObject->getType() != ObjType_Player)
	{
		queryType = Bucket_Players;
	}
	
	//uint32 newCell = updateObject->getGridBucket();

	//ZMAP Northbound!
	if((oldCell + GRIDWIDTH) == newCell)
	{
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetGridContentsListRow((updateObject->getGridBucket() + (GRIDWIDTH*VIEWRANGE)) + GRIDWIDTH, &FinalList, queryType);
		
		ObjectCreationIteration(&FinalList,updateObject);
		
		return;
	}

	//ZMAP Southbound! 
	else if((oldCell - GRIDWIDTH) == newCell)
	{
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetGridContentsListRow((updateObject->getGridBucket() - (GRIDWIDTH*VIEWRANGE)) - GRIDWIDTH, &FinalList, queryType);
		
		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

			//ZMAP Eastbound! 
	else if((oldCell + 1) == newCell)
	{
	
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetGridContentsListColumn((updateObject->getGridBucket() + VIEWRANGE) + 1, &FinalList, queryType);

		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

		//ZMAP Westbound! 
	else if((oldCell - 1) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetGridContentsListColumn((updateObject->getGridBucket() - VIEWRANGE) - 1, &FinalList, queryType);

		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

	// NorthEastbound
	else if((oldCell + (GRIDWIDTH+1)) == newCell)
	{
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetCellContents((updateObject->getGridBucket() + ((GRIDWIDTH+1)*VIEWRANGE)) , &FinalList, queryType);//

		getGrid()->GetGridContentsListColumnDown((updateObject->getGridBucket() + ((GRIDWIDTH+1)*VIEWRANGE)) - GRIDWIDTH, &FinalList, queryType);//
		
		getGrid()->GetGridContentsListRowLeft((updateObject->getGridBucket() + ((GRIDWIDTH+1)*VIEWRANGE)) - 1, &FinalList, queryType);//
		
		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

	// NorthWestbound
	else if((oldCell + (GRIDWIDTH-1)) == newCell)
	{
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetCellContents((updateObject->getGridBucket() + ((GRIDWIDTH-1)*VIEWRANGE)) , &FinalList, queryType);//
		
		getGrid()->GetGridContentsListColumnDown((updateObject->getGridBucket() + ((GRIDWIDTH-1)*VIEWRANGE))  - GRIDWIDTH, &FinalList, queryType);//

		getGrid()->GetGridContentsListRowRight((updateObject->getGridBucket() + ((GRIDWIDTH-1)*VIEWRANGE))   + 1, &FinalList, queryType);//
	
		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

		// SouthWestbound
	else if((oldCell - (GRIDWIDTH+1)) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetCellContents((updateObject->getGridBucket() - ((GRIDWIDTH+1)*VIEWRANGE)) , &FinalList, queryType);

		getGrid()->GetGridContentsListColumnUp((updateObject->getGridBucket() - ((GRIDWIDTH+1)*VIEWRANGE))  + GRIDWIDTH, &FinalList, queryType);

		getGrid()->GetGridContentsListRowRight((updateObject->getGridBucket() - ((GRIDWIDTH+1)*VIEWRANGE))  + 1, &FinalList, queryType);//

		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}

		// SouthEastbound
	else if((oldCell - (GRIDWIDTH-1)) == newCell)
	{
		
		ObjectListType FinalList;
		ObjectListType::iterator it;

		getGrid()->GetCellContents((updateObject->getGridBucket() - (GRIDWIDTH-1)*VIEWRANGE) , &FinalList, queryType);

		getGrid()->GetGridContentsListColumnUp((updateObject->getGridBucket() - ((GRIDWIDTH-1)*VIEWRANGE))  + GRIDWIDTH, &FinalList, queryType);

		getGrid()->GetGridContentsListRowLeft((updateObject->getGridBucket() - ((GRIDWIDTH-1)*VIEWRANGE))  - 1, &FinalList, queryType);//

		ObjectCreationIteration(&FinalList,updateObject);

		return;
	}
}

//======================================================================================================
// iterate through all Objects and create them and register them as necessary
void SpatialIndexManager::ObjectCreationIteration(std::list<Object*>* FinalList, Object* updateObject)
{

	for(std::list<Object*>::iterator i = FinalList->begin(); i != FinalList->end(); i++)
	{
		CheckObjectIterationForCreation((*i),updateObject);
	}
}

//================================================================================================
// this is called for players and creatures / NPCs alike
//
void SpatialIndexManager::CheckObjectIterationForCreation(Object* toBeTested, Object* updatedObject)
{
	PlayerObject* updatedPlayer = dynamic_cast<PlayerObject*>(updatedObject);

	if(toBeTested->getId() == updatedObject->getId())
	{
		assert(false);
	}

	//we are a player and need to create the following object for us
	if(updatedPlayer)
	{
		sendCreateObject(toBeTested,updatedPlayer,false);
		
		if(toBeTested->getType() == ObjType_NPC || toBeTested->getType() == ObjType_Creature)
		{
			gContainerManager->registerPlayerToContainer(toBeTested, updatedPlayer);
		}
		if(toBeTested->getType() == ObjType_Player )
		{
			PlayerObject* testedPlayer = dynamic_cast<PlayerObject*> (toBeTested);
			
			gContainerManager->registerPlayerToContainer(updatedObject, testedPlayer);
			sendCreateObject(updatedObject,testedPlayer,false);
		}
		return;
	}
				
	PlayerObject* testedPlayer = dynamic_cast<PlayerObject*> (toBeTested);
	if(testedPlayer)
	{
		sendCreateObject(updatedObject,testedPlayer,false);
		gContainerManager->registerPlayerToContainer(updatedObject, testedPlayer);
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
	this->AddObject(creature);

	//are we in a cell? otherwise bail out
	if(creature->getParentId() == 0)
	{		
		return;
	}

	CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(creature->getParentId()));
	if(!cell)
	{
		assert(false && "SpatialIndexManager::createInWorld cannot cast cell ???? ");
		return;
	}
	
	BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));	
	if(!building)
	{
		assert(false && "SpatialIndexManager::createInWorld cannot cast building ???? ");
		return;
	}

	//add the Creature to the cell we are in
	cell->addObjectSecure(creature);
}


void SpatialIndexManager::createInWorld(PlayerObject* player)
{
	//just create in the SI - it will keep track of nearby players
	this->AddObject(player);

	//are we in a cell? otherwise bail out
	if(player->getParentId() == 0)
	{
		return;
	}

	CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
	if(!cell)
	{
		assert(false && "cannot cast cell ???? ");
		return;
	}
	
	BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));	
	if(!building)
	{
		assert(false && "cannot cast building ???? ");
		return;
	}

	//add the Creature to the cell we are in
	cell->addObjectSecure(player);
				
	//iterate through all the cells and add the player as listener
	CellObjectList::iterator cellIt = building->getCellList()->begin();

	while(cellIt != building->getCellList()->end())
	{
		gContainerManager->registerPlayerToContainer((*cellIt),player);
		++cellIt;
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
		player->getEquipManager()->addEquippedObject(object);//veeeeery debateable .. - probably shouldnt be done here
		gContainerManager->createObjectToRegisteredPlayers(parent, object);

		//sendCreateObject(object,player,false);
		gContainerManager->updateEquipListToRegisteredPlayers(player);
		return;
	}

	//please note that individual cells have players as listeners, not the entire building
	//items in cells
	CellObject* cell = dynamic_cast<CellObject*>(parent);
	if(cell)
	{
		gContainerManager->createObjectToRegisteredPlayers(parent, object);
		return;
	}	
	
	if(parent)
	{
		//items in containers
		gContainerManager->createObjectToRegisteredPlayers(parent, object);
		return;
	}	

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

//====================================================================================================================
//
// send to all players in chatrange
void SpatialIndexManager::sendToChatRange(Object* container, std::function<void (PlayerObject* const player)> callback)
{

	//get the Objects and unregister as necessary
	ObjectListType playerList;
	//getGrid()->GetPlayerViewingRangeCellContents(gridCell, &playerList);//cell means gridcell here
	getGrid()->GetChatRangeCellContents(container->getGridBucket(), &playerList);

	for(ObjectListType::iterator i = playerList.begin(); i != playerList.end(); i++)
	{
		PlayerObject* player = dynamic_cast<PlayerObject*>((*i));

		callback(player);
		
	}

}

//=============================================================================================================================
//
// just create other players / creatures for us - we still exist for them as we were still in logged state
//

bool SpatialIndexManager::InitializeObject(PlayerObject *player)
{
	//Pesudo
	// 1. Calculate CellID
	// 2. Set CellID
	// 3. Insert object into the cell in the hash table

	//now create around for us
	//we have to query the grid as noncontainers must be created to
		
	ObjectListType playerList;
	getGrid()->GetViewingRangeCellContents(player->getGridBucket(), &playerList,(Bucket_Creatures|Bucket_Objects|Bucket_Players));
		
	for(ObjectListType::iterator i = playerList.begin(); i != playerList.end(); i++)
	{
		//we just added ourselves to the grid - dont send a create to ourselves
		if(((*i)->getId() == player->getId()))
		{
			continue;
		}
		
		sendCreateObject((*i), player, false);

		//registrations should still be intact
		//if(((*i)->getType() == ObjType_Creature) || ((*i)->getType() == ObjType_NPC))
		//{
			//gContainerManager->registerPlayerToContainer((*i), player);
			//continue;
		//}
		

	}
	return true;
}
