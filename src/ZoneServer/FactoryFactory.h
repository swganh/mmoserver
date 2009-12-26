	   /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_FACTORY_OBJECT_FACTORY_H
#define ANH_ZONESERVER_FACTORY_OBJECT_FACTORY_H

#include "ObjectFactoryCallback.h"
#include "FactoryBase.h"

#define gFactoryFactory FactoryFactory::getSingletonPtr()


//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class FactoryObject;

//=============================================================================

enum FFQuery
{
	FFQuery_MainData		= 1,
	FFQuery_ResourceData	= 2,
	FFQuery_byDeed			= 3,
	FFQuery_AdminData		= 4,
	FFQuery_AttributeData	= 5

};

//=============================================================================

class FactoryFactory : public FactoryBase, public ObjectFactoryCallback
{
	public:

		static FactoryFactory*	getSingletonPtr() { return mSingleton; }
		static FactoryFactory*	Init(Database* database);

		~FactoryFactory();

		virtual void	handleObjectReady(Object* object,DispatchClient* client);
		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

		void			releaseAllPoolsMemory();

	private:

		FactoryFactory(Database* database);

		void			_setupDatabindings();
		void			_destroyDatabindings();

		void			_createFactory(DatabaseResult* result, FactoryObject* factory);

		static FactoryFactory*		mSingleton;
		static bool					mInsFlag;


		DataBinding*				mFactoryBinding;

};

//=============================================================================


#endif


