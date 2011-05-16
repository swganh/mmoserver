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

#include "TangibleFactory.h"
#include "ContainerObjectFactory.h"
#include "ItemFactory.h"
#include "DatapadFactory.h"

#include "ResourceContainerFactory.h"
#include "TangibleEnums.h"
#include "TerminalFactory.h"
#include "TicketCollectorFactory.h"
#include "Utils/utils.h"

#include <assert.h>

// Fix for issues with glog redefining this constant
#ifdef ERROR
#undef ERROR
#endif

#include <glog/logging.h>
//=============================================================================

bool					TangibleFactory::mInsFlag    = false;
TangibleFactory*		TangibleFactory::mSingleton  = NULL;

//======================================================================================================================

TangibleFactory*	TangibleFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new TangibleFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

TangibleFactory::TangibleFactory(Database* database) : FactoryBase(database)
{
    mContainerFactory			= ContainerObjectFactory::Init(mDatabase);
    mTerminalFactory			= TerminalFactory::Init(mDatabase);
    mTicketCollectorFactory		= TicketCollectorFactory::Init(mDatabase);
    mItemFactory				= ItemFactory::Init(mDatabase);
    mResourceContainerFactory	= ResourceContainerFactory::Init(mDatabase);
}

//=============================================================================

TangibleFactory::~TangibleFactory()
{
    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void TangibleFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    switch(subGroup)
    {
    case TanGroup_Item:
        mItemFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;
    case TanGroup_Terminal:
        mTerminalFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;
    case TanGroup_Container:
        mContainerFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;
    case TanGroup_TicketCollector:
        mTicketCollectorFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;
    case TanGroup_ResourceContainer:
        mResourceContainerFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;
    case TanGroup_ManufacturingSchematic:
        gDatapadFactory->requestManufacturingSchematic(ofCallback,id);
    case TanGroup_Hopper:
        mItemFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;

    default:
        DLOG(INFO) << "TangibleFactory::requestObject Unknown Group";
        break;
    }
}

//=============================================================================

void TangibleFactory::releaseAllPoolsMemory()
{
    mItemFactory->releaseQueryContainerPoolMemory();
    mTerminalFactory->releaseQueryContainerPoolMemory();
    mContainerFactory->releaseQueryContainerPoolMemory();
    mTicketCollectorFactory->releaseQueryContainerPoolMemory();
    mResourceContainerFactory->releaseQueryContainerPoolMemory();
}

//=============================================================================


