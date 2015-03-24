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

#include "ZoneServer/Objects/Tangible Object/TangibleFactory.h"
#include <ZoneServer\Objects\ItemFactory.h>
#include "ZoneServer\Objects\DatapadFactory.h"

#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceContainerFactory.h"
#include "ZoneServer/Objects/Tangible Object/TangibleEnums.h"
#include "ZoneServer/Objects/TerminalFactory.h"
#include "ZoneServer/GameSystemManagers/Travel Manager/TicketCollectorFactory.h"
#include "Utils/utils.h"

#include <assert.h>

<<<<<<< HEAD:src/ZoneServer/Objects/Tangible Object/TangibleFactory.cpp
#include "anh/logger.h"
=======
// Fix for issues with glog redefining this constant
#ifdef ERROR
#undef ERROR
#endif

#include <glog/logging.h>
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/TangibleFactory.cpp
//=============================================================================

bool					TangibleFactory::mInsFlag    = false;
TangibleFactory*		TangibleFactory::mSingleton  = NULL;

//======================================================================================================================

TangibleFactory*	TangibleFactory::Init(swganh::app::SwganhKernel*	kernel)
{
    if(!mInsFlag)
    {
        mSingleton = new TangibleFactory(kernel);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

TangibleFactory::TangibleFactory(swganh::app::SwganhKernel*	kernel) : FactoryBase(kernel)
{
    mTerminalFactory			= TerminalFactory::Init(kernel);
    mTicketCollectorFactory		= TicketCollectorFactory::Init(kernel);
    mItemFactory				= ItemFactory::Init(kernel);
    mResourceContainerFactory	= ResourceContainerFactory::Init(kernel);
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
    mTicketCollectorFactory->releaseQueryContainerPoolMemory();
    mResourceContainerFactory->releaseQueryContainerPoolMemory();
}

//=============================================================================


