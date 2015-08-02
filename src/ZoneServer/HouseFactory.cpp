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

#include "HouseFactory.h"


#include "Utils/logger.h"

#include "Deed.h"
#include "HouseObject.h"
#include "PlayerObject.h"
#include "ResourceContainer.h"
#include "CellFactory.h"
#include "CellObject.h"
#include "TangibleFactory.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "MessageLib/MessageLib.h"
#include "Utils/utils.h"

//=============================================================================

bool				HouseFactory::mInsFlag    = false;
HouseFactory*		HouseFactory::mSingleton  = NULL;

//======================================================================================================================

HouseFactory*	HouseFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new HouseFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

HouseFactory::HouseFactory(Database* database) : FactoryBase(database)
{

    _setupDatabindings();
    mCellFactory = CellFactory::Init(mDatabase);
}

//=============================================================================

HouseFactory::~HouseFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void HouseFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {

    case HOFQuery_CellData:
    {
        HouseObject*	house = dynamic_cast<HouseObject*>(asyncContainer->mObject);
        uint32			cellCount;
        uint64			cellId;

        DataBinding*	cellBinding = mDatabase->createDataBinding(1);
        cellBinding->addField(DFT_int64,0,8);

        // store us for later lookup
        mObjectLoadMap.insert(std::make_pair(house->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(house,asyncContainer->mOfCallback,asyncContainer->mClient)));

        cellCount = static_cast<uint32>(result->getRowCount());

        house->setLoadCount(cellCount);
        uint64 maxid = 0xffffffffffffffff;
        for(uint32 j = 0; j < cellCount; j++)
        {
            result->getNextRow(cellBinding,&cellId);

            mCellFactory->requestStructureCell(this,cellId,0,0,asyncContainer->mClient);
            if(cellId < maxid)
                maxid = cellId;
        }
        house->setMinCellId(maxid);


        mDatabase->destroyDataBinding(cellBinding);

        //read in admin list - do houses only have an admin list ???
        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HOFQuery_AdminData,asyncContainer->mClient);
        asContainer->mObject = house;

        int8 sql[1024];
        sprintf(sql,"SELECT PlayerID FROM %s.structure_admin_data WHERE StructureID = %" PRIu64 " AND AdminType like 'ADMIN';",mDatabase->galaxy(),house->getId());
        mDatabase->executeSqlAsync(this,asContainer,sql);
        

    }
    break;

    case HOFQuery_AdminData:
    {
        HouseObject*	house = dynamic_cast<HouseObject*>(asyncContainer->mObject);


        struct adminStruct
        {
            uint64 playerID;
            BString adminList;
        };

        adminStruct adminData;

        DataBinding*	adminBinding = mDatabase->createDataBinding(1);
        adminBinding->addField(DFT_uint64,offsetof(adminStruct,playerID),8,0);

        uint64 count = result->getRowCount();

        for(uint32 j = 0; j < count; j++)
        {
            result->getNextRow(adminBinding,&adminData);
            house->addHousingAdminEntry(adminData.playerID);
        }

        mDatabase->destroyDataBinding(adminBinding);

    }
    break;

    case HOFQuery_AttributeData:
    {

        HouseObject* house = dynamic_cast<HouseObject*>(asyncContainer->mObject);
        //_buildAttributeMap(harvester,result);

        Attribute_QueryContainer	attribute;
        uint64						count = result->getRowCount();
        //int8						str[256];
        //BStringVector				dataElements;

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(mAttributeBinding,(void*)&attribute);
            house->addInternalAttribute(attribute.mKey,std::string(attribute.mValue.getAnsi()));
        }

        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HOFQuery_CellData,asyncContainer->mClient);
        asContainer->mObject = house;

        mDatabase->executeSqlAsync(this,asContainer,"SELECT id FROM %s.structure_cells WHERE parent_id = %" PRIu64 " ORDER BY structure_cells.id;",mDatabase->galaxy(),house->getId());
        



    }
    break;

    case HOFQuery_MainData:
    {
        QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,HOFQuery_AttributeData,asyncContainer->mClient,asyncContainer->mId);


        HouseObject* house = new(HouseObject);
        _createHouse(result,house);

        asynContainer->mObject = house;
        asynContainer->mClient = asyncContainer->mClient;
        asynContainer->mId		= house->getId();


        mDatabase->executeSqlAsync(this,asynContainer,"SELECT attributes.name,sa.value,attributes.internal"
                                   " FROM %s.structure_attributes sa"
                                   " INNER JOIN %s.attributes ON (sa.attribute_id = attributes.id)"
                                   " WHERE sa.structure_id = %" PRIu64 " ORDER BY sa.order",
                                   mDatabase->galaxy(),mDatabase->galaxy(),house->getId());
        

    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================


