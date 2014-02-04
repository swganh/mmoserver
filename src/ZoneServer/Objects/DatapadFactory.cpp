/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "DatapadFactory.h"

#include "anh/logger.h"

#include "Zoneserver/Objects/Datapad.h"
#include "Zoneserver/objects/IntangibleObject.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "Zoneserver/Objects/Item.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/ManufacturingSchematic.h"
#include "ZoneServer/Objects/ObjectFactoryCallback.h"
#include "ZoneServer/Objects/Tangible Object/TangibleFactory.h"
#include "Zoneserver/Objects/VehicleControllerFactory.h"
#include "Zoneserver/Objects/waypoints/WaypointFactory.h"
#include "Zoneserver/Objects/waypoints/WaypointObject.h"
#include "ZoneServer/WorldManager.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include <cppconn/resultset.h>
#include "Utils/utils.h"

//=============================================================================

bool				DatapadFactory::mInsFlag    = false;
DatapadFactory*		DatapadFactory::mSingleton  = NULL;

//======================================================================================================================

DatapadFactory*	DatapadFactory::Init(swganh::app::SwganhKernel*	kernel)
{
    if(!mInsFlag)
    {
        mSingleton = new DatapadFactory(kernel);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

DatapadFactory::DatapadFactory(swganh::app::SwganhKernel*	kernel) : FactoryBase(kernel)
{
    mWaypointFactory = WaypointFactory::Init(kernel);

    _setupDatabindings();
}

//=============================================================================

DatapadFactory::~DatapadFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void DatapadFactory::handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case DPFQuery_MainDatapadData:
    {
        //get the count of all Waypoints and Schematics
        Datapad* datapad = _createDatapad(result);

        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,DPFQuery_ObjectCount,asyncContainer->mClient);
        asContainer->mObject = datapad;

		std::stringstream sql;

		sql << "SELECT " << mDatabase->galaxy() << ".sf_getDatapadObjectCount(" << datapad->getId() << ");";
        mDatabase->executeSqlAsync(this,asContainer, sql.str());
       
    }
    break;

	//this gets solely called on player load
    case DPFQuery_ObjectCount:
    {
        Datapad* datapad = dynamic_cast<Datapad*>(asyncContainer->mObject);

        uint32 objectCount;
        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);

        binding->addField(swganh::database::DFT_uint32,0,4);
        result->getNextRow(binding,&objectCount);

        datapad->setObjectLoadCounter(objectCount);

        if(objectCount != 0)
        {
            uint64 dtpId = datapad->getId();

            datapad->setLoadState(LoadState_Loading);

            // query contents
            QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,DPFQuery_Objects,asyncContainer->mClient);
            asContainer->mObject = datapad;

			std::stringstream sql;

			sql << "(SELECT \'waypoints\',waypoints.waypoint_id FROM " << mDatabase->galaxy() << ".waypoints WHERE owner_id = " << dtpId-DATAPAD_OFFSET << ")"
				<< " UNION (SELECT \'manschematics\',items.id FROM " << mDatabase->galaxy() << ".items WHERE (parent_id=" << dtpId << "))"
				<< " UNION (SELECT \'vehicles\',vehicles.id FROM " << mDatabase->galaxy() << ".vehicles WHERE (parent=" << dtpId << "))";

            mDatabase->executeSqlAsync(this,asContainer, sql.str());

        }
        else
        {
            datapad->setLoadState(LoadState_Loaded);
            asyncContainer->mOfCallback->handleObjectReady(datapad,asyncContainer->mClient);
        }

        mDatabase->destroyDataBinding(binding);
    }
    break;

    case DPFQuery_ItemId:
    {
        uint64 id;
        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);

        binding->addField(swganh::database::DFT_uint64,0,8);
        result->getNextRow(binding,&id);

        gTangibleFactory->requestObject(this,id,TanGroup_Item,0,asyncContainer->mClient);
        mDatabase->destroyDataBinding(binding);
    }
    break;

    case DPFQuery_MSParent:
    {
        uint64 id;
        
		if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(warning) << "DatapadFactory::handleDatabaseJobComplete  Unable to load Schematic IDs";
			mQueryContainerPool.free(asyncContainer);
            return;
        }

        id = result_set->getUInt64(1);
        
        PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(id-3));
        if(!player)
        {
            //factoryPanic!!!!!!!!
        	LOG(warning) << "DatapadFactory::handleDatabaseJobComplete Failed getting player to create MS";
			mQueryContainerPool.free(asyncContainer);
            return;
        }

        Datapad* datapad = player->getDataPad();

        if(!datapad)
        {
            //factoryPanic!!!!!!!!
        	LOG(error) << "DatapadFactory::handleDatabaseJobComplete Failed getting datapad to create Manufacturing Schematic";
			mQueryContainerPool.free(asyncContainer);
            return;
        }
        mObjectLoadMap.insert(std::make_pair(datapad->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(datapad,datapad,NULL,1)));
        gTangibleFactory->requestObject(this,asyncContainer->mId,TanGroup_Item,0,NULL);

    }
    break;

    case DPFQuery_Objects:
    {
        Datapad* datapad = dynamic_cast<Datapad*>(asyncContainer->mObject);
        datapad->mWaypointUpdateCounter = 0;
        datapad->mManSUpdateCounter = 0;

        Type1_QueryContainer queryContainer;

        swganh::database::DataBinding*	binding = mDatabase->createDataBinding(2);
        binding->addField(swganh::database::DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
        binding->addField(swganh::database::DFT_uint64,offsetof(Type1_QueryContainer,mId),8,1);

        uint64 count = result->getRowCount();

        //InLoadingContainer* ilc = new(mILCPool.ordered_malloc()) InLoadingContainer(inventory,asyncContainer->mOfCallback,asyncContainer->mClient);
        //ilc->mLoadCounter = count;

        mObjectLoadMap.insert(std::make_pair(datapad->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(datapad,asyncContainer->mOfCallback,asyncContainer->mClient,static_cast<uint32>(count))));

        for(uint32 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&queryContainer);

            if(strcmp(queryContainer.mString.getAnsi(),"waypoints") == 0)
            {
                ++datapad->mWaypointUpdateCounter;
                mWaypointFactory->requestObject(this,queryContainer.mId,0,0,asyncContainer->mClient);
            }

            else if(strcmp(queryContainer.mString.getAnsi(),"manschematics") == 0)
            {
                ++datapad->mManSUpdateCounter;
                gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Item,0,asyncContainer->mClient);

            }
            else if(strcmp(queryContainer.mString.getAnsi(),"vehicles") == 0)
            {
                //datapad counter gets updated in vehicle factory
                gVehicleControllerFactory->requestObject(this,queryContainer.mId,0,0,asyncContainer->mClient);

            }

        }

        mDatabase->destroyDataBinding(binding);
    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);

}

