/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CREATURESPAWNREGION_H
#define ANH_ZONESERVER_CREATURESPAWNREGION_H

#include "MathLib/Vector3.h"
#include "MathLib/Quaternion.h"

//=============================================================================


//=============================================================================


class CreatureSpawnRegion
{
	public:
		CreatureSpawnRegion::CreatureSpawnRegion();
		CreatureSpawnRegion::~CreatureSpawnRegion();
		
		/*
		// Copy constructor
		SpawnData::SpawnData(const SpawnData& spawn);			

		// Assignment operator
		SpawnData& operator=(const SpawnData& spawn);
		SpawnData& operator=(const SpawnData* spawn);
		*/
		uint64 mId;
		float mPosX;
		float mPosZ;
		float mWidth;
		float mLength;
};

#endif

