/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "BadgeRegionFactory.h"
#include "ObjectFactoryCallback.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "LogManager/LogManager.h"
#include "Utils/utils.h"

#include <assert.h>

//=============================================================================

bool				BadgeRegionFactory::mInsFlag    = false;
BadgeRegionFactory*	BadgeRegionFactory::mSingleton  = NULL;

//======================================================================================================================

BadgeRegionFactory*	BadgeRegionFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new BadgeRegionFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

BadgeRegionFactory::BadgeRegionFactory(Database* database) : FactoryBase(database)
{
	_setupDatabindings();
}

//=============================================================================

BadgeRegionFactory::~BadgeRegionFactory()
{
	_destroyDatabindings();

	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void BadgeRegionFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case BadgeFQuery_MainData:
		{
			BadgeRegion* badgeRegion = _createBadgeRegion(result);

			if(badgeRegion->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
				asyncContainer->mOfCallback->handleObjectReady(badgeRegion,asyncContainer->mClient);
			else
			{

			}
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void BadgeRegionFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,BadgeFQuery_MainData,client),
								"SELECT badge_regions.id,badge_regions.badge_id,planet_regions.region_name,planet_regions.region_file,planet_regions.x,planet_regions.z,"
								"planet_regions.width,planet_regions.height,badge_regions.parent_id"
								" FROM badge_regions"
								" INNER JOIN planet_regions ON (badge_regions.region_id = planet_regions.region_id)"
								" WHERE (badge_regions.id = %lld)",id);
}

//=============================================================================

BadgeRegion* BadgeRegionFactory::_createBadgeRegion(DatabaseResult* result)
{
	BadgeRegion*	badgeRegion = new BadgeRegion();

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mBadgeRegionBinding,badgeRegion);

	badgeRegion->setLoadState(LoadState_Loaded);

	return badgeRegion;
}

//=============================================================================

void BadgeRegionFactory::_setupDatabindings()
{
	mBadgeRegionBinding = mDatabase->CreateDataBinding(9);
	mBadgeRegionBinding->addField(DFT_uint64,offsetof(BadgeRegion,mId),8,0);
	mBadgeRegionBinding->addField(DFT_uint32,offsetof(BadgeRegion,mBadgeId),4,1);
	mBadgeRegionBinding->addField(DFT_bstring,offsetof(BadgeRegion,mRegionName),64,2);
	mBadgeRegionBinding->addField(DFT_bstring,offsetof(BadgeRegion,mNameFile),64,3);
	mBadgeRegionBinding->addField(DFT_float,offsetof(BadgeRegion,mPosition.mX),4,4);
	mBadgeRegionBinding->addField(DFT_float,offsetof(BadgeRegion,mPosition.mZ),4,5);
	mBadgeRegionBinding->addField(DFT_float,offsetof(BadgeRegion,mWidth),4,6);
	mBadgeRegionBinding->addField(DFT_float,offsetof(BadgeRegion,mHeight),4,7);
	mBadgeRegionBinding->addField(DFT_uint64,offsetof(BadgeRegion,mParentId),8,8);
}

//=============================================================================

void BadgeRegionFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mBadgeRegionBinding);
}

//=============================================================================

