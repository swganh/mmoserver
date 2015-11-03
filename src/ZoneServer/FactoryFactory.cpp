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

#include "FactoryFactory.h"


#include "Utils/logger.h"

#include "FactoryCrate.h"

#include "Deed.h"
#include "FactoryObject.h"
#include "PlayerObject.h"
#include "ResourceContainer.h"
#include "TangibleFactory.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "MessageLib/MessageLib.h"
#include "Utils/utils.h"
#include <cassert>

// itemtypes

// weapons				1
// armor				2
// food					4
// clothing				8
// vehicle				16
// droid				32
// chemical				64
// tissue				128
// creatures			256
// furniture			512
// installation			1024
// lightsaber			2048
// generic				4096
// genetics				8192
// mand tailor			16384
// mand armor			32768
// mand droid			65536
// starshipcomponents	131072
// ship tools			262144
// misc					524288

//=============================================================================

bool				FactoryFactory::mInsFlag    = false;
FactoryFactory*		FactoryFactory::mSingleton  = NULL;

//======================================================================================================================

FactoryFactory*	FactoryFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new FactoryFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

FactoryFactory::FactoryFactory(Database* database) : FactoryBase(database)
{

    _setupDatabindings();
}

//=============================================================================

FactoryFactory::~FactoryFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void FactoryFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case FFQuery_HopperItemAttributeUpdate:
    {

        Type1_QueryContainer queryContainer;

        DataBinding*	binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);

        uint64 count;
        count = result->getRowCount();

        // should be 1 result featuring the value as string
        // note that we can get MySQL to give us the resContainersVolume as string as we just tell it to not cast the value
        // MySQL just uses strings in its interface!! so we just wait and cast the value in the virtual function
        // this way we dont have to differentiate between resourceContainers and FactoryCrates and save ourselves alot of unecessary work
        for(uint64 i = 0; i < count; i++)
        {

            TangibleObject* tangible = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(asyncContainer->mId));
            if(!tangible)
            {
                LOG(WARNING) << "Oject is not tangible";
                mDatabase->destroyDataBinding(binding);
                return;
            }
            result->getNextRow(binding,&queryContainer);
            tangible->upDateFactoryVolume(queryContainer.mString.getAnsi()); //virtual function present in tangible, resourceContainer and factoryCrate

        }
        InLoadingContainer* ilc = _getObject(asyncContainer->mHopper);

        if(!ilc) { //Crashbug patch for: http://paste.swganh.org/viewp.php?id=20100627075436-83ca0076fa8abc2a4347e45ee3ede3eb
            mDatabase->destroyDataBinding(binding);
            LOG(WARNING) << "No InLoadingContainer found for hopper [" << asyncContainer->mId << "]";
            return;
        }

        if((--ilc->mLoadCounter)== 0)
        {
            if(!(_removeFromObjectLoadMap(asyncContainer->mHopper)))
                LOG(WARNING) << "Failed removing object from loadmap";

            ilc->mOfCallback->handleObjectReady(asyncContainer->mObject,ilc->mClient,asyncContainer->mHopper);

            mILCPool.free(ilc);
        }

        mDatabase->destroyDataBinding(binding);

    }
    break;

    case FFQuery_HopperUpdate:
    {
        //the player openened a factories hopper.
        //we now asynchronically read the hopper and its content and update them when necessary
        Type1_QueryContainer queryContainer;

        DataBinding*	binding = mDatabase->createDataBinding(2);
        binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
        binding->addField(DFT_uint64,offsetof(Type1_QueryContainer,mId),8,1);


        uint64 count;
        count = result->getRowCount();
        if(!count)
        {
            asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject,asyncContainer->mClient,asyncContainer->mHopper);
            mDatabase->destroyDataBinding(binding);
            return;
        }

        //asyncContainer->mId == HOPPER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
        mObjectLoadMap.insert(std::make_pair(asyncContainer->mId,new(mILCPool.ordered_malloc()) InLoadingContainer(asyncContainer->mObject,asyncContainer->mOfCallback,asyncContainer->mClient,(uint32)count)));

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&queryContainer);

            //read in the ID - find the item in the world or load it newly
            if(strcmp(queryContainer.mString.getAnsi(),"item") == 0)
            {
                Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(queryContainer.mId));
                if(!item)
                {
                    //the item is new - load it over the itemfactory
                    gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Item,0,asyncContainer->mClient);

                }
                //else update relevant attributes
                else
                {
                    QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,FFQuery_HopperItemAttributeUpdate,asyncContainer->mClient,queryContainer.mId);
                    asynContainer->mObject = asyncContainer->mObject;
                    asynContainer->mHopper = asyncContainer->mHopper;

                    mDatabase->executeSqlAsync(this,asynContainer, "SELECT value FROM %s.item_attributes WHERE item_id = %" PRIu64 " AND attribute_id = 400",mDatabase->galaxy(), queryContainer.mId);

                }
            }

            if(strcmp(queryContainer.mString.getAnsi(),"resource") == 0)
            {
                ResourceContainer* rc = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(queryContainer.mId));
                if(!rc)
                {
                    //the container is new - load it over the itemfactory
                    gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_ResourceContainer,0,asyncContainer->mClient);

                }
                else
                {
                    QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,FFQuery_HopperItemAttributeUpdate,asyncContainer->mClient,queryContainer.mId);
                    asynContainer->mObject = asyncContainer->mObject;
                    asynContainer->mHopper = asyncContainer->mHopper;

                    mDatabase->executeSqlAsync(this,asynContainer, "SELECT amount FROM %s.resource_containers WHERE id= %" PRIu64 "",mDatabase->galaxy(), queryContainer.mId);
                }
            }
        }
        mDatabase->destroyDataBinding(binding);
    }
    break;

    case FFQuery_AttributeData:
    {

        FactoryObject* factory = dynamic_cast<FactoryObject*>(asyncContainer->mObject);
        //_buildAttributeMap(harvester,result);

        Attribute_QueryContainer	attribute;
        uint64						count = result->getRowCount();
        //int8						str[256];
        //BStringVector				dataElements;

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(mAttributeBinding,(void*)&attribute);
            factory->addInternalAttribute(attribute.mKey,std::string(attribute.mValue.getAnsi()));
        }


        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,FFQuery_Hopper,asyncContainer->mClient);
        asContainer->mObject = factory;

        mDatabase->executeSqlAsync(this,asContainer, "(SELECT \'input\',id FROM %s.items WHERE parent_id = %" PRIu64 " AND item_type = 2773) "
                                                     "UNION (SELECT \'output\',id FROM %s.items WHERE parent_id = %" PRIu64 " AND item_type = 2774)",
                                                     mDatabase->galaxy(),factory->getId(),
                                                     mDatabase->galaxy(),factory->getId());
    }
    break;

    case FFQuery_Hopper:
    {

        FactoryObject* factory = dynamic_cast<FactoryObject*>(asyncContainer->mObject);

        Type1_QueryContainer queryContainer;

        DataBinding*	binding = mDatabase->createDataBinding(2);
        binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
        binding->addField(DFT_uint64,offsetof(Type1_QueryContainer,mId),8,1);

        uint64 count = result->getRowCount();

        //InLoadingContainer* ilc = new(mILCPool.ordered_malloc()) InLoadingContainer(inventory,asyncContainer->mOfCallback,asyncContainer->mClient);
        //ilc->mLoadCounter = count;

        //mObjectLoadMap.insert(std::make_pair(datapad->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(datapad,asyncContainer->mOfCallback,asyncContainer->mClient,static_cast<uint32>(count))));

        mObjectLoadMap.insert(std::make_pair(factory->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(factory,asyncContainer->mOfCallback,asyncContainer->mClient,2)));

        for(uint32 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&queryContainer);

            if(strcmp(queryContainer.mString.getAnsi(),"input") == 0)
            {
                gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Hopper,0,NULL);
            }

            else if(strcmp(queryContainer.mString.getAnsi(),"output") == 0)
            {
                gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Hopper,0,NULL);
            }


        }

        mDatabase->destroyDataBinding(binding);

    }
    break;

    case FFQuery_MainData:
    {
        QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,FFQuery_AttributeData,asyncContainer->mClient,asyncContainer->mId);


        FactoryObject* factory = new(FactoryObject);
        _createFactory(result,factory);

        asynContainer->mObject = factory;
        asynContainer->mClient = asyncContainer->mClient;
        asynContainer->mId		= factory->getId();


        mDatabase->executeSqlAsync(this,asynContainer,"SELECT attributes.name,sa.value,attributes.internal"
                                   " FROM %s.structure_attributes sa"
                                   " INNER JOIN %s.attributes ON (sa.attribute_id = attributes.id)"
                                   " WHERE sa.structure_id = %" PRIu64 " ORDER BY sa.order",
                                   mDatabase->galaxy(),mDatabase->galaxy(),factory->getId());


    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================


