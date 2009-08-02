#pragma once
#include "Common/MessageDispatchCallback.h"
#include "ObjectFactory.h"
#include "ZoneOpcodes.h"
#include "Item.h"
#include "PlayerObject.h"
#include <vector>


#define gMedicManager MedicManager::getSingletonPtr()

class Message;
class Database;
class MessageDispatch;
class PlayerObject;
class ObjectControllerCommandMap;

class MedicManager
{
public:
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
	bool HealDamage(PlayerObject* Medic, CreatureObject* Target, ObjectControllerCmdProperties* cmdProperties){ return HealDamage(Medic, Target, 0, cmdProperties); }
	bool HealDamage(PlayerObject* Medic, CreatureObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties);
	bool Diagnose(PlayerObject* Medic, PlayerObject* Target);
	~MedicManager(void);
private:
	static MedicManager*	mSingleton;
	static bool				mInsFlag;
	MessageDispatch*		Dispatch;
	MedicManager(MessageDispatch* dispatch){ Dispatch = dispatch; }
	
};
