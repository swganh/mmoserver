/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CREATURE_FACTORY_H
#define ANH_ZONESERVER_CREATURE_FACTORY_H

#include "FactoryBase.h"

#define	 gCreatureFactory	CreatureFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class PersistentNpcFactory;
class ShuttleFactory;

//=============================================================================

class CreatureFactory : public FactoryBase
{
	public:

		static CreatureFactory*	getSingletonPtr() { return mSingleton; }
		static CreatureFactory*	Init(Database* database);

		~CreatureFactory();

		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result){}
		void					requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

		void					releaseAllPoolsMemory();

	private:

		CreatureFactory(Database* database);

		static CreatureFactory*		mSingleton;
		static bool					mInsFlag;

		PersistentNpcFactory*		mPersistentNpcFactory;
		ShuttleFactory*				mShuttleFactory;

};

//=============================================================================

#endif

