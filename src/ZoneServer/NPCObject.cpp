/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "NPCObject.h"
#include "MathLib/Quaternion.h"
#include "MathLib/Vector3.h"
#include "MessageLib/MessageLib.h"
#include "ZoneServer/WorldManager.h"
#include "Heightmap.h"

//=============================================================================

NPCObject::NPCObject() : CreatureObject(),
mLastConversationTarget(0),
mLastConversationRequest(0),
mAiState(NpcIsDormant)
{
	mType = ObjType_NPC;
	mCreoGroup = CreoGroup_PersistentNpc;
}

//=============================================================================

NPCObject::~NPCObject()
{
}

//=============================================================================
//
//	Returns a new random position given the rectangle of +- offsetX and +- offsetY.
//

Anh_Math::Vector3 NPCObject::getRandomPosition(Anh_Math::Vector3& currentPos, int32 offsetX, int32 offsetZ) const
{
	Anh_Math::Vector3 v(currentPos);

	// TODO: Validate map bounderies.
	v.mX = (float)(v.mX - (offsetX/2)) + gRandom->getRand() % (int)offsetX;
	v.mZ = (float)(v.mZ - (offsetZ/2)) + gRandom->getRand() % (int)offsetZ;

	v.mY = Heightmap::getSingletonPtr()->getHeight(v.mX, v.mZ ); 

	// gLogger->logMsgF("NPCObject::getRandomPosition: %.0f, %.0f, %.0f ", MSG_NORMAL, v.mX, v.mY, v.mZ);
	return v;
}

//=============================================================================
//
//	Retrieve the height for a given 2D x,z position.
//

float NPCObject::getHeightAt2DPosition(float xPos, float zPos) const
{
	float yPos = Heightmap::getSingletonPtr()->getHeight(xPos, zPos); 
	return yPos;
}


//=============================================================================
//
//	Set the direction of the npc.
//

void NPCObject::setDirection(float deltaX, float deltaZ)
{
	// Let's turn to the direction we are heading.
	float x = deltaX;
	float z = deltaZ;
	float h = sqrt(x*x + z*z);

	if ((z/h) < 0.0)
	{	
		if (x/h < 0.0)
		{
			this->mDirection.mW = cos((3.14159354 * 0.5) + 0.5f*acos(-z/h));
			this->mDirection.mY = sin((3.14159354 * 0.5) + 0.5f*acos(-z/h));
		}
		else
		{
			this->mDirection.mY = sin(0.5f*acos(z/h));
			this->mDirection.mW = cos(0.5f*acos(z/h));
		}
	}
	else
	{
		this->mDirection.mY = sin(0.5f*asin(x/h));	
		this->mDirection.mW = cos(0.5f*acos(z/h));
	}

	// send out position updates to known players
	if (this->getKnownPlayers()->empty())
	{
		return;
	}

	this->setInMoveCount(this->getInMoveCount() + 1);

	if (this->getParentId())
	{
		// We are inside a cell.
		gMessageLib->sendDataTransformWithParent(this);
		gMessageLib->sendUpdateTransformMessageWithParent(this);
	}
	else
	{
		gMessageLib->sendDataTransform(this);
		gMessageLib->sendUpdateTransformMessage(this);
	}
}

//=============================================================================
//
//	Move npc and update position in game world.
//

void NPCObject::moveAndUpdatePosition(void)
{
	// this->mPosition += this->getPositionOffset();
	 
	// send out position updates to known players
	this->updatePosition(this->getParentId(),this->mPosition + this->getPositionOffset());
}

//=============================================================================
//
//	Update npc position in game world.
//

