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

#include "ContainerObjectFactory.h"


#include "Utils/logger.h"

#include "Container.h"
#include "ContainerManager.h"
#include "CreatureObject.h"
#include "ObjectFactoryCallback.h"
#include "PlayerObject.h"
#include "TangibleFactory.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "NetworkManager/DispatchClient.h"

#include "Utils/utils.h"

//=============================================================================

ContainerObjectFactory*	ContainerObjectFactory::mSingleton  = NULL;

//======================================================================================================================

ContainerObjectFactory*	ContainerObjectFactory::Init(Database* database)
{
    if(!mSingleton)
    {
        mSingleton = new ContainerObjectFactory(database);
    }
    return mSingleton;
}

//=============================================================================

ContainerObjectFactory::ContainerObjectFactory(Database* database) : FactoryBase(database)
{
    _setupDatabindings();
}

//=============================================================================

ContainerObjectFactory::~ContainerObjectFactory()
{
    _destroyDatabindings();
}

//=============================================================================

void ContainerObjectFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case CFQuery_MainData:
    {
        Container* container = _createContainer(result);

        // If not a player as parent, we will not have to send any create updates.
        PlayerObject* player = NULL;
        if (asyncContainer->mClient)
        {
            player = gWorldManager->getPlayerByAccId(asyncContainer->mClient->getAccountId());
        }
        container->setParent(player);

        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,CFQuery_ObjectCount,asyncContainer->mClient);
        asContainer->mObject = container;

        mDatabase->executeSqlAsync(this,asContainer,"SELECT %s.sf_getInventoryObjectCount(%" PRIu64 ")",mDatabase->galaxy(),container->getId());
        
    }
    break;

    case CFQuery_ObjectCount:
    {
        Container* container  = dynamic_cast<Container*>(asyncContainer->mObject);

        uint32 objectCount;
        DataBinding* binding = mDatabase->createDataBinding(1);

        binding->addField(DFT_uint32,0,4);
        result->getNextRow(binding,&objectCount);

        //container->setObjectLoadCounter(objectCount);

        if (objectCount != 0)
        {
            uint64 containerId = container->getId();

            container->setLoadState(LoadState_Loading);

            // query contents
            QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,CFQuery_Objects,asyncContainer->mClient);
            asContainer->mObject = container;

            mDatabase->executeSqlAsync(this,asContainer,
                                       "(SELECT \'containers\',containers.id FROM %s.containers INNER JOIN %s.container_types ON (containers.container_type = container_types.id)"
                                       " WHERE (container_types.name NOT LIKE 'unknown') AND (containers.parent_id = %" PRIu64 "))"
                                       " UNION (SELECT \'items\',items.id FROM %s.items WHERE (parent_id=%" PRIu64 "))"
                                       " UNION (SELECT \'resource_containers\',resource_containers.id FROM %s.resource_containers WHERE (parent_id=%" PRIu64 "))",
                                       mDatabase->galaxy(),mDatabase->galaxy(),
                                       containerId,
                                       mDatabase->galaxy(), containerId,
                                       mDatabase->galaxy(),containerId);

        }
        else
        {
            container->setLoadState(LoadState_Loaded);
            asyncContainer->mOfCallback->handleObjectReady(container,asyncContainer->mClient);
        }

        mDatabase->destroyDataBinding(binding);
    }
    break;

    case CFQuery_Objects:
    {
        Container* container = dynamic_cast<Container*>(asyncContainer->mObject);

        Type1_QueryContainer queryContainer;

        DataBinding* binding = mDatabase->createDataBinding(2);
        binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
        binding->addField(DFT_uint64,offsetof(Type1_QueryContainer,mId),8,1);

        uint64 count = result->getRowCount();

        mObjectLoadMap.insert(std::make_pair(container->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(container,asyncContainer->mOfCallback,asyncContainer->mClient,static_cast<uint32>(count))));

        for(uint32 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&queryContainer);

            if(strcmp(queryContainer.mString.getAnsi(),"containers") == 0)
                gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Container,0,asyncContainer->mClient);
            else if(strcmp(queryContainer.mString.getAnsi(),"items") == 0)
                gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_Item,0,asyncContainer->mClient);
            else if(strcmp(queryContainer.mString.getAnsi(),"resource_containers") == 0)
                gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_ResourceContainer,0,asyncContainer->mClient);
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

void ContainerObjectFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    if (!gWorldManager->getObjectById(id))
    {
        // The container does not exist.
        mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,CFQuery_MainData,client),
                                   "SELECT containers.id,containers.parent_id,containers.oX,containers.oY,containers.oZ,containers.oW,containers.x,"
                                   "containers.y,containers.z,containers.container_type,container_types.object_string,container_types.name,container_types.file,container_types.details_file"
                                   " FROM containers INNER JOIN container_types ON (containers.container_type = container_types.id)"
                                   " WHERE (containers.id = %" PRIu64 ")",id);
       
    }
    else
    {
        // Just update the contence, if needed.

        // If not a player as parent, we will not have to send any create updates.
        PlayerObject* player = NULL;
        if (client)
        {
            player = gWorldManager->getPlayerByAccId(client->getAccountId());
        }
        Object* object = gWorldManager->getObjectById(id);
        if (object->getType() == ObjType_Tangible)
        {
            TangibleObject* tangObj = dynamic_cast<TangibleObject*>(object);
            if (tangObj->getTangibleGroup() == TanGroup_Container)
            {
                Container* container = dynamic_cast<Container*>(tangObj);
                container->setParent(player);

                QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,CFQuery_ObjectCount,client);
                asContainer->mObject = container;

                mDatabase->executeSqlAsync(this,asContainer,"SELECT %s.sf_getInventoryObjectCount(%" PRIu64 ")",mDatabase->galaxy(),container->getId());
                
            }
        }
    }
}
//=============================================================================

Container* ContainerObjectFactory::_createContainer(DatabaseResult* result)
{
    if (!result->getRowCount()) {
    	return nullptr;
    }

    Container*	container = new Container();

    result->getNextRow(mContainerBinding,(void*)container);

    // container->setLoadState(LoadState_Loaded);

    return container;
}

//=============================================================================

void ContainerObjectFactory::_setupDatabindings()
{
    mContainerBinding = mDatabase->createDataBinding(14);
    mContainerBinding->addField(DFT_uint64,offsetof(Container,mId),8,0);
    mContainerBinding->addField(DFT_uint64,offsetof(Container,mParentId),8,1);
    mContainerBinding->addField(DFT_float,offsetof(Container,mDirection.x),4,2);
    mContainerBinding->addField(DFT_float,offsetof(Container,mDirection.y),4,3);
    mContainerBinding->addField(DFT_float,offsetof(Container,mDirection.z),4,4);
    mContainerBinding->addField(DFT_float,offsetof(Container,mDirection.w),4,5);
    mContainerBinding->addField(DFT_float,offsetof(Container,mPosition.x),4,6);
    mContainerBinding->addField(DFT_float,offsetof(Container,mPosition.y),4,7);
    mContainerBinding->addField(DFT_float,offsetof(Container,mPosition.z),4,8);
    mContainerBinding->addField(DFT_uint32,offsetof(Container,mTanType),4,9);
    mContainerBinding->addField(DFT_bstring,offsetof(Container,mModel),256,10);
    mContainerBinding->addField(DFT_bstring,offsetof(Container,mName),64,11);
    mContainerBinding->addField(DFT_bstring,offsetof(Container,mNameFile),64,12);
    mContainerBinding->addField(DFT_bstring,offsetof(Container,mDetailFile),64,13);
}

//=============================================================================

void ContainerObjectFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mContainerBinding);
}

//=============================================================================

void ContainerObjectFactory::handleObjectReady(Object* object,DispatchClient* client)
{
    InLoadingContainer* ilc	= _getObject(object->getParentId());
    if(!ilc) { //Crashbug patch: http://paste.swganh.org/viewp.php?id=20100627024455-d7efda0b4aebaa96b06438b2c42dfe6c
        LOG(WARNING) << "Failed to locate InLoadingContainer for parent id [" << object->getParentId() << "]";
        return;
    }
    ilc->mLoadCounter--;

    Container* container = dynamic_cast<Container*>(ilc->mObject);

    // reminder: objects are owned by the global map, containers only keeps references

    // If object with same key already exist in world map, this object will be invalid.
    if (!gWorldManager->existObject(object))
    {

		container->addObject(object);

        gWorldManager->addObject(object,true);
		
		//update watchers
		gContainerManager->createObjectToRegisteredPlayers(container, object);

    }

    // if (container->getObjectLoadCounter() == (container->getObjects())->size())
    if(!ilc->mLoadCounter)
    {
        container->setLoadState(LoadState_Loaded);

        if (!(_removeFromObjectLoadMap(container->getId())))
            LOG(WARNING) << "Failed removing object from loadmap";

        ilc->mOfCallback->handleObjectReady(container,ilc->mClient);

        mILCPool.free(ilc);
    }
}


