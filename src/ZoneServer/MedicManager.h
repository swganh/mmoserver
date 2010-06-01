
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
	bool CheckWoundPack(PlayerObject* Medic, CreatureObject* Target, ObjectControllerCmdProperties* cmdProperties);
	
	bool CheckMedicine(PlayerObject* Medic, CreatureObject* Target, ObjectControllerCmdProperties* cmdProperties, uint64 mOPCode);

	bool CheckMedicRange(PlayerObject* Medic, CreatureObject* Target, float healRange);
	int  CalculateBF(PlayerObject* Medic, CreatureObject* Target, int32 maxhealamount);
	
	bool HealDamage(PlayerObject* Medic, CreatureObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties);
	bool HealDamageRanged(PlayerObject* Medic, CreatureObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties);
	bool HealWound(PlayerObject* Medic, CreatureObject* Target, uint64 WoundPackobjectID, ObjectControllerCmdProperties* cmdProperties);

	void startInjuryTreatmentEvent(PlayerObject* Medic);
	bool Diagnose(PlayerObject* Medic, PlayerObject* Target);
	void successForage(PlayerObject* player);

private:
	static MedicManager*	mSingleton;
	static bool				mInsFlag;
	MessageDispatch*		Dispatch;

	MedicManager(MessageDispatch* dispatch);	
};
