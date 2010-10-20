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
#include "HouseObject.h"
#include "PlayerObject.h"
#include "RegionObject.h"

#include "MountObject.h"
#include "ObjectContainer.h"
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
	uint32 finalCell = getGrid()->AddObject(newObject);
	
	//any errors ?
	if(finalCell == 0xffffffff)
	{
		return false;
	}

	//now create it for everyone around
	if(newObject->getType() == ObjType_Player)
	{
		PlayerObject* player = dynamic_cast<PlayerObject*>(newObject);
		
		ObjectListType* playerList = getGrid()->GetAllViewingRangeCellContents(finalCell);
		
		for(ObjectListType::iterator i = playerList->begin(); i != playerList->end(); i++)
		{
			sendCreateObject((*i),player, true);

			if((*i)->getType() == ObjType_Player)
			{
				PlayerObject* otherPlayer = dynamic_cast<PlayerObject*>((*i));
				sendCreateObject(player,otherPlayer, true);

				registerPlayerToContainer(otherPlayer, player);
				registerPlayerToContainer(player, otherPlayer);
			}
		}
	}
	else 
	{
		ObjectListType* playerList = getGrid()->GetPlayerViewingRangeCellContents(finalCell);
		for(ObjectListType::iterator i = playerList->begin(); i != playerList->end(); i++)
		{
			PlayerObject* otherPlayer = dynamic_cast<PlayerObject*>((*i));
			sendCreateObject(newObject,otherPlayer, true);
		}
	}

	return true;
}

