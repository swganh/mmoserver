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
#include "MathLib/Vector3.h"
#include "MathLib/Quaternion.h"


//=============================================================================

class SpawnPoint
{
	public:

		SpawnPoint(){}
		~SpawnPoint(){}

	uint64					mCellId;
	Anh_Math::Quaternion	mDirection;
	Anh_Math::Vector3		mPosition;
	string					mName;
};

//=============================================================================

#endif