//=============================================================================

void FactoryFactory::_createFactory(DatabaseResult* result, FactoryObject* factory)
{
    if (!result->getRowCount()) {
        return;
    }

    result->getNextRow(mFactoryBinding,factory);

    factory->setLoadState(LoadState_Loaded);
    factory->setType(ObjType_Structure);
    factory->mCustomName.convert(BSTRType_Unicode16);
    factory->setCapacity(2); // we want to load 2 hoppers!
}

//=============================================================================

void FactoryFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    //request the harvesters Data first

    int8 sql[1024];
    sprintf(sql,	"SELECT s.id,s.owner,s.oX,s.oY,s.oZ,s.oW,s.x,s.y,s.z,"
            "std.type,std.object_string,std.stf_name, std.stf_file, s.name,"
            "std.lots_used, f.active, std.maint_cost_wk, std.power_used, std.schematicMask, s.condition, std.max_condition, f.ManSchematicId "
            "FROM %s.structures s INNER JOIN %s.structure_type_data std ON (s.type = std.type) INNER JOIN %s.factories f ON (s.id = f.id) "
            "WHERE (s.id = %" PRIu64 ")",
            mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),id);
    QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,FFQuery_MainData,client,id);

    mDatabase->executeSqlAsync(this,asynContainer,sql);
}

