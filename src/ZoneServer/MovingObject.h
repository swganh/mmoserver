/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_MOVING_OBJECT_H
#define ANH_ZONESERVER_MOVING_OBJECT_H

#include "Object.h"


class Message;
class DispatchClient;
//=============================================================================

class MovingObject : public Object
{
	friend class ObjectFactory;
	friend class PlayerObjectFactory;

public:
	MovingObject();
	virtual ~MovingObject();

	//NPC Player movement through server (warping, elevators)
	void updatePosition(uint64 parentId, const glm::vec3& newPosition);
	void updatePositionInCell(uint64 parentId, const glm::vec3& newPosition);
	void updatePositionOutside(uint64 parentId, const glm::vec3& newPosition);

	// current movement updates sequence
	uint32		getInMoveCount(){ return mInMoveCount; }
	void		setInMoveCount(uint32 moveCount){ mInMoveCount = moveCount; }
	uint32		incInMoveCount(){ return ++mInMoveCount; }

	// walk speed
	float		getBaseAcceleration(){ return mBaseAcceleration; }
	void		setBaseAcceleration(float acc){ mBaseAcceleration = acc; }
	void		setCurrentAcceleration(float acceleration){ mCurrentAcceleration = acceleration; }
	float		getCurrentAcceleration(){ return mCurrentAcceleration; }

	// turn rate
	float		getBaseTurnRate(){ return mBaseTurnRate; }
	void		setBaseTurnRate(float rate){ mBaseTurnRate = rate; }
	float		getCurrentTurnRate(){ return mCurrentTurnRate; }
	void		setCurrentTurnRate(float rate){ mCurrentTurnRate = rate; }

	// speed
	float		getBaseRunSpeedLimit(){ return mBaseRunSpeedLimit; }
	void		setBaseRunSpeedLimit(float speed){ mBaseRunSpeedLimit = speed; }

	float		getCurrentSpeedModifier(){ return mCurrentSpeedMod; }
	void		setCurrentSpeedModifier(float rate){ mCurrentSpeedMod = rate; }

	float		getBaseSpeedModifier(){ return mBaseSpeedMod; }
	void		setBaseSpeedModifier(float rate){ mBaseSpeedMod = rate; }
		
	//current Run speed limit is the speed at which we run
	void		setCurrentRunSpeedLimit(float speed){ mCurrentRunSpeedLimit = speed; }
	float		getCurrentRunSpeedLimit(){ return mCurrentRunSpeedLimit; }

	//current speed is the speed of our current movement
	float		getCurrentSpeed(){ return mCurrentSpeed; }
	void		setCurrentSpeed(float speed){ mCurrentSpeed = speed; }

	// terrain negotiation
	float		getBaseTerrainNegotiation(){ return mBaseTerrainNegotiation; }
	void		setBaseTerrainNegotiation(float tn){ mBaseTerrainNegotiation = tn; }
	float		getCurrentTerrainNegotiation(){ return mCurrentTerrainNegotiation; }
	void		setCurrentTerrainNegotiation(float tn){ mCurrentTerrainNegotiation = tn; }

	// update current values
	virtual void	updateMovementProperties() = 0;

	virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount){}


protected:

	uint32		mInMoveCount;
	float		mBaseAcceleration;
	float		mBaseRunSpeedLimit;
	float		mBaseTerrainNegotiation;
	float		mBaseTurnRate;
	float		mCurrentAcceleration;
	float		mCurrentRunSpeedLimit;
	float		mCurrentSpeed;
	float		mCurrentTerrainNegotiation;
	float		mCurrentTurnRate;
	float		mCurrentSpeedMod;
	float		mBaseSpeedMod;
};

//=============================================================================

#endif
