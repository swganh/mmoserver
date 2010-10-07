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

#include "CellObject.h"
#include "Datapad.h"
#include "MessageLib/MessageLib.h"
#include "MovingObject.h"
#include "PlayerObject.h"
#include "VehicleController.h"
#include "WorldManager.h"
#include "SpatialIndexManager.h"
 

//=============================================================================

MovingObject::MovingObject()
: ObjectContainer()
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
, mSubZone(NULL)
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

	gSpatialIndexManager->UpdateObject(this);
	this->mPosition = newPosition;
	
	//we have been inside - move us outside
	if (this->getParentId() != 0)
	{
		// if we just left a building we need to update our containment - send a zero to uncontain us from that old cell
		gMessageLib->broadcastContainmentMessage(this,this->getParentId(),0);
		

		// remove us from the last cell we were in
		if (CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
		{
			cell->removeObject(this);
		}
		else
		{
			gLogger->log(LogManager::NOTICE,"Error removing %"PRIu64" from cell(%"PRIu64")",this->getId(),this->getParentId());
		}

		// now set our new ParentId
		this->setParentId(0);
		//and update the position
		this->mPosition = newPosition;

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
		if (!this->getKnownPlayers()->empty())
		{
			gMessageLib->broadcastContainmentMessage(this,oldParentId,0);
		}

		// only remove us from si, if we just entered the building
		if (oldParentId != 0)
		{
			// We are still inside.
			if ((cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(oldParentId))))
			{
				cell->removeObject(this);
			}
			else
			{
				gLogger->log(LogManager::NOTICE,"Error removing %"PRIu64" from cell(%"PRIu64")",this->getId(),oldParentId);
			}
		}
		else
		{
			// remove us from qt
			// We just entered a building.
			if (this->getSubZoneId())
			{
				if (QTRegion* region = gWorldManager->getQTRegion(this->getSubZoneId()))
				{
					this->setSubZoneId(0);
					region->mTree->removeObject(this);
				}
			}
		}

		// put us into new one
		if (!this->getKnownPlayers()->empty())
		{
			gMessageLib->broadcastContainmentMessage(this,parentId,4);
		}
		if ((cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(parentId))))
		{
			cell->addObjectSecure(this);
		}
		else
		{
			gLogger->log(LogManager::NOTICE,"Error adding %"PRIu64" to cell(%"PRIu64")",this->getId(),parentId);
		}
		// update the player
		this->setParentId(parentId);
	}
}

void MovingObject::updatePosition(uint64 parentId, const glm::vec3& newPosition)
{

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

	//TODO do we need to update our known Objects ???
	//answer YES if we are a player 
	bool isPlayer = false;
	
	if(PlayerObject* player = dynamic_cast<PlayerObject*>(this))
	{
		isPlayer = true;
		//we cannot stop entertaining here, as the entertainermanager uses this code to move us to the placed instrument
		
		player->getController()->playerWorldUpdate(true);

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
	if ((!isPlayer) && this->getKnownPlayers()->empty())
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
			this->incInMoveCount();
			gMessageLib->sendUpdateTransformMessageWithParent(this);
		}
	}
	else
	{
		if(isPlayer)
			gMessageLib->sendDataTransform0B(this);
		else
		{
			this->incInMoveCount();
			gMessageLib->sendUpdateTransformMessage(this);
		}
	}
}