//the factories hopper is accessed - update the hoppers contents
void FactoryFactory::upDateHopper(ObjectFactoryCallback* ofCallback,uint64 hopperId, DispatchClient* client, FactoryObject* factory )
{
    QueryContainerBase* asynContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,FFQuery_HopperUpdate,client,hopperId);
    asynContainer->mObject = factory;
    asynContainer->mHopper = hopperId;

    mDatabase->executeSqlAsync(this,asynContainer, "(SELECT \'item\',id FROM %s.items WHERE parent_id = %" PRIu64 ") "
                                                   "UNION (SELECT \'resource\',id FROM %s.resource_containers WHERE parent_id = %" PRIu64 ")",
                                                   mDatabase->galaxy(),hopperId,
                                                   mDatabase->galaxy(),hopperId);

}
//=============================================================================

void FactoryFactory::_setupDatabindings()
{
    mFactoryBinding = mDatabase->createDataBinding(22);
    mFactoryBinding->addField(DFT_uint64,offsetof(FactoryObject,mId),8,0);
    mFactoryBinding->addField(DFT_uint64,offsetof(FactoryObject,mOwner),8,1);
    mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.x),4,2);
    mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.y),4,3);
    mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.z),4,4);
    mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mDirection.w),4,5);
    mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mPosition.x),4,6);
    mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mPosition.y),4,7);
    mFactoryBinding->addField(DFT_float,offsetof(FactoryObject,mPosition.z),4,8);

    mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mFactoryFamily),4,9);//thats the structure_type_data ID
    mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mModel),256,10);
    mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mName),256,11);
    mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mNameFile),256,12);
    mFactoryBinding->addField(DFT_bstring,offsetof(FactoryObject,mCustomName),256,13);

    mFactoryBinding->addField(DFT_uint8,offsetof(FactoryObject,mLotsUsed),1,14);
    mFactoryBinding->addField(DFT_uint8,offsetof(FactoryObject,mActive),1,15);
    mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,maint_cost_wk),4,16);
    mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mPowerUsed),4,17);

    mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mSchematicMask),4,18);

    mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mDamage),4,19);
    mFactoryBinding->addField(DFT_uint32,offsetof(FactoryObject,mMaxCondition),4,20);
    mFactoryBinding->addField(DFT_uint64,offsetof(FactoryObject,mManSchematicID),8,21);
}

