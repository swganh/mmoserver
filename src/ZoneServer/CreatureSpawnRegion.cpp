/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
// #include "Utils/utils.h"
#include "CreatureSpawnRegion.h"
#include "LogManager/LogManager.h"

//=============================================================================
CreatureSpawnRegion::CreatureSpawnRegion() : mId(0)
{
}

CreatureSpawnRegion::~CreatureSpawnRegion()
{
	gLogger->logMsgF("CreatureSpawnRegion::~CreatureSpawnRegion()\n", MSG_NORMAL);
}



