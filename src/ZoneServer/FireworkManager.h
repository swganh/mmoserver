
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#pragma once

#include "Common/MessageDispatchCallback.h"
#include "MathLib/Vector3.h"
#include "TangibleObject.h"

#define gFireworkManager FireworkManager::getSingletonPtr()

class PlayerObject;

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
	TangibleObject* createFirework(uint32 typeId, PlayerObject* player, Anh_Math::Vector3 position);


protected:
	~FireworkManager(void);

private:
	static FireworkManager*	mSingleton;
	
	FireworkManager(){}
	
};
