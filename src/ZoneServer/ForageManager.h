/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#pragma once

#include <vector>
#include "MathLib/Vector3.h"
#include "Utils/typedefs.h"

#define gForageManager ForageManager::getSingletonPtr()

class Database;
class Message;
class MessageDispatch;
class PlayerObject;
class ObjectControllerCommandMap;
class ZoneTree;

enum forageClasses
{
	ForageClass_Scout,
	ForageClass_Medic
};

enum forageFails
{
	NOT_OUTSIDE,
	PLAYER_MOVED,
	ACTION_LOW,
	IN_COMBAT,
	AREA_EMPTY,
	ENTERED_COMBAT,
	NO_SKILL,
	ALREADY_FORAGING
};

class ForageAttempt;
class ForagePocket;

#define gForageManager	ForageManager::getSingletonPtr()

class ForageManager
{
public:
	static ForageManager*	getSingletonPtr() { return mSingleton; }
	
	static ForageManager*	Instance()
	{
		if (!mSingleton)
		{
			mSingleton = new ForageManager();

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

	//foraging
	void forageUpdate();
	void startForage(PlayerObject* player, forageClasses);
	static void failForage(PlayerObject* player, forageFails fail);
	static void ForageManager::successForage(PlayerObject* player, forageClasses forageClass);

protected:
	ForageManager::ForageManager();
	~ForageManager(void);

private:
	static ForageManager*	mSingleton;
	ZoneTree*				mSI;

	ForagePocket*			pHead;

};
