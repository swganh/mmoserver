/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_RESOURCECOLLECTIONMANAGER_H
#define ANH_ZONESERVER_RESOURCECOLLECTIONMANAGER_H

#include <boost/pool/pool.hpp>
#include <glm/glm.hpp>

#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include "DatabaseManager/DatabaseCallback.h"


#define	 gResourceCollectionManager		ResourceCollectionManager::getSingletonPtr()

//======================================================================================================================

class Database;
class DatabaseCallback;
class DatabaseResult;
class DataBinding;
class UIWindow;

namespace Anh_Utils
{
class Clock;
}

//======================================================================================================================

enum RCMQueryType
{
    RCMQuery_SampleCosts		= 1,
    RCMQuery_SurveyCosts		= 2
};

//======================================================================================================================

class RCMAsyncContainer
{
public:

    RCMAsyncContainer(RCMQueryType qt) {
        mQueryType = qt;
    }
    ~RCMAsyncContainer() {}

    RCMQueryType	mQueryType;
};

//======================================================================================================================
class ResourceCollectionManager : public DatabaseCallback
{
public:

    ~ResourceCollectionManager();
    static ResourceCollectionManager*		Init(Database* database);
    static ResourceCollectionManager*		getSingletonPtr() {
        return mSingleton;
    }

    virtual void							handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    void									releaseAllPoolsMemory() {
        mDBAsyncPool.release_memory();
    }

    uint32									surveyHealthCost;
    uint32									surveyActionCost;
    uint32									surveyMindCost;

    uint32									sampleRadioactiveDamageModifier;
    uint32									sampleHealthCost;
    uint32									sampleActionCost;
    uint32									sampleMindCost;

private:

    ResourceCollectionManager(Database* database);

    void									handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window);
    void									_setupDatabindings();
    void									_destroyDatabindings();

    static bool								mInsFlag;
    static ResourceCollectionManager*		mSingleton;

    Database*								mDatabase;

    boost::pool<boost::default_user_allocator_malloc_free>				mDBAsyncPool;

    DataBinding*							mCommandCostBinding;
};

#endif
