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

#ifndef ANH_ZONESERVER_FACTORY_BASE_H
#define ANH_ZONESERVER_FACTORY_BASE_H

#include <map>
#include <boost/pool/pool.hpp>

#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include "DatabaseManager/DatabaseCallback.h"

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class InLoadingContainer;
class Type1_QueryContainer;
class Object;
class Item;
class QueryContainerBase;
class SpawnData;

//=============================================================================

typedef std::map<uint64,InLoadingContainer*>	ObjectLoadMap;

//=============================================================================

class FactoryBase : public DatabaseCallback
{
public:

    FactoryBase(Database* database);

    virtual void	handleDatabaseJobComplete(void* ref,DatabaseResult* result) = 0;
    virtual void	requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client) = 0;

    virtual			~FactoryBase();

    bool			releaseQueryContainerPoolMemory() {
        return(mQueryContainerPool.release_memory());
    }
    bool			releaseILCPoolMemory() {
        return(mILCPool.release_memory());
    }

protected:

    void				_buildAttributeMap(Object* object,DatabaseResult* result);

    InLoadingContainer* _getObject(uint64 id);
    bool				_removeFromObjectLoadMap(uint64 id);

    boost::pool<boost::default_user_allocator_malloc_free>		mILCPool;
    boost::pool<boost::default_user_allocator_malloc_free>		mQueryContainerPool;

    ObjectLoadMap			mObjectLoadMap;

    DataBinding*			mAttributeBinding;
    Database*				mDatabase;
};

//=============================================================================

class NonPersistantQueryContainerBase
{
public:

    NonPersistantQueryContainerBase(ObjectFactoryCallback* ofCallback,uint32 queryType,DispatchClient* client,Item* newItem) :
        mClient(client),mOfCallback(ofCallback),mQueryType(queryType),mItem(newItem) {}

    DispatchClient*			mClient;
    ObjectFactoryCallback*	mOfCallback;
    Object*					mObject;
    uint32					mQueryType;
    Item*					mItem;
};

//=============================================================================

class QueryNonPersistentItemFactory
{
public:

    QueryNonPersistentItemFactory(ObjectFactoryCallback* ofCallback,uint32 queryType, uint64 id = 0) :
        mOfCallback(ofCallback),mQueryType(queryType),mId(id) {}

    ObjectFactoryCallback*	mOfCallback;
    Object*					mObject;
    uint32					mQueryType;
    uint64					mId;
};

//=============================================================================

class QueryContainerBase
{
public:

    QueryContainerBase(ObjectFactoryCallback* ofCallback,uint32 queryType,DispatchClient* client, uint64 id = 0) :
        mClient(client),mOfCallback(ofCallback),mQueryType(queryType),mId(id) {}

    DispatchClient*			mClient;
    ObjectFactoryCallback*	mOfCallback;
    Object*					mObject;
    uint32					mQueryType;
    uint64					mId;
    uint64					mHopper;
    uint32					mDepth;
};

//=============================================================================

class InLoadingContainer
{
public:

    InLoadingContainer(Object* object,ObjectFactoryCallback* ofCallback,DispatchClient* client,uint32 loadCounter = 1)
        : mObject(object),mOfCallback(ofCallback),mClient(client),mLoadCounter(loadCounter), mManSchem(false) {}

    Object*					mObject;
    ObjectFactoryCallback*	mOfCallback;
    DispatchClient*			mClient;
    uint32					mLoadCounter;
    bool					mManSchem;
    bool					mDPad;
    bool					mInventory;
};

//=============================================================================

class Type1_QueryContainer
{
public:

    Type1_QueryContainer() {}
    uint32	mVolume;
    uint64	mId;
    BString	mString;

};

//=============================================================================

class Attribute_QueryContainer
{
public:

    Attribute_QueryContainer() {}

    BString	mKey;
    BString	mValue;
    uint8	mInternal;
};

//=============================================================================

enum NonPersistentQuery
{
    NPQuery_MainData	= 1,
    NPQuery_Attributes	= 2
};

//=============================================================================

#endif
