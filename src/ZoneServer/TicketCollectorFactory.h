/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TICKETCOLLECTOR_OBJECT_FACTORY_H
#define ANH_ZONESERVER_TICKETCOLLECTOR_OBJECT_FACTORY_H

#include "FactoryBase.h"

#define	 gTicketCollectorFactory	TicketCollectorFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class TicketCollector;

//=============================================================================

enum TCFQuery
{
	TCFQuery_MainData	= 1
};

//=============================================================================

class TicketCollectorFactory : public FactoryBase
{
	public:

		static TicketCollectorFactory*	getSingletonPtr() { return mSingleton; }
		static TicketCollectorFactory*	Init(Database* database);

		~TicketCollectorFactory();

		void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

	private:

		TicketCollectorFactory(Database* database);

		void				_setupDatabindings();
		void				_destroyDatabindings();

		TicketCollector*	_createTicketCollector(DatabaseResult* result);

		static TicketCollectorFactory*	mSingleton;
		static bool					mInsFlag;

		DataBinding*					mTicketCollectorBinding;
};

//=============================================================================


#endif

