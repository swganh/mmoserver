/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "MovingObject.h"
#include "MathLib/Quaternion.h"


//=============================================================================

MovingObject::MovingObject() : 
Object(),
mCurrentSpeed(0.0f),
mInMoveCount(0),
mBaseAcceleration(1.50f),	// Note: These speed realted values are created in DB whe creating toon.
mBaseRunSpeedLimit(5.75f),
mCurrentRunSpeedLimit(5.75f),
mBaseTurnRate(1.0f),
mCurrentTurnRate(1.0f),
mBaseTerrainNegotiation(1.0f),
mCurrentTerrainNegotiation(1.0f)
{
}

//=============================================================================

MovingObject::~MovingObject()
{
}

//=============================================================================