//=============================================================================

void HouseFactory::_createHouse(DatabaseResult* result, HouseObject* house)
{
    if (!result->getRowCount()) {
       	return;
    }

    result->getNextRow(mHouseBinding,house);

    house->setLoadState(LoadState_Loaded);
    house->setType(ObjType_Building);
    house->mCustomName.convert(BSTRType_Unicode16);
    house->setPlayerStructureFamily(PlayerStructure_House);

}

//=============================================================================

void HouseFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    //request the houses Data first

    int8 sql[1024];
    sprintf(sql,	"SELECT s.id,s.owner,s.oX,s.oY,s.oZ,s.oW,s.x,s.y,s.z, "
            "std.type,std.object_string,std.stf_name, std.stf_file, s.name, "
            "std.lots_used, h.private, std.maint_cost_wk, s.condition, std.max_condition, std.max_storage "
            "FROM %s.structures s INNER JOIN %s.structure_type_data std ON (s.type = std.type) INNER JOIN %s.houses h ON (s.id = h.id) "
            "WHERE (s.id = %" PRIu64 ")",
            mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),id);

    QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,HOFQuery_MainData,client,id);

    mDatabase->executeSqlAsync(this,asynContainer,sql);
    
}

//=============================================================================

void HouseFactory::_setupDatabindings()
{
    mHouseBinding = mDatabase->createDataBinding(20);
    mHouseBinding->addField(DFT_uint64,offsetof(HouseObject,mId),8,0);
    mHouseBinding->addField(DFT_uint64,offsetof(HouseObject,mOwner),8,1);
    mHouseBinding->addField(DFT_float,offsetof(HouseObject,mDirection.x),4,2);
    mHouseBinding->addField(DFT_float,offsetof(HouseObject,mDirection.y),4,3);
    mHouseBinding->addField(DFT_float,offsetof(HouseObject,mDirection.z),4,4);
    mHouseBinding->addField(DFT_float,offsetof(HouseObject,mDirection.w),4,5);
    mHouseBinding->addField(DFT_float,offsetof(HouseObject,mPosition.x),4,6);
    mHouseBinding->addField(DFT_float,offsetof(HouseObject,mPosition.y),4,7);
    mHouseBinding->addField(DFT_float,offsetof(HouseObject,mPosition.z),4,8);

    mHouseBinding->addField(DFT_uint32,offsetof(HouseObject,mHouseFamily),4,9);//thats the structure_type_data ID
    mHouseBinding->addField(DFT_bstring,offsetof(HouseObject,mModel),256,10);
    mHouseBinding->addField(DFT_bstring,offsetof(HouseObject,mName),256,11);
    mHouseBinding->addField(DFT_bstring,offsetof(HouseObject,mNameFile),256,12);
    mHouseBinding->addField(DFT_bstring,offsetof(HouseObject,mCustomName),256,13);

    mHouseBinding->addField(DFT_uint8,offsetof(HouseObject,mLotsUsed),1,14);
    mHouseBinding->addField(DFT_uint8,offsetof(HouseObject,mPublic),1,15);
    mHouseBinding->addField(DFT_uint32,offsetof(HouseObject,maint_cost_wk),4,16);
    mHouseBinding->addField(DFT_uint32,offsetof(HouseObject,mDamage),4,17);
    mHouseBinding->addField(DFT_uint32,offsetof(HouseObject,mMaxCondition),4,18);
    mHouseBinding->addField(DFT_uint32,offsetof(HouseObject,mMaxStorage),4,19);

}

//=============================================================================

void HouseFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mHouseBinding);

}

//=============================================================================

void HouseFactory::handleObjectReady(Object* object,DispatchClient* client)
{
    //Perform checking on startup there is no client!
    // client will in all cases be NULL in this factory
    //if(!client)
    //	return;

    //add our cells
    InLoadingContainer* ilc = _getObject(object->getParentId());

    //Perform checking.
    if(!ilc)    {
        return;
    }

    HouseObject*		house = dynamic_cast<HouseObject*>(ilc->mObject);

    //add hopper / new item to worldObjectlist, but NOT to the SI
    gWorldManager->addObject(object,true);

    //pondering whether to use the objectcontainer instead
    house->addCell(dynamic_cast<CellObject*>(object));

    if(house->getLoadCount() == (house->getCellList())->size())
    {
        if(!(_removeFromObjectLoadMap(house->getId())))
            LOG(WARNING) << "Failed removing object from loadmap";

        ilc->mOfCallback->handleObjectReady(house,ilc->mClient);

        mILCPool.free(ilc);
    }

}

//=============================================================================

void HouseFactory::releaseAllPoolsMemory()
{
    releaseQueryContainerPoolMemory();
    releaseILCPoolMemory();

}