//=============================================================================
//a Manufacturing Schematic will be loaded into the datapad
//
void DatapadFactory::requestManufacturingSchematic(ObjectFactoryCallback* ofCallback, uint64 id)
{

    QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,DPFQuery_MSParent,NULL);
    asContainer->mId = id;

	std::stringstream sql;
	sql << "SELECT items.parent_id FROM " << mDatabase->galaxy()
		<< ".items WHERE id = " << id << ";";
    
	mDatabase->executeSqlAsync(this, asContainer, sql.str());
}


//=============================================================================
//
void DatapadFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,DPFQuery_MainDatapadData,client),
                               "SELECT datapads.id,datapad_types.object_string,datapad_types.name,datapad_types.file"
                               " FROM %s.datapads INNER JOIN %s.datapad_types ON (datapads.datapad_type = datapad_types.id)"
                               " WHERE (datapads.id = %"PRIu64")",mDatabase->galaxy(),mDatabase->galaxy(), id);
  
}

//=============================================================================

Datapad* DatapadFactory::_createDatapad(swganh::database::DatabaseResult* result)
{
    if (!result->getRowCount()) {
    	return nullptr;
    }

    Datapad* datapad = new Datapad();

    // get our results
    result->getNextRow(mDatapadBinding,(void*)datapad);
    datapad->setParentId(datapad->mId - 3);

	gWorldManager->addObject(datapad, true);

	return datapad;
}

//=============================================================================

void DatapadFactory::_setupDatabindings()
{
    // datapad binding
    mDatapadBinding = mDatabase->createDataBinding(4);
    mDatapadBinding->addField(swganh::database::DFT_uint64,offsetof(Datapad,mId),8,0);
    mDatapadBinding->addField(swganh::database::DFT_bstring,offsetof(Datapad,mModel),256,1);
    mDatapadBinding->addField(swganh::database::DFT_bstring,offsetof(Datapad,mName),64,2);
    mDatapadBinding->addField(swganh::database::DFT_bstring,offsetof(Datapad,mNameFile),64,3);
}

//=============================================================================

void DatapadFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mDatapadBinding);
}

//=============================================================================

