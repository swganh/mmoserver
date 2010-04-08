
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#pragma once

#include "AttackableCreature.h"
#include "MathLib/Vector3.h"
#include "Utils/typedefs.h"

#define gScoutManager ScoutManager::getSingletonPtr()

class Database;
class Message;
class MessageDispatch;
class PlayerObject;
class ObjectControllerCommandMap;
class ZoneTree;

#define gScoutManager	ScoutManager::getSingletonPtr()

enum HarvestSelection
{
	HARVEST_ANY = 0,
	HARVEST_MEAT,
	HARVEST_HIDE,
	HARVEST_BONE
};


class ScoutManager
{
public:
	static ScoutManager*	getSingletonPtr() { return mSingleton; }
	
	static ScoutManager*	Instance()
	{
		if (!mSingleton)
		{
			mSingleton = new ScoutManager();

		}
		return mSingleton;
	}

	static inline void deleteManager(void)    
	{ 
		if (mSingleton)
		{
			delete mSingleton;
			mSingleton = 0;
		}
	}

	//camps
	bool createCamp(uint32 typeId,uint64 parentId, const glm::vec3& position, const string& customName, PlayerObject* player);

	//foraging
	static void successForage(PlayerObject* player);

	//harvesting
	void handleHarvestCorpse(PlayerObject* player, CreatureObject* target, HarvestSelection harvest);
	uint32 getHarvestSkillFactor(CreatureObject* object);
	uint32 getCreatureFactor(CreatureObject* object);

protected:
	ScoutManager::ScoutManager();
	~ScoutManager(void);

private:
	static ScoutManager*	mSingleton;

};
