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

#include "HarvesterFactory.h"


#include "Utils/logger.h"

#include "Deed.h"
#include "HarvesterObject.h"
#include "ResourceContainerFactory.h"
#include "WorldManager.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"

//=============================================================================

bool				HarvesterFactory::mInsFlag    = false;
HarvesterFactory*	HarvesterFactory::mSingleton  = NULL;

//======================================================================================================================

HarvesterFactory*	HarvesterFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new HarvesterFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

HarvesterFactory::HarvesterFactory(Database* database) : FactoryBase(database)
{

    _setupDatabindings();
}

//=============================================================================

HarvesterFactory::~HarvesterFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void HarvesterFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case HFQuery_ResourceData:
    {

        uint32 count = (uint32)result->getRowCount();

        HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(asyncContainer->mObject);

        if(count)
        {
            DataBinding* binding = mDatabase->createDataBinding(2);
            binding->addField(DFT_uint64,offsetof(HarvesterHopperItem,ResourceID),8,0);
            binding->addField(DFT_float,offsetof(HarvesterHopperItem,Quantity),4,1);

            HResourceList*	hRList = harvester->getResourceList();
            hRList->resize(hRList->size()+count);

            HarvesterHopperItem hopperTemp;
            for(uint64 i=0; i <count; i++)
            {
                result->getNextRow(binding,&hopperTemp);
                hRList->push_back(std::make_pair(hopperTemp.ResourceID,hopperTemp.Quantity));
            }

        }

        QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HFQuery_AttributeData,asyncContainer->mClient,asyncContainer->mId);
        asynContainer->mId		= harvester->getId();
        asynContainer->mObject	= harvester;

        mDatabase->executeSqlAsync(this,asynContainer,"SELECT attributes.name,sa.value,attributes.internal"
                                   " FROM %s.structure_attributes sa"
                                   " INNER JOIN %s.attributes ON (sa.attribute_id = attributes.id)"
                                   " WHERE sa.structure_id = %" PRIu64 " ORDER BY sa.order",
                                   mDatabase->galaxy(),mDatabase->galaxy(),harvester->getId());
       
    }
    break;

    case HFQuery_AttributeData:
    {
        HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(asyncContainer->mObject);
        //_buildAttributeMap(harvester,result);

        Attribute_QueryContainer	attribute;
        uint64						count = result->getRowCount();
        //int8						str[256];
        //BStringVector				dataElements;

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(mAttributeBinding,(void*)&attribute);
            harvester->addInternalAttribute(attribute.mKey,std::string(attribute.mValue.getAnsi()));
        }

        harvester->setLoadState(LoadState_Loaded);

        LOG(INFO) << "Loaded harvester with id [" << harvester->getId() << "]";
        asyncContainer->mOfCallback->handleObjectReady(harvester,asyncContainer->mClient);

    }
    break;

    case HFQuery_MainData:
    {
        QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HFQuery_ResourceData,asyncContainer->mClient,asyncContainer->mId);

        HarvesterObject* harvester = new(HarvesterObject);
        _createHarvester(result,harvester);

        asynContainer->mObject = harvester;
        asynContainer->mClient = asyncContainer->mClient;

        //asyncContainer->mOfCallback->handleObjectReady(harvester,asyncContainer->mClient);

        //now request the associated resource container count

        int8 sql[250];
        sprintf(sql,"SELECT hr.resourceID, hr.quantity FROM %s.harvester_resources hr WHERE hr.ID = '%" PRIu64 "' ",mDatabase->galaxy(),harvester->getId());
        mDatabase->executeSqlAsync(this,asynContainer,sql);
     

    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================


//=============================================================================

void HarvesterFactory::_createHarvester(DatabaseResult* result, HarvesterObject* harvester)
{
    if (!result->getRowCount()) {
       	return;
    }

    result->getNextRow(mHarvesterBinding,harvester);

    harvester->setLoadState(LoadState_Loaded);
    harvester->setType(ObjType_Structure);
    harvester->mCustomName.convert(BSTRType_Unicode16);
}

//=============================================================================

void HarvesterFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    //request the harvesters Data first

    int8 sql2[1024];
    sprintf(sql2,	"SELECT s.id,s.owner,s.oX,s.oY,s.oZ,s.oW,s.x,s.y,s.z,std.type,std.object_string,std.stf_name, std.stf_file, s.name, std.lots_used, std.resource_Category, h.ResourceID, h.active, h.rate, std.maint_cost_wk, std.power_used, s.condition, std.max_condition, std.repair_cost "
            "FROM %s.structures s INNER JOIN %s.structure_type_data std ON (s.type = std.type) INNER JOIN %s.harvesters h ON (s.id = h.id) "
            "WHERE (s.id = %" PRIu64 ")",
            mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),id);
    QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,HFQuery_MainData,client,id);

    mDatabase->executeSqlAsync(this,asynContainer,sql2);
    
}


//=============================================================================

void HarvesterFactory::_setupDatabindings()
{
    mHarvesterBinding = mDatabase->createDataBinding(24);
    mHarvesterBinding->addField(DFT_uint64,offsetof(HarvesterObject,mId),8,0);
    mHarvesterBinding->addField(DFT_uint64,offsetof(HarvesterObject,mOwner),8,1);
    mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.x),4,2);
    mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.y),4,3);
    mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.z),4,4);
    mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mDirection.w),4,5);
    mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mPosition.x),4,6);
    mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mPosition.y),4,7);
    mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mPosition.z),4,8);
    mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mHarvesterFamily),4,9);
    mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mModel),256,10);
    mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mName),256,11);
    mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mNameFile),256,12);
    mHarvesterBinding->addField(DFT_bstring,offsetof(HarvesterObject,mCustomName),256,13);
    mHarvesterBinding->addField(DFT_uint8,offsetof(HarvesterObject,mLotsUsed),1,14);
    mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mResourceCategory),4,15);

    mHarvesterBinding->addField(DFT_uint64,offsetof(HarvesterObject,mCurrentResource),8,16);
    mHarvesterBinding->addField(DFT_uint8,offsetof(HarvesterObject,mActive),1,17);
    mHarvesterBinding->addField(DFT_float,offsetof(HarvesterObject,mCurrentExtractionRate),4,18);

    mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,maint_cost_wk),4,19);
    mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mPowerUsed),4,20);
    mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mDamage),4,21);
    mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mMaxCondition),4,22);
    mHarvesterBinding->addField(DFT_uint32,offsetof(HarvesterObject,mRepairCost),4,23);

}

//=============================================================================

void HarvesterFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mHarvesterBinding);

}

//=============================================================================

void HarvesterFactory::handleObjectReady(Object* object,DispatchClient* client)
{
    //*ONLY* used to load resource containers
    //never used ....

    InLoadingContainer* ilc = _getObject(object->getParentId());
    if (! ilc) {//ILC sanity check...
    	LOG(WARNING) << "Could not locate InLoadingContainer for object parent [" << object->getParentId() << "]";
        return;
    }

    HarvesterObject*		harvester = dynamic_cast<HarvesterObject*>(ilc->mObject);

    //add res containers
    gWorldManager->addObject(object,true);


    if(harvester->decLoadCount() == 0)
    {
        if(!(_removeFromObjectLoadMap(harvester->getId())))
        	LOG(WARNING) << "Failed removing object from loadmap";

        ilc->mOfCallback->handleObjectReady(harvester,ilc->mClient);

        mILCPool.free(ilc);
    }

}

//=============================================================================

void HarvesterFactory::releaseAllPoolsMemory()
{
    releaseQueryContainerPoolMemory();
    releaseILCPoolMemory();

}
