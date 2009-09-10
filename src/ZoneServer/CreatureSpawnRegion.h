/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CREATURESPAWNREGION_H
#define ANH_ZONESERVER_CREATURESPAWNREGION_H

//=============================================================================

#include "Utils/typedefs.h"


//=============================================================================

class CreatureSpawnRegion
{
	public:
		CreatureSpawnRegion();
		~CreatureSpawnRegion();
		
		uint64 mId;
		float mPosX;
		float mPosZ;
		float mWidth;
		float mLength;
};

#endif

