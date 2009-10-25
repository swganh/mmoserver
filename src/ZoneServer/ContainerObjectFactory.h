/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CONTAINER_OBJECT_FACTORY_H
#define ANH_ZONESERVER_CONTAINER_OBJECT_FACTORY_H

#include "FactoryBase.h"
#include "ObjectFactoryCallback.h"

#define	 gContainerFactory	ContainerObjectFactory::getSingletonPtr()

//=============================================================================

class Container;
class Database;
class DataBinding;
class DispatchClient;
class TangibleFactory;

//=============================================================================

enum CFQuery
{
	CFQuery_MainData	= 1,
	CFQuery_ObjectCount = 2,
	CFQuery_Objects		= 3
};

//=============================================================================

class ContainerObjectFactory : public FactoryBase, public ObjectFactoryCallback
{
	public:

		static ContainerObjectFactory*	getSingletonPtr() { return mSingleton; }
		static ContainerObjectFactory*	Init(Database* database);
		static inline void destroySingleton(void)
		{
			if (mSingleton)
			{
				delete mSingleton;
				mSingleton = 0;
			}
		}

		virtual void	handleObjectReady(Object* object,DispatchClient* client);
		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);



	private:
		~ContainerObjectFactory();
		ContainerObjectFactory(Database* database);

		void _setupDatabindings();
		void _destroyDatabindings();

		Container* _createContainer(DatabaseResult* result);

		TangibleFactory* mTangibleFactory;

		static ContainerObjectFactory*	mSingleton;


		DataBinding* mContainerBinding;
};

//=============================================================================

class CFAsyncContainer
{
	public:

		CFAsyncContainer(ObjectFactoryCallback* of,CFQuery qt,DispatchClient* cl){ ofCallback = of;client = cl;queryType = qt; }

		DispatchClient*			client;
		ObjectFactoryCallback*	ofCallback;
		Container*				container;
		CFQuery					queryType;
};

//=============================================================================


#endif

