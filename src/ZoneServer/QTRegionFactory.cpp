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

#include "QTRegionFactory.h"
#include "QTRegion.h"
#include "ObjectFactoryCallback.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/utils.h"

//=============================================================================

bool				QTRegionFactory::mInsFlag    = false;
QTRegionFactory*	QTRegionFactory::mSingleton  = NULL;

//======================================================================================================================

QTRegionFactory* QTRegionFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new QTRegionFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

QTRegionFactory::QTRegionFactory(Database* database) : FactoryBase(database)
{
    _setupDatabindings();
}

//=============================================================================

QTRegionFactory::~QTRegionFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void QTRegionFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case QTRFQuery_MainData:
    {
        QTRegion* region = _createRegion(result);

        if(region->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
            asyncContainer->mOfCallback->handleObjectReady(region,asyncContainer->mClient);
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

void QTRegionFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,QTRFQuery_MainData,client),
                               "SELECT zone_regions.id,zone_regions.qtdepth,planet_regions.region_name,planet_regions.region_file,planet_regions.x,planet_regions.z,"
                               "planet_regions.width,planet_regions.height"
                               " FROM zone_regions"
                               " INNER JOIN planet_regions ON (zone_regions.region_id = planet_regions.region_id)"
                               " WHERE (zone_regions.id = %"PRIu64")",id);
   
}

//=============================================================================

QTRegion* QTRegionFactory::_createRegion(DatabaseResult* result)
{
    QTRegion*	region = new QTRegion();

    result->getNextRow(mRegionBinding,(void*)region);

    region->initTree();
    region->setLoadState(LoadState_Loaded);

    return region;
}

//=============================================================================

void QTRegionFactory::_setupDatabindings()
{
    mRegionBinding = mDatabase->createDataBinding(8);
    mRegionBinding->addField(DFT_uint32,offsetof(QTRegion,mId),4,0);
    mRegionBinding->addField(DFT_uint8,offsetof(QTRegion,mQTDepth),1,1);
    mRegionBinding->addField(DFT_bstring,offsetof(QTRegion,mRegionName),64,2);
    mRegionBinding->addField(DFT_bstring,offsetof(QTRegion,mNameFile),64,3);
    mRegionBinding->addField(DFT_float,offsetof(QTRegion,mPosition.x),4,4);
    mRegionBinding->addField(DFT_float,offsetof(QTRegion,mPosition.z),4,5);
    mRegionBinding->addField(DFT_float,offsetof(QTRegion,mWidth),4,6);
    mRegionBinding->addField(DFT_float,offsetof(QTRegion,mHeight),4,7);
}

//=============================================================================

void QTRegionFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mRegionBinding);
}

//=============================================================================

