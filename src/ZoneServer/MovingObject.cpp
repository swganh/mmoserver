/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MovingObject.h"

#include <glm/gtx/transform2.hpp>

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


void MovingObject::faceObject(Object* object_to_face)
{	
    // Create a mirror direction vector for the direction we want to face.
    glm::vec3 direction_vector = glm::normalize(object_to_face->mPosition - mPosition);
    direction_vector.x = -direction_vector.x;

    // Create a lookat matrix from the direction vector and convert it to a quaternion.
    mDirection = glm::toQuat(glm::lookAt(
        direction_vector, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)
        ));

    // If in the 3rd quadrant the signs need to be flipped.
    if (mDirection.y <= 0.0f && mDirection.w >= 0.0f) {
        mDirection.y = -mDirection.y;
        mDirection.w = -mDirection.w;
    }
}

//=============================================================================

