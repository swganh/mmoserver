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

#include "CreatureFactory.h"

#ifdef _WIN32
#undef ERROR
#endif
#include <glog/logging.h>

#include "CreatureEnums.h"
#include "ObjectFactoryCallback.h"
#include "PersistentNpcFactory.h"
#include "ShuttleFactory.h"

#include "Utils/utils.h"

#include <assert.h>

//=============================================================================

bool					CreatureFactory::mInsFlag    = false;
CreatureFactory*		CreatureFactory::mSingleton  = NULL;

//======================================================================================================================

CreatureFactory*	CreatureFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new CreatureFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

CreatureFactory::CreatureFactory(Database* database) : FactoryBase(database)
{
    mPersistentNpcFactory	= PersistentNpcFactory::Init(mDatabase);
    mShuttleFactory			= ShuttleFactory::Init(mDatabase);
}

//=============================================================================

CreatureFactory::~CreatureFactory()
{
    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void CreatureFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    switch(subGroup)
    {
    case CreoGroup_PersistentNpc:
        mPersistentNpcFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;
    case CreoGroup_Shuttle:
        mShuttleFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;

    default:
        LOG(ERROR) << "Unknown Group";
        break;
    }
}

//=============================================================================

void CreatureFactory::releaseAllPoolsMemory()
{
    mPersistentNpcFactory->releaseQueryContainerPoolMemory()	;
    mShuttleFactory->releaseQueryContainerPoolMemory();
}

//=============================================================================

