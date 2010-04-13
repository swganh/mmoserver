/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SPAWNPOINT_H
#define ANH_ZONESERVER_SPAWNPOINT_H

#include "Utils/typedefs.h"
#include <glm/glm.hpp>


//=============================================================================

class SpawnPoint
{
 public:
    SpawnPoint(){}
	~SpawnPoint(){}

	uint64		mCellId;
    glm::quat	mDirection;
    glm::vec3	mPosition;
	string		mName;
};

//=============================================================================

#endif


