/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_BADGEREGION_FACTORY_H
#define ANH_ZONESERVER_BADGEREGION_FACTORY_H

#include "BadgeRegion.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"

#define	 gBadgeRegionFactory	BadgeRegionFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;

//=============================================================================

enum BadgeFQuery
{
	BadgeFQuery_MainData	= 1,
};

//=============================================================================

class BadgeRegionFactory : public FactoryBase
{
public:

	static BadgeRegionFactory*	getSingletonPtr() { return mSingleton; }
	static BadgeRegionFactory*	Init(Database* database);

	~BadgeRegionFactory();

	void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
	void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

private:

	BadgeRegionFactory(Database* database);

	void				_setupDatabindings();
	void				_destroyDatabindings();

	BadgeRegion*		_createBadgeRegion(DatabaseResult* result);

	static BadgeRegionFactory*		mSingleton;
	static bool						mInsFlag;

	DataBinding*					mBadgeRegionBinding;
};

//=============================================================================


#endif