void SpatialIndexManager::UpdateObject(Object *updateObject)
{

	uint32 oldCell = updateObject->zmapCellID;
	
	//sets the new gridcell, updates subcells
	getGrid()->UpdateObject(updateObject);

	if(updateObject->zmapCellID != oldCell)
	{
		//test how much we moved if only one grid proceed normally
		if((updateObject->zmapCellID == (oldCell +1))||(updateObject->zmapCellID == (oldCell -1))
			||(updateObject->zmapCellID == (oldCell + GRIDWIDTH))	||(updateObject->zmapCellID == (oldCell - GRIDWIDTH))	
			)
		{
	
			//remove us from the row we left
			UpdateBackCells(updateObject,oldCell);
			
			//create us for the row in which direction we moved
			UpdateFrontCells(updateObject,oldCell);
		}
		else //we teleported destroy all and create everything new
		{
			//remove us from everything
			RemoveObject(updateObject,oldCell);
			
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
		uint32 cell = owner->zmapCellID;
		
		ObjectListType* playerList = getGrid()->GetPlayerViewingRangeCellContents(cell);

		for(ObjectListType::iterator i = playerList->begin(); i != playerList->end(); i++)
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

void SpatialIndexManager::RemoveObject(Object *removeObject)
{
	uint32 cell = removeObject->zmapCellID;
	RemoveObject(removeObject, cell);
}

void SpatialIndexManager::RemovePlayerFromWorld(PlayerObject *removePlayer)
{
	//remove us from the grid
	RemoveObject(removePlayer);

	//remove us out of the cell
	if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(removePlayer->getParentId())))
	{
		//that might be able to go entirely - already done by removeObject
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


//===========================================================================
// this will remove an object from the grid
// if the object is a container with listeners, the listeners will be unregistered
void SpatialIndexManager::RemoveObject(Object *removeObject, uint32 cell)
{
	
	getGrid()->RemoveObject(removeObject);

	//now destroy it for everyone around as well as around for it
	
	ObjectListType* playerList = getGrid()->GetPlayerViewingRangeCellContents(cell);

	for(ObjectListType::iterator i = playerList->begin(); i != playerList->end(); i++)
	{
		PlayerObject* player = dynamic_cast<PlayerObject*>((*i));

		//is the object a container?? do we need to despawn the content and unregister it ?
		ObjectContainer* container = dynamic_cast<ObjectContainer*>(removeObject);
		if((container) && (container->checkRegisteredWatchers(player)))
		{
			unRegisterPlayerFromContainer(container, player);	
		}

		gMessageLib->sendDestroyObject(removeObject->getId(),player);
	}

	return;
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
			//is the object a container?? do we need to despawn the content and unregister it ?
			ObjectContainer* container = dynamic_cast<ObjectContainer*>(object);
			if((container) && (container->checkRegisteredWatchers(player)))
			{
				unRegisterPlayerFromContainer(container,player);	
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
				
				//is the object a container?? do we need to despawn the content and unregister it ?
				ObjectContainer* container = dynamic_cast<ObjectContainer*>(object);
				if((container) && (container->checkRegisteredWatchers(player)))
				{
					unRegisterPlayerFromContainer(container,player);	
				}

				//destroy item for the player
				gMessageLib->sendDestroyObject(object->getId(),player);

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

	if(toBeTested->getId() != updatedPlayer->getId())
	{

		if(!updatedPlayer)
		{
			//ahem
			assert(false);
			return;
		}

		//is the object a container?? do we need to despawn the content and unregister it ?
		ObjectContainer* container = dynamic_cast<ObjectContainer*>(toBeTested);
		if((container) && (container->checkRegisteredWatchers(updatedPlayer)))
		{
			unRegisterPlayerFromContainer(container,updatedPlayer);	
		}

		//we (updateObject) got out of range of the following (*i) objects
		//destroy them for us
		if(updatedPlayer)
		{
			gMessageLib->sendDestroyObject(toBeTested->getId(),updatedPlayer);
		}
				
		//if its a player, destroy us for him
		PlayerObject* them = dynamic_cast<PlayerObject*> (toBeTested);
		if(them)
		{
			unRegisterPlayerFromContainer(updatedPlayer,them);	

			gMessageLib->sendDestroyObject(updatedPlayer->getId(),them);
			
		}
	}
}


//=================================================================================
//destroy objects out of our range when moving for ONE GRID
//do not use when teleporting
//
void SpatialIndexManager::UpdateBackCells(Object* updateObject, uint32 oldCell)
{
	
	uint32 newCell = updateObject->zmapCellID;

	//ZMAP Northbound! 
	if((oldCell + GRIDWIDTH) == newCell)
	{
		ObjectListType* FinalList;
		ObjectListType::iterator it;
		
		FinalList = getGrid()->GetAllGridContentsListRow(oldCell - (GRIDWIDTH*VIEWRANGE));

		for(ObjectListType::iterator i = FinalList->begin(); i != FinalList->end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);			
		}	

		return;
	}

	//ZMAP Southbound! 
	else if((oldCell - GRIDWIDTH) == newCell)
	{
		
		ObjectListType* FinalList;
		ObjectListType::iterator it;
		
		FinalList = getGrid()->GetAllGridContentsListRow(oldCell + (GRIDWIDTH*VIEWRANGE));

		for(ObjectListType::iterator i = FinalList->begin(); i != FinalList->end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);			
		}

		return;
	}

	//ZMAP Westbound! 
	else if((oldCell - 1) == newCell)
	{
		
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		FinalList = getGrid()->GetAllGridContentsListRow(oldCell + VIEWRANGE);
		
		for(ObjectListType::iterator i = FinalList->begin(); i != FinalList->end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);
		}

		return;
	}

	//Eastbound! 
	else if((oldCell + 1) == newCell)
	{
		
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		FinalList = getGrid()->GetAllGridContentsListColumn(oldCell - VIEWRANGE);
		

		for(ObjectListType::iterator i = FinalList->begin(); i != FinalList->end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);
		}

		return;
	}

	// NorthEastbound
	else if((oldCell + (GRIDWIDTH+1)) == newCell)
	{
		
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		ObjectListType* temp = getGrid()->GetAllCellContents(oldCell - (GRIDWIDTH+1)*VIEWRANGE);
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListColumnUp(oldCell - ((GRIDWIDTH+1)*VIEWRANGE) +GRIDWIDTH );//		FinalList.splice(it, temp);
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListRowRight(oldCell - ((GRIDWIDTH+1)*VIEWRANGE) + 1);//
		FinalList->splice(it, *temp);

		for(ObjectListType::iterator i = FinalList->begin(); i != FinalList->end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);
		}

		return;
	}

	// NorthWestbound -> up left
	else if((oldCell + GRIDWIDTH-1) == newCell)
	{
		
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		//so we need to delete down right (Southeast)
		ObjectListType* temp = getGrid()->GetAllCellContents(oldCell - ((GRIDWIDTH-1)*VIEWRANGE));
		FinalList->splice(it, *temp); 

		temp = getGrid()->GetAllGridContentsListColumnUp(oldCell - ((GRIDWIDTH-1)*VIEWRANGE) +GRIDWIDTH );//		FinalList.splice(it, temp);
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListRowLeft(oldCell - ((GRIDWIDTH-1)*VIEWRANGE)-1);//
		FinalList->splice(it, *temp);

		for(ObjectListType::iterator i = FinalList->begin(); i != FinalList->end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);	
		}

		return;
	}

		// SouthWestbound	  -> down left
	else if((oldCell - (GRIDWIDTH+1)) == newCell)
	{
		
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		//so we need to delete up right (Northeast)
		ObjectListType* temp = getGrid()->GetAllCellContents(oldCell + (GRIDWIDTH+1)*VIEWRANGE);
		FinalList->splice(it, *temp);

		//get the column down 
		temp = getGrid()->GetAllGridContentsListColumnDown(oldCell + ((GRIDWIDTH+1)*VIEWRANGE) - GRIDWIDTH );//		FinalList.splice(it, temp);
		FinalList->splice(it, *temp);

		//get the row 
		temp = getGrid()->GetAllGridContentsListRowLeft(oldCell + ((GRIDWIDTH+1)*VIEWRANGE) -1);//
		FinalList->splice(it, *temp);

		for(ObjectListType::iterator i = FinalList->begin(); i != FinalList->end(); i++)
		{
			CheckObjectIterationForDestruction((*i),updateObject);
		}

		return;
	}

		// SouthEastbound	-> down right
	else if((oldCell - (GRIDWIDTH-1)) == newCell)
	{
		
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		//so we need to delete up left (Northwest)
		ObjectListType* temp = getGrid()->GetAllCellContents(oldCell + ((GRIDWIDTH-1)*VIEWRANGE) );
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListColumnDown(oldCell + ((GRIDWIDTH-1)*VIEWRANGE) -GRIDWIDTH );//		FinalList.splice(it, temp);
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListRowRight(oldCell + ((GRIDWIDTH-1)*VIEWRANGE) +1);//
		FinalList->splice(it, *temp);
		

		for(ObjectListType::iterator i = FinalList->begin(); i != FinalList->end(); i++)
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

	uint32 newCell = updateObject->zmapCellID;

	//ZMAP Northbound! TODO: Sync with game
	if((oldCell + GRIDWIDTH) == newCell)
	{
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		FinalList = getGrid()->GetAllGridContentsListRow((updateObject->zmapCellID + (GRIDWIDTH*VIEWRANGE)) + GRIDWIDTH);
		

		ObjectCreationIteration(FinalList,updateObject);
		
		return;
	}

	//ZMAP Southbound! TODO: Sync with game
	else if((oldCell - GRIDWIDTH) == newCell)
	{
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		FinalList = getGrid()->GetAllGridContentsListRow((updateObject->zmapCellID - (GRIDWIDTH*VIEWRANGE)) - GRIDWIDTH);
		
		ObjectCreationIteration(FinalList,updateObject);

		return;
	}

			//ZMAP Eastbound! TODO: Sync with game
	else if((oldCell + 1) == newCell)
	{
	
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		FinalList = getGrid()->GetAllGridContentsListColumn((updateObject->zmapCellID + VIEWRANGE) + 1 );

		ObjectCreationIteration(FinalList,updateObject);

		return;
	}

		//ZMAP Westbound! TODO: Sync with game
	else if((oldCell - 1) == newCell)
	{
		
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		FinalList = getGrid()->GetAllGridContentsListColumn((updateObject->zmapCellID - VIEWRANGE) - 1 );

		ObjectCreationIteration(FinalList,updateObject);

		return;
	}

	// NorthEastbound
	else if((oldCell + (GRIDWIDTH+1)) == newCell)
	{
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		ObjectListType* temp = getGrid()->GetAllCellContents((updateObject->zmapCellID + ((GRIDWIDTH+1)*VIEWRANGE)) + (GRIDWIDTH+1));//
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListColumnDown((updateObject->zmapCellID + ((GRIDWIDTH+1)*VIEWRANGE)) - GRIDWIDTH);//
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListRowLeft((updateObject->zmapCellID + ((GRIDWIDTH+1)*VIEWRANGE)) - 1);//
		FinalList->splice(it, *temp);
		
		ObjectCreationIteration(FinalList,updateObject);

		return;
	}

	// NorthWestbound
	else if((oldCell + (GRIDWIDTH-1)) == newCell)
	{
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		ObjectListType* temp = getGrid()->GetAllCellContents((updateObject->zmapCellID + ((GRIDWIDTH-1)*VIEWRANGE)) + (GRIDWIDTH-1));//
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListColumnDown((updateObject->zmapCellID + ((GRIDWIDTH-1)*VIEWRANGE))  + (GRIDWIDTH-1) - GRIDWIDTH);//
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListRowRight((updateObject->zmapCellID + ((GRIDWIDTH-1)*VIEWRANGE))  + (GRIDWIDTH-1) + 1);//
		FinalList->splice(it, *temp);
		ObjectCreationIteration(FinalList,updateObject);

		return;
	}

		// SouthWestbound
	else if((oldCell - (GRIDWIDTH+1)) == newCell)
	{
		
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		ObjectListType* temp = getGrid()->GetAllCellContents((updateObject->zmapCellID - (GRIDWIDTH+1)*VIEWRANGE) - (GRIDWIDTH+1));
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListColumnUp((updateObject->zmapCellID - ((GRIDWIDTH+1)*VIEWRANGE)) - (GRIDWIDTH+1) + GRIDWIDTH);//		FinalList.splice(it, temp);
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListRowRight((updateObject->zmapCellID + ((GRIDWIDTH+1)*VIEWRANGE)) - (GRIDWIDTH+1) + 1);//
		FinalList->splice(it, *temp);

		ObjectCreationIteration(FinalList,updateObject);

		return;
	}

		// SouthEastbound
	else if((oldCell - (GRIDWIDTH-1)) == newCell)
	{
		
		ObjectListType* FinalList;
		ObjectListType::iterator it;

		ObjectListType* temp = getGrid()->GetAllCellContents((updateObject->zmapCellID - (GRIDWIDTH-1)*VIEWRANGE) - (GRIDWIDTH-1));
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListColumnUp((updateObject->zmapCellID - ((GRIDWIDTH-1)*VIEWRANGE)) - (GRIDWIDTH-1) + GRIDWIDTH);//		FinalList.splice(it, temp);
		FinalList->splice(it, *temp);

		temp = getGrid()->GetAllGridContentsListRowLeft((updateObject->zmapCellID + ((GRIDWIDTH-1)*VIEWRANGE)) - (GRIDWIDTH-1) - 1);//
		FinalList->splice(it, *temp);

		ObjectCreationIteration(FinalList,updateObject);

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
			sendCreateObject(toBeTested,updatedPlayer,true);
			
			//if it is a house we need to register the cells for watching *only* when we enter
		}
				
		PlayerObject* testedPlayer = dynamic_cast<PlayerObject*> (toBeTested);
		if(testedPlayer)
		{
			sendCreateObject(updatedObject,testedPlayer,true);

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

void SpatialIndexManager::createCreatureInWorld(CreatureObject* creature)
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
				
			//iterate through all the cells and add the player as listener
			PlayerObject* player = dynamic_cast<PlayerObject*>(creature);
			if(player)
			{		
				CellObjectList::iterator cellIt = building->getCellList()->begin();

				while(cellIt != building->getCellList()->end())
				{
					this->registerPlayerToContainer((*cellIt),player);
					++cellIt;
				}
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

void SpatialIndexManager::createObjectinWorld(Object* object)
{

	if(CreatureObject* creature = dynamic_cast<CreatureObject*>(object))
	{
		createCreatureInWorld(creature);
		return;
	}

	uint32 baseCell = 0xffffffff;

	//get parent object determine who to send creates for

	if(object->getParentId() == 0)
	{
		//just create in the SI - it will keep track of nearby players
		this->AddObject(object);
		return;
	}

	ObjectContainer* parent = dynamic_cast<ObjectContainer*>(gWorldManager->getObjectById(object->getParentId()));

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


//get the Objects in range of the player
void SpatialIndexManager::getObjectsInRange(const Object* const object,ObjectSet* resultSet,uint32 objTypes,float range, bool cellContent)
{
	
	ObjectList* 	resultList;
	
	Object*			tmpObject;
	ObjectType		tmpType;
	uint64			objectId = object->getId();

	uint32			CustomRange;

	//see that we do not query more grids when necessary
	CustomRange = (uint32) range / (MAPWIDTH/GRIDWIDTH);
	if(CustomRange > VIEWRANGE)
	{
		CustomRange = VIEWRANGE;
	}

	glm::vec3   position;

	position = object->getWorldPosition();
	resultList = getGrid()->GetObjectCustomRangeCellContents(getGrid()->getCellId(position.x, position.z),CustomRange);

	// filter needed objects
	ObjectList::iterator it = resultList->begin();
	while(it != resultList->end())
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
						float rangeTest = glm::distance(object->getWorldPosition(),tmpObject->getWorldPosition());
						if(rangeTest <= range)
						{
							resultSet->insert(cellChild);
						}
					}

					++cellChildsIt;
									
				}
			}

			++it;
		}

	}
	
	resultList->clear();
	delete resultList;
}

