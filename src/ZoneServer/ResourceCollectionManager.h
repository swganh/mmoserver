/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_RESOURCECOLLECTIONMANAGER_H
#define ANH_ZONESERVER_RESOURCECOLLECTIONMANAGER_H

#define	 gResourceCollectionManager		ResourceCollectionManager::getSingletonPtr()

#include "Utils/typedefs.h"
#include "MathLib/Quaternion.h"
#include "DatabaseManager/DatabaseCallback.h"
#include <boost/pool/pool.hpp>

//======================================================================================================================

class Database;
class DatabaseCallback;
class DatabaseResult;
class DataBinding;
class UIWindow;

namespace Anh_Utils
{
    class Clock;
}

typedef struct tagResourceLocation
{
   Anh_Math::Vector3 position;
   float			 ratio;
} ResourceLocation;

//======================================================================================================================

enum RCMQueryType
{
	RCMQuery_SampleCosts		= 1,
	RCMQuery_SurveyCosts		= 2
};

//======================================================================================================================

class RCMAsyncContainer
{
	public:

		RCMAsyncContainer(RCMQueryType qt){ mQueryType = qt;}
		~RCMAsyncContainer(){}

		RCMQueryType	mQueryType;
};

//======================================================================================================================
class ResourceCollectionManager : public DatabaseCallback,public  UICallback
{
	public:

		~ResourceCollectionManager();
		static ResourceCollectionManager*		Init(Database* database);
		static ResourceCollectionManager*		getSingletonPtr() { return mSingleton; }

		virtual void							handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		void									releaseAllPoolsMemory(){ mDBAsyncPool.release_memory(); }

		uint32									surveyHealthCost;
		uint32									surveyActionCost;
		uint32									surveyMindCost;

		uint32									sampleRadioactiveDamageModifier;
		uint32									sampleHealthCost;
		uint32									sampleActionCost;
		uint32									sampleMindCost;

	private:

		ResourceCollectionManager(Database* database);

		void									handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);
		void									_setupDatabindings();
		void									_destroyDatabindings();

		static bool								mInsFlag;
		static ResourceCollectionManager*		mSingleton;

		Database*								mDatabase;

		boost::pool<boost::default_user_allocator_malloc_free>				mDBAsyncPool;

		DataBinding*							mCommandCostBinding;
};

#endif
