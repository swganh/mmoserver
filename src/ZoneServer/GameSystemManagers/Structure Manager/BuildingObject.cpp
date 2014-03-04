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

#include "ZoneServer/GameSystemManagers/Structure Manager/BuildingObject.h"

#include "anh/Utils/rand.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/GameSystemManagers/Structure Manager/CellObject.h"
#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/GameSystemManagers/Spatial Index Manager/SpatialIndexManager.h"
#include "ZoneServer/GameSystemManagers/Spawn Manager/SpawnPoint.h"

//=============================================================================

BuildingObject::BuildingObject() : PlayerStructure()
{
    mType = ObjType_Building;
    mMaxStorage = 0;

}

//=============================================================================

BuildingObject::~BuildingObject()
{
    SpawnPoints::iterator it = mSpawnPoints.begin();

    while(it != mSpawnPoints.end())
    {
        delete(*it);

        it = mSpawnPoints.erase(it);
    }
    //destroy the cells
    CellObjectList::iterator cellIt = mCells.begin();

    while(cellIt != mCells.end())
    {
        gWorldManager->destroyObject((*cellIt));
        //cellIt++;
        cellIt = mCells.erase(cellIt);
    }

}

//=============================================================================



//=============================================================================

SpawnPoint* BuildingObject::getRandomSpawnPoint()
{
    if(mSpawnPoints.size())
    {
        return(mSpawnPoints[gRandom->getRand()%mSpawnPoints.size()]);
    }

    return(NULL);
}

//=============================================================================

bool BuildingObject::removeCell(CellObject* cellObject)
{
    CellObjectList::iterator it = mCells.begin();

    while(it != mCells.end())
    {
        if((*it) == cellObject)
        {
            mCells.erase(it);
            return(true);
        }
        ++it;
    }
    return(false);
}

//=============================================================================

bool BuildingObject::checkForCell(CellObject* cellObject)
{
    CellObjectList::iterator it = mCells.begin();

    while(it != mCells.end())
    {
        if((*it) == cellObject)
            return(true);
        ++it;
    }
    return(false);
}

//=============================================================================
//gets the count of all tangibles in a cell
//how to not count the structure terminal ????
// - dont count tangibles marked as ststic :)
uint16 BuildingObject::getCellContentCount()
{
    uint16 count = 0;
    CellObjectList::iterator it = mCells.begin();

    while(it != mCells.end())
    {
        count += (*it)->getHeadCount();

        ++it;
    }
    return(count);
}


ObjectList BuildingObject::getAllCellChilds()
{
	ObjectList	resultList;
	this->ViewObjects(nullptr, 0, false, [&](Object* object){
		if(object->getType() != SWG_CELL)	{
			resultList.push_back(object);
		}
	});

    return(resultList);
}

//================================================================================
//
//the cells send an updated permission  to the specified player
//
void BuildingObject::updateCellPermissions(PlayerObject* player, bool access)
{
    //iterate through all the cells 
    //place players inside the world
	this->ViewObjects(nullptr, 1, false, [&](Object* object){
		if(object->getObjectType() == SWG_CELL)	{
			CellObject* cell = dynamic_cast<CellObject*>(object);

			gMessageLib->sendUpdateCellPermissionMessage(cell,access,player);

			//are we inside the cell ?
			if((player->getParentId() == cell->getId()) && (!access))
			{
				//were no longer allowed to be inside ....
				//so get going

				//TODO find outside position (sign position for example??) to place the player

				glm::vec3 playerPosition = player->mPosition;
				glm::vec3 playerWorldPosition = player->getWorldPosition();
				glm::vec3 position;

				glm::vec3 playerNewPosition;
				position.x = (0 - playerPosition.x) - 5;
				position.z = (0 - playerPosition.z) - 2;

				position.y = mPosition.y + 50;

				position.x += playerWorldPosition.x;
				//position.y += player->getWorldPosition.x;
				position.z += playerWorldPosition.z;

				player->GetCreature()->updatePosition(0,position);
        
			}
		}
	});

}

void BuildingObject::prepareDestruction() 
{
    //iterate through all the registered watchers
    //place players inside into the world and unregister the content
    //add an option to delete those players we send to ...

    gContainerManager->sendToRegisteredPlayers(this, [=] (PlayerObject* player) {

        gSpatialIndexManager->removeStructureItemsForPlayer(player,this);

        //is the player inside ??? or was he merely still watching??
        CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
        if (!cell) {
			//we have no interest in bystanders
            return;
        }

        BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));
        if (!building || building->getId() != this->getId()) {
            return;
        }

        //No need to update the SI here as we only leave the cell
        player->GetCreature()->updatePosition(0, player->GetCreature()->getWorldPosition());
        player->setParentIdIncDB(0);

        cell->RemoveObject(this, player);

        gMessageLib->broadcastContainmentMessage(player, 0, 0xffffffff);
    });

    //remove items in the building from the world
    CellObjectList* cell_list = getCellList();
    std::for_each(cell_list->begin(), cell_list->end(), [] (CellObject* cell) {
        cell->prepareDestruction();
    });
}
