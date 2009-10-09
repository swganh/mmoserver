/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_RESOURCECONTAINER_OBJECT_FACTORY_H
#define ANH_ZONESERVER_RESOURCECONTAINER_OBJECT_FACTORY_H

#include "FactoryBase.h"

#define 	gResourceContainerFactory	ResourceContainerFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class ResourceContainer;

//=============================================================================

enum RCFQuery
{
	RCFQuery_MainData	= 1,
	RCFQuery_Attributes	= 2,
};

//=============================================================================

class ResourceContainerFactory : public FactoryBase
{
	public:

		static ResourceContainerFactory*	getSingletonPtr() { return mSingleton; }
		static ResourceContainerFactory*	Init(Database* database);

		~ResourceContainerFactory();

		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

	private:

		ResourceContainerFactory(Database* database);

		void				_setupDatabindings();
		void				_destroyDatabindings();

		ResourceContainer*	_createResourceContainer(DatabaseResult* result);

		static ResourceContainerFactory*	mSingleton;
		static bool						mInsFlag;

		DataBinding*						mResourceContainerBinding;
};

//=============================================================================


#endif

