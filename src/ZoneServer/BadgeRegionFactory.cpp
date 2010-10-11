/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "BadgeRegionFactory.h"
#include "BadgeRegion.h"
#include "ObjectFactoryCallback.h"
#include "Common/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/utils.h"

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

void BadgeRegionFactory::handleDatabaseJobComplete(void* ref, DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case BadgeFQuery_MainData:
    {
        BadgeRegion* badgeRegion = _createBadgeRegion(result);

        if(badgeRegion->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
            asyncContainer->mOfCallback->handleObjectReady(badgeRegion, asyncContainer->mClient);
        else
        {

        }
    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void BadgeRegionFactory::requestObject(ObjectFactoryCallback* ofCallback, uint64 id, uint16 subGroup, uint16 subType, DispatchClient* client)
{
    // Get our badge based on the regionID
    mDatabase->ExecuteProcedureAsync(this, new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback, BadgeFQuery_MainData, client), "CALL sp_BadgeGetByRegion(%"PRIu64");", id);
    
}

//=============================================================================

BadgeRegion* BadgeRegionFactory::_createBadgeRegion(DatabaseResult* result)
{
    if (!result->getRowCount()) {
    	return nullptr;
    }

    BadgeRegion*	badgeRegion = new BadgeRegion();

    result->GetNextRow(mBadgeRegionBinding, badgeRegion);

    badgeRegion->setLoadState(LoadState_Loaded);

    return badgeRegion;
}

//=============================================================================

void BadgeRegionFactory::_setupDatabindings()
{
    mBadgeRegionBinding = mDatabase->CreateDataBinding(9);
    mBadgeRegionBinding->addField(DFT_uint64,offsetof(BadgeRegion, mId), 8, 0);
    mBadgeRegionBinding->addField(DFT_uint32,offsetof(BadgeRegion, mBadgeId), 4, 1);
    mBadgeRegionBinding->addField(DFT_bstring,offsetof(BadgeRegion, mRegionName), 64, 2);
    mBadgeRegionBinding->addField(DFT_bstring,offsetof(BadgeRegion, mNameFile), 64, 3);
    mBadgeRegionBinding->addField(DFT_float,offsetof(BadgeRegion, mPosition.x), 4, 4);
    mBadgeRegionBinding->addField(DFT_float,offsetof(BadgeRegion, mPosition.z), 4, 5);
    mBadgeRegionBinding->addField(DFT_float,offsetof(BadgeRegion, mWidth), 4, 6);
    mBadgeRegionBinding->addField(DFT_float,offsetof(BadgeRegion, mHeight), 4, 7);
    mBadgeRegionBinding->addField(DFT_uint64,offsetof(BadgeRegion, mParentId), 8, 8);
}

//=============================================================================

void BadgeRegionFactory::_destroyDatabindings()
{
    mDatabase->DestroyDataBinding(mBadgeRegionBinding);
}

//=============================================================================