void NPCObject::updatePosition(uint64 parentId, Anh_Math::Vector3 newPosition)
{
	// gLogger->logMsgF("NPCObject::updatePosition: update NPC position)", MSG_NORMAL);

	if (parentId == 0)
	{
		if (this->getParentId() != 0)
		{
			// if we just left a building
			if (!this->getKnownPlayers()->empty())
			{
				gMessageLib->broadcastContainmentMessage(this->getId(),this->getParentId(),0,this);
			}

			// remove us from the last cell we were in
			if (CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
			{
				cell->removeChild(this);
			}
			else
			{
				gLogger->logMsgF("Error removing %llu from cell(%lld)\n",MSG_HIGH,this->getId(),this->getParentId());
			}

			// we are outside again
			this->setParentId(0);

			this->mPosition = newPosition;

			// Add us to the world.
			if (!this->getKnownPlayers()->empty())
			{
				gMessageLib->broadcastContainmentMessage(this->getId(),0,4,this);
			}

			// add us to the qtree
			if (QTRegion* newRegion = gWorldManager->getSI()->getQTRegion((double)this->mPosition.mX,(double)this->mPosition.mZ))
			{
				this->setSubZoneId((uint32)newRegion->getId());
				newRegion->mTree->addObject(this);
			}
			else
			{
				// we should never get here !
				gLogger->logMsg("NPCObject::updatePosition: could not find zone region in map\n");
			}
		}
		else
		{
			// We are still outside.

			// get the qt of the new position
			// if (QTRegion* newRegion = gWorldManager->getSI()->getQTRegion((double)this->mPosition.mX,(double)this->mPosition.mZ))
			if (QTRegion* newRegion = gWorldManager->getSI()->getQTRegion((double)newPosition.mX, (double)newPosition.mZ))
			{
				// we didnt change so update the old one
				if((uint32)newRegion->getId() == this->getSubZoneId())
				{
					// this also updates the object (npcs) position
					newRegion->mTree->updateObject(this, newPosition);
				}
				else
				{
					// remove from old
					if (QTRegion* oldRegion = gWorldManager->getQTRegion(this->getSubZoneId()))
					{
						oldRegion->mTree->removeObject(this);
					}

					// put into new
					this->mPosition = newPosition;
					this->setSubZoneId((uint32)newRegion->getId());
					newRegion->mTree->addObject(this);
				}			
			}
		}
	}
	else
	{
		uint64 oldParentId = this->getParentId();
		if (oldParentId != parentId)
		{
			// We changed cell
			CellObject* cell = NULL;

			// Remove us.
			if (!this->getKnownPlayers()->empty())
			{
				gMessageLib->broadcastContainmentMessage(this->getId(),oldParentId,0,this);
			}
			// gLogger->logMsgF("NPC changed cell from (%lld) to (%lld)",MSG_NORMAL, oldParentId, parentId);

			// only remove us from si, if we just entered the building
			if (oldParentId != 0) 
			{
				// We are still inside.
				if (cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(oldParentId)))
				{
					cell->removeChild(this);
				}
				else
				{
					gLogger->logMsgF("Error removing %lld from cell(%lld)\n",MSG_NORMAL,this->getId(),oldParentId);
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
				gMessageLib->broadcastContainmentMessage(this->getId(),parentId,4,this);
			}
			if (cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(parentId)))
			{
				cell->addChild(this);
			}
			else
			{
				gLogger->logMsgF("Error adding %lld to cell(%lld)\n",MSG_NORMAL,this->getId(),parentId);
			}
			// update the player
			this->setParentId(parentId);
		}
	}
	this->mPosition = newPosition;

	// send out position updates to known players
	this->setInMoveCount(this->getInMoveCount() + 1);
	
	//check whether updates are necessary before building the packet and then destroying it
	if (this->getKnownPlayers()->empty())
	{
		return;
	}

	if (this->getParentId())
	{
		// We are inside a cell.
		// gMessageLib->sendDataTransformWithParent(this);
		gMessageLib->sendUpdateTransformMessageWithParent(this);
	}
	else
	{
		// gMessageLib->sendDataTransform(this);
		gMessageLib->sendUpdateTransformMessage(this);
	}
}


//=============================================================================
//
//	Set AI state.
//
void NPCObject::setAiState(NPCObject::Npc_AI_State state)
{
	mAiState = state;
}

//=============================================================================
//
//	Get AI state.
//
NPCObject::Npc_AI_State NPCObject::getAiState(void) const
{
	return mAiState;
}


