/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "MovingObject.h"
#include "MathLib/Quaternion.h"


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

