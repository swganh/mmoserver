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

#include "CreatureFactory.h"

#include "anh/logger.h"

#include "CreatureEnums.h"
#include "ZoneServer/Objects/ObjectFactoryCallback.h"
#include "ZoneServer/GameSystemManagers/NPC Manager/PersistentNpcFactory.h"
#include "ZoneServer/Objects/ShuttleFactory.h"
#include "ZoneServer/Objects/Creature Object/CreatureObject.h"
#include "ZoneServer/Objects/Inventory.h"

#include "ZoneServer/WorldManager.h"

#include "anh\event_dispatcher\event_dispatcher.h"
#include "anh/app/swganh_kernel.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include <cppconn/resultset.h>

#include "Utils/utils.h"

#include <assert.h>

using namespace swganh::event_dispatcher;

//=============================================================================

bool					CreatureFactory::mInsFlag    = false;
CreatureFactory*		CreatureFactory::mSingleton  = NULL;

//======================================================================================================================


CreatureFactory*	CreatureFactory::Init(swganh::database::Database* database)
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

CreatureFactory::CreatureFactory(swganh::database::Database* database) : FactoryBase(database)
{
    mPersistentNpcFactory	= PersistentNpcFactory::Init(mDatabase);
    mShuttleFactory			= ShuttleFactory::Init(mDatabase);
	RegisterEventHandlers();
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
        LOG(error) << "Unknown Group";
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

void CreatureFactory::RegisterEventHandlers()
{
	auto dispatcher = gWorldManager->getKernel()->GetEventDispatcher();

	dispatcher->Subscribe("CreatureObjectFactory::PersistInventoryCredits", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
		auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
		PersistInventoryCredits(value_event->Get());
        
		
    });
}

void CreatureFactory::PersistInventoryCredits(CreatureObject* creature)
{
	std::stringstream sql;
	Inventory* inventory = dynamic_cast<Inventory*>(creature->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	sql << "UPDATE " << mDatabase->galaxy() << ".inventories set credits=" << inventory->getCredits() << " WHERE id=" << inventory->getId() <<";";
	
	gWorldManager->getKernel()->GetDatabase()->executeSqlAsync(NULL, NULL, sql.str());
}