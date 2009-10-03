#pragma once
#include "Common/MessageDispatchCallback.h"
#include "ObjectFactory.h"
#include "ZoneOpcodes.h"
#include "ObjectControllerOpcodes.h"
#include "Item.h"
#include "PlayerObject.h"
#include <vector>


#define gFireworkManager FireworkManager::getSingletonPtr()

class Message;
class Database;
class MessageDispatch;
class PlayerObject;
class ObjectControllerCommandMap;

class FireworkManager
{
public:
	static FireworkManager*	getSingletonPtr() { return mSingleton; }
	
	static FireworkManager*	Instance()
	{
		if (!mSingleton)
		{
			mSingleton = new FireworkManager();
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


	//bool createFirework(uint32 typeId, PlayerObject* player, bool isShow=false);
	bool createFirework(uint32 typeId, PlayerObject* player, Anh_Math::Vector3 position, bool isShow=false);


protected:
	~FireworkManager(void);

private:
	static FireworkManager*	mSingleton;
	
	FireworkManager(){}
	
};
