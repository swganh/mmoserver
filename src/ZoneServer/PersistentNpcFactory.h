/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_PERSISTENTNPC_FACTORY_H
#define ANH_ZONESERVER_PERSISTENTNPC_FACTORY_H

#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"
#include "NpcIdentifier.h"


#define	 gPersistentNPCFactory	PersistentNPCFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class NPCObject;

//=============================================================================

enum PersistentNpcQuery
{
	PersistentNpcQuery_MainData		= 1,
	PersistentNpcQuery_Attributes	= 2,
};

//=============================================================================

class PersistentNpcFactory : public FactoryBase
{
public:

	static PersistentNpcFactory*	getSingletonPtr() { return mSingleton; }
	static PersistentNpcFactory*	Init(Database* database);

	~PersistentNpcFactory();

	void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
	void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

private:

	PersistentNpcFactory(Database* database);

	void				_setupDatabindings();
	void				_destroyDatabindings();

	NPCObject*			_createPersistentNpc(DatabaseResult* result);

	static PersistentNpcFactory*	mSingleton;
	static bool						mInsFlag;

	DataBinding*					mPersistentNpcBinding;
	DataBinding*					mNpcIdentifierBinding;
};

//=============================================================================


#endif

