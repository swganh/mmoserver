
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#pragma once

#include <vector>
#include "Common/MessageDispatchCallback.h"

#define gMedicManager MedicManager::getSingletonPtr()

class CreatureObject;
class Database;
class Message;
class MessageDispatch;
class ObjectControllerCommandMap;
class ObjectControllerCmdProperties;
class PlayerObject;

class MedicManager
{
public:
	~MedicManager();

	static MedicManager*	getSingletonPtr() { return mSingleton; }
	static MedicManager*	Init(MessageDispatch* dispatch) 
	{
		if(!mInsFlag)
		{
			mSingleton = new MedicManager(dispatch);
			mInsFlag = true;
			return mSingleton;
		} else {
			return mSingleton;
		}
	}

	bool CheckStim(PlayerObject* Medic, CreatureObject* Target, ObjectControllerCmdProperties* cmdProperties);
	bool HealDamage(PlayerObject* Medic, CreatureObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties);
	bool HealDamageRanged(PlayerObject* Medic, CreatureObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties);
	bool Diagnose(PlayerObject* Medic, PlayerObject* Target);

private:
	static MedicManager*	mSingleton;
	static bool				mInsFlag;
	MessageDispatch*		Dispatch;

	MedicManager(MessageDispatch* dispatch);	
};
