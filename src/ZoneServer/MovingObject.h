/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
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

		// current movement updates sequence
		uint32		getInMoveCount(){ return mInMoveCount; }
		void		setInMoveCount(uint32 moveCount){ mInMoveCount = moveCount; }

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