void SpatialIndexManager::getPlayersInRange(const Object* const object,PlayerObjectSet* resultSet, bool cellContent)
{
	//please note that players are always in the grid, even if in a cell!!!!
	
	ObjectList* 	resultList;
	
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

	resultList = getGrid()->GetPlayerViewingRangeCellContents(getGrid()->getCellId(position.x, position.z));
	
	// filter needed objects
	ObjectList::iterator it = resultList->begin();
	while(it != resultList->end())
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

			++it;
		}
		
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

// registers a containers content as known to a player
// a container can be a backpack placed in a cell (or a cell itself ??? need to think of that - it might give us the ability to keep cell content loaded for a player until he leaves range)
void SpatialIndexManager::registerPlayerToContainer(ObjectContainer* container,PlayerObject* player)
{
	//are we sure the player doesnt know the container already ???
	if(container->checkRegisteredWatchers(player))
	{

		gLogger->log(LogManager::DEBUG,"SpatialIndexManager::registerPlayerToContainer :: Container %I64u already known to player %I64u",container->getId(),player->getId());
		return;	
								
	}

	container->addContainerKnownObjectSafe(player);
	player->addContainerKnownObjectSafe(container);


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

//registering a player to a building is different, as the cells of a building must be known at all time to a player
//even if the cellscontent is not loaded
void SpatialIndexManager::registerPlayerToBuilding(BuildingObject* building,PlayerObject* player)
{
	//iterate through all the cells and add and register their content
	
	//BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));

	if(!building)
	{
		assert(false);
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

//unregistering a player from a building is different, as the cells of a building must be known at all time to a player
//even if the cellscontent can be destroyed unless of course we go out of range and the building is destroyed
void SpatialIndexManager::unRegisterPlayerFromBuilding(BuildingObject* building,PlayerObject* player)
{
	if(!building)
	{
		assert(false);
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

// UnRegisters a container to a player
// a container can be a backpack placed in a cell (or a cell itself ??? need to think of that - it might give us the ability to keep cell content loaded for a player until he leaves range)
// we will only destroy the contents for the client however, not the container
void SpatialIndexManager::unRegisterPlayerFromContainer(ObjectContainer* container,PlayerObject* player)
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
		this->unRegisterPlayerFromBuilding(building,player);
		return;
	}

	container->removeContainerKnownObject(player);
	player->removeContainerKnownObject(container);

	ObjectIDList*			contentList		= container->getObjects();
	ObjectIDList::iterator	it				= contentList->begin();
	
	//its important we do not destroy cells when unregistering a building!!
	while(it != contentList->end())
	{
		TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById((*it)));
		if(!tO)
		{
			assert(false && "SpatialIndexManager::registerPlayerToContainer ::unable to find tangible object (container content)");
		}

		unRegisterPlayerFromContainer(tO,player);

		gMessageLib->sendDestroyObject(tO->getId(),player);
		it++;
	}
	
}

void SpatialIndexManager::createObjectToRegisteredPlayers(ObjectContainer* container,Object* object)
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
			ObjectContainer* oc = dynamic_cast<ObjectContainer*>(object);
			registerPlayerToContainer(oc,recipient);
		}
	);
	
}

//=======================================================================
// player / creature equipped something - update all registered listeners with the changed equiplist
void SpatialIndexManager::updateEquipListToRegisteredPlayers(PlayerObject* player)
{

	sendToPlayersInRange(player,false,[player](PlayerObject* recepient)
		{
			gMessageLib->sendEquippedListUpdate(player, recepient);
		}
	);
}


// sends given function to all of the containers registered watchers
void SpatialIndexManager::sendToRegisteredPlayers(ObjectContainer* container, std::function<void (PlayerObject* player)> callback)
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
void SpatialIndexManager::destroyObjectToRegisteredPlayers(ObjectContainer* container,uint64 object)
{
	gSpatialIndexManager->sendToRegisteredPlayers(container,[container, object, this] (PlayerObject* recipient) 
		{
			ObjectContainer* destroyObject = dynamic_cast<ObjectContainer*>(gWorldManager->getObjectById(object));
			if(destroyObject)
				this->unRegisterPlayerFromContainer(destroyObject,recipient);

			//destroy it for the registered Players
			gMessageLib->sendDestroyObject(object,recipient);
		}
	);

	//the destroyed object can be a container in itself
	//if so destroy it for the parent containers known players
	
}