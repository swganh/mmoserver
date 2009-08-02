/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_FACTORY_H
#define ANH_ZONESERVER_TANGIBLE_FACTORY_H

#include "TangibleObject.h"
#include "ContainerObjectFactory.h"
#include "TerminalFactory.h"
#include "TicketCollectorFactory.h"
#include "ItemFactory.h"
#include "ResourceContainerFactory.h"

#include "FactoryBase.h"

#define		gTangibleFactory	TangibleFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;

//=============================================================================

class TangibleFactory : public FactoryBase
{
	public:

		static TangibleFactory*	getSingletonPtr() { return mSingleton; }
		static TangibleFactory*	Init(Database* database);

		~TangibleFactory();

		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result){}
		void					requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

		void					releaseAllPoolsMemory();

	private:

		TangibleFactory(Database* database);

		static TangibleFactory*		mSingleton;
		static bool				mInsFlag;

		ContainerObjectFactory*		mContainerFactory;
		TerminalFactory*			mTerminalFactory;
		TicketCollectorFactory*		mTicketCollectorFactory;
		ItemFactory*				mItemFactory;
		ResourceContainerFactory*	mResourceContainerFactory;
};

//=============================================================================

#endif