void DatapadFactory::handleObjectReady(std::shared_ptr<Object> object)
{
	Datapad* datapad(0);
    uint64 theID(0);

    switch(object->getType())
    {
		case ObjType_Waypoint:
		{
			
			uint64 datapad_id 	= object->getParentId()+DATAPAD_OFFSET;
			mIlc	= _getObject(datapad_id);
			datapad = dynamic_cast<Datapad*>(mIlc->mObject);

			mIlc->mLoadCounter--;

			datapad->AddWaypoint(std::dynamic_pointer_cast<WaypointObject>(object));
		}
		break;
	}

	if(!mIlc)	{
		LOG (error) << "DatapadFactory::handleObjectReady : no milc container!!!!!!!";
        return;
	}

    if(!(mIlc->mLoadCounter))
    {
        if(!(_removeFromObjectLoadMap(theID)))
        	LOG(warning) << "DatapadFactory::handleObjectReady  Failed removing object from loadmap";

        mIlc->mOfCallback->handleObjectReady(datapad,mIlc->mClient);

        mILCPool.free(mIlc);
    }

}

void DatapadFactory::handleObjectReady(Object* object,DispatchClient* client)
{

    Datapad* datapad(0);
    uint64 theID(0);

    switch(object->getType())
    {
    
    case ObjType_Tangible:
    {

        Item* item = dynamic_cast<Item*>(object);
        if(item->getItemType()== ItemType_ManSchematic)
        {
            theID	= object->getParentId();
            mIlc	= _getObject(theID);

            if(!mIlc)
            {
            	LOG(warning) << "Failed getting ilc during ObjType_Tangible where ItemType_ManSchematic";
                return;
            }

            datapad = dynamic_cast<Datapad*>(mIlc->mObject);

            //parentId of schematics is the datapad!
            //add the msco to the datapad

            datapad->addManufacturingSchematic(dynamic_cast<ManufacturingSchematic*>(object));

            //now load the associated item
            QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(this,DPFQuery_ItemId,NULL);
            asContainer->mObject = datapad;
            asContainer->mId = item->getId();//queryContainer.mId;
            int8 sql[256];
            sprintf(sql,"SELECT items.id FROM %s.items WHERE (parent_id=%"PRIu64")",mDatabase->galaxy(),item->getId());
            mDatabase->executeSqlAsync(this,asContainer,sql);

            mObjectLoadMap.insert(std::make_pair(item->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(datapad,0,0,1)));

        }
        else
        {
            uint64 id						= object->getParentId();
            //we are loading the corresponding tangible object to have all the items attributes available for display/usage
            //find the corresponding schematic

            //temp mILc
            InLoadingContainer*mIlcDPad		= _getObject(id);
            if(!mIlcDPad)
            {
            	LOG(warning) << "Failed getting mIlcDPad during ObjType_Tangible";
                return;
            }
            datapad							= dynamic_cast<Datapad*>(mIlcDPad->mObject);
            _removeFromObjectLoadMap(id);
            mILCPool.free(mIlcDPad);

            //regular mIlc
            theID							= datapad->getId();
            mIlc							= _getObject(theID);

            if(!mIlc)
            {
            	LOG(warning) << "Failed getting ilc during ObjType_Tangible";
                return;
            }

            ManufacturingSchematic* schem	= datapad->getManufacturingSchematicById(id);

            //this is the item associated to the Man schematic
            //set the man schematic pointer and decrease the loadcount
            mIlc->mLoadCounter--;
            schem->setItem(dynamic_cast<Item*>(object));
        }


    }
    break;

    case ObjType_Intangible:
    {
        theID	= object->getParentId();
        mIlc	= _getObject(theID);
        if(!mIlc)//sanity
        {
            LOG(warning) << "Failed getting ilc during ObjType_Intangible";
            return;
        }

        if((datapad = dynamic_cast<Datapad*>(mIlc->mObject)))
        {
            mIlc->mLoadCounter--;

            if(IntangibleObject* itno = dynamic_cast<IntangibleObject*>(object))
            {
                if(datapad->getCapacity())
                {
                    datapad->addData(itno);
                    Object* ob = gWorldManager->getObjectById(object->getId());
                    if(!ob)
                        gWorldManager->addObject(object,true);
                }
                else
                {
                	LOG(warning) << "Datapad at max Capacity";
                    delete(object);
                }
            }
        }

    }
    break;
    default:
        break;
    }

    if(!mIlc)
        return;

    if(!(mIlc->mLoadCounter))
    {
        if(!(_removeFromObjectLoadMap(theID)))
        	LOG(warning) << "Failed removing object from loadmap";

        mIlc->mOfCallback->handleObjectReady(datapad,mIlc->mClient);

        mILCPool.free(mIlc);
    }
}

//=============================================================================

