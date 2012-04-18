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

#include "ZoneServer/MovingObject.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/BuildingObject.h"
#include "ZoneServer/CellObject.h"
#include "ZoneServer/ContainerManager.h"
#include "ZoneServer/Datapad.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/SpatialIndexManager.h"
#include "ZoneServer/VehicleController.h"
#include "ZoneServer/WorldManager.h"

//=============================================================================

MovingObject::MovingObject()
    : Object()
    , mInMoveCount(0)
    , mBaseAcceleration(1.50f)
    , mBaseRunSpeedLimit(5.75f)
    , mBaseTerrainNegotiation(1.0f)
    , mBaseTurnRate(1.0f)
    , mCurrentRunSpeedLimit(5.75f)
    , mCurrentSpeed(0.0f)
    , mCurrentTerrainNegotiation(1.0f)
    , mCurrentTurnRate(1.0f)
    , mCurrentSpeedMod(1.0f)
    , mBaseSpeedMod(1.0f)
{
}

//=============================================================================

MovingObject::~MovingObject()
{
}

//=============================================================================

//=============================================================================

//=============================================================================
//
//	Update npc position in game world.
//

void MovingObject::updatePositionOutside(uint64 parentId, const glm::vec3& newPosition)
{
    this->mPosition = newPosition;

    //we have been inside - move us outside
    if (this->getParentId() != 0)
    {
        // if we just left a building we need to update our containment - send a zero to uncontain us from that old cell
        gMessageLib->broadcastContainmentMessage(this,this->getParentId(),0);


        // remove us from the last cell we were in
        // leave building content known - we might enter again
        if (CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
        {
            cell->removeObject(this);
        }
        else
        {
            LOG(WARNING) << "Error removing " << this->getId() << " from cell " << this->getParentId();
        }

        // now set our new ParentId
        this->setParentId(0);

        gMessageLib->broadcastContainmentMessage(this,0,4);

    }
}

void MovingObject::updatePositionInCell(uint64 parentId, const glm::vec3& newPosition)
{
    uint64 oldParentId = this->getParentId();

    if (oldParentId != parentId)
    {
        // We changed cell
        CellObject* cell = NULL;

        // Remove us.
        gMessageLib->broadcastContainmentMessage(this,oldParentId,0);


        if (oldParentId != 0)
        {
            //ONLY REMOVE US FROM BUILDING; WHEN WE CHANGED THE BUILDING - we might have been teleporting

            // We are still inside.
            cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(oldParentId));
            if(!cell)
            {
                LOG(WARNING) << "Error removing " << this->getId() << " from cell " << this->getParentId();
                assert(false);
                return;
            }

            //Did we change the building ??? (teleport moving to cloning center etc)
            if(PlayerObject* player = dynamic_cast<PlayerObject*>(this))
            {
                BuildingObject* oldBuilding = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));

                CellObject* newCell;
                newCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(parentId));
                if (!newCell)
                {
                    this->setParentId(parentId);
                    LOG(WARNING) << this->getId() << " Error casting new cell " << this->getParentId();
                    assert(false);
                    return;
                }
                BuildingObject* newBuilding = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(newCell->getParentId()));

                if(newBuilding != oldBuilding)
                {
                    //unregister player from the old building - either always or only when out of range??
                    gContainerManager->unRegisterPlayerFromBuilding(oldBuilding,player);

                    //and register player for the new building
                    gContainerManager->registerPlayerToBuilding(oldBuilding,player);

                }

            }

            cell->removeObject(this);

        }


        // put us into new one
        gMessageLib->broadcastContainmentMessage(this,parentId,4);

        cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(parentId));
        if (!cell)
        {
            this->setParentId(parentId);
            LOG(WARNING) << "Error adding " << this->getId() << " from cell " << this->getParentId();
            assert(false);
            return;
        }

        cell->addObjectSecure(this);

        // update the player
        this->setParentId(parentId);
    }
}

//server initiated movement
void MovingObject::updatePosition(uint64 parentId, const glm::vec3& newPosition)
{
    facePosition(newPosition);

    if (parentId == 0)
    {
        //we are to be moved (or already are) outside
        updatePositionOutside(parentId, newPosition);
    }
    else
    {
        updatePositionInCell(parentId, newPosition);
    }

    this->mPosition = newPosition;
    this->setParentId(parentId);

    // update grid with world position
    gSpatialIndexManager->UpdateObject(this);

    //TODO do we need to update our known Objects ???
    //answer YES if we are a player
    bool isPlayer = false;

    if(PlayerObject* player = dynamic_cast<PlayerObject*>(this))
    {
        isPlayer = true;

        //we cannot stop entertaining here, as the entertainermanager uses this code to move us to the placed instrument


        //dismount us if we were moved inside
        if(player->checkIfMounted() && player->getMount() && parentId)
        {
            Datapad* datapad			= player->getDataPad();

            if(datapad)
            {
                if(VehicleController* datapad_pet = dynamic_cast<VehicleController*>(datapad->getDataById(player->getMount()->controller())))
                {
                    datapad_pet->Store();
                }
            }
        }
    }

    //check whether updates are necessary before building the packet and then destroying it
    if ((!isPlayer) && this->getRegisteredWatchers()->empty())
    {
        return;
    }

    if (this->getParentId())
    {
        // We are inside a cell.
        //needs to be 0000000B as unknown int otherwise the datatransform gets ignored
        if(isPlayer)
            gMessageLib->sendDataTransformWithParent0B(this);
        else
        {
            gMessageLib->sendUpdateTransformMessageWithParent(this);
        }
    }
    else
    {
        if(isPlayer)
            gMessageLib->sendDataTransform0B(this);
        else
        {
            gMessageLib->sendUpdateTransformMessage(this);
        }
    }
}
