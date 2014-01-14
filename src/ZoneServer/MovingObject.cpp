/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MovingObject.h"

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
	// Build the direction vector between the object to face and us.
	glm::vec3 direction = glm::normalize(object_to_face->mPosition - mPosition);

	// Build the vectors needed for the direction matrix (used to create the quaternion).
	glm::vec3 up(0.0f, 1.0f, 0.0f);					// Y is the default up direction
	glm::vec3 right = glm::cross(up, direction);	// The perpendicular vector to up and direction
	up = glm::cross(direction, right);				// The true up vector

	// Calculate the quaternion from the given matrix.
	mDirection = glm::normalize(glm::toQuat(glm::mat3(right, up, direction)));
}

//=============================================================================