//=============================================================================

void FactoryFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mFactoryBinding);
}

//=============================================================================

void FactoryFactory::handleObjectReady(Object* object,DispatchClient* client)
{
    //On serverstartup or factory create used to load in and out put hopper on factory create and to load hopper content
    //On runtime used to load hopper content when we access a hopper or to create a new factory
    //ILC ID on hoppercontent load is the hoppers ID

    InLoadingContainer* ilc = _getObject(object->getParentId());
    if(!ilc) { //Crashbug patch for: http://paste.swganh.org/viewp.php?id=20100627071644-6c8c2b45ecb37f7914372484cd105bfe
        LOG(WARNING) << "No InLoadingContainer found for object parent[" << object->getParentId() << "]";
        return;
    }
    FactoryObject*		factory = dynamic_cast<FactoryObject*>(ilc->mObject);
    if(!factory)
    {
        factory = NULL;

    }

    //add hopper / new item to worldObjectlist, but NOT to the SI
    gWorldManager->addObject(object,true);

    //do we have a valid Object?
    TangibleObject* tangible = dynamic_cast<TangibleObject*>(object);
    if(!tangible)
    {
        LOG(WARNING) << "Not tangible on handleObjectReady!";
        return;
    }

    uint64 parent = 0;
    if(strcmp(tangible->getName().getAnsi(),"ingredient_hopper")==0)
    {
        factory->setIngredientHopper(object->getId());
        factory->addObject(object);
    }
    else if(strcmp(tangible->getName().getAnsi(),"output_hopper")==0)
    {
        factory->setOutputHopper(object->getId());
        factory->addObject(object);
    }
    else
    {
        //its a tangible item of a hopper read in during runtime
        TangibleObject* hopper;
        if(factory->getOutputHopper() == tangible->getParentId())
        {
            hopper = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(factory->getOutputHopper()));
        }
        else
        {
            hopper = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(factory->getIngredientHopper()));
        }

        if(!hopper)
        {
            LOG(ERR) << "OutputHopper not found on item load";
            assert(false && "FactoryFactory::handleObjectReady WorldManager could not find output hopper");
        }

        parent = hopper->getId();

        //add to hopper / create for players
        hopper->handleObjectReady(object,NULL);
    }



    if(( --ilc->mLoadCounter) == 0)
    {
        if(!(_removeFromObjectLoadMap(object->getParentId())))
            LOG(WARNING) << "Failed removing object from loadmap";

        factory->setLoadState(LoadState_Loaded);
        if(!parent)			   //factories dont have a parent! main cell is 0!!!
            ilc->mOfCallback->handleObjectReady(factory,ilc->mClient);
        else
            ilc->mOfCallback->handleObjectReady(factory,ilc->mClient,parent);	//only hoppers have a parent (the factory)

        mILCPool.free(ilc);
    }

}

//=============================================================================

void FactoryFactory::releaseAllPoolsMemory()
{
    releaseQueryContainerPoolMemory();
    releaseILCPoolMemory();

}
