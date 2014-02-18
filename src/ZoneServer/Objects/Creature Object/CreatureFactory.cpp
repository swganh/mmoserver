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

#include "CreatureFactory.h"

#include "anh/logger.h"

#include "CreatureEnums.h"
#include "ZoneServer/Objects/Object/ObjectFactoryCallback.h"
#include "ZoneServer/GameSystemManagers/NPC Manager/PersistentNpcFactory.h"
#include "ZoneServer/Objects/ShuttleFactory.h"
#include "ZoneServer/Objects/Creature Object/CreatureObject.h"
#include "ZoneServer/Objects/Inventory.h"
#include "ZoneServer/Objects/Bank.h"

#include "ZoneServer/WorldManager.h"
#include "ZoneServer\Services\equipment\equipment_service.h"
#include "anh\service\service_manager.h"

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


CreatureFactory*	CreatureFactory::Init(swganh::app::SwganhKernel*	kernel)
{
    if(!mInsFlag)
    {
        mSingleton = new CreatureFactory(kernel);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

CreatureFactory::CreatureFactory(swganh::app::SwganhKernel*	kernel) : FactoryBase(kernel)
{
	mDatabase = kernel->GetDatabase();
    mPersistentNpcFactory	= PersistentNpcFactory::Init(kernel);
    mShuttleFactory			= ShuttleFactory::Init(kernel);
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
	case CreoGroup_AttackableObject:
		break;

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
	auto dispatcher = kernel_->GetEventDispatcher();

	dispatcher->Subscribe("CreatureObjectFactory::PersistInventoryCredits", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
		auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
		PersistInventoryCredits(value_event->Get());
        
		
    });

	dispatcher->Subscribe("CreatureObjectFactory::PersistBankCredits", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
		auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
		PersistBankCredits(value_event->Get());
    });

	dispatcher->Subscribe("CreatureObjectFactory::PersistHomeBank", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
		auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
		PersistHomeBank(value_event->Get());

		//Bank* bank = dynamic_cast<Bank*>(value_event->Get()->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
		//std::stringstream sql;
		//sql << "UPDATE " << mDatabase->galaxy() << ".banks SET planet_id=" << bank->planet() << " WHERE id=" << bank->getId() << ";";
		//mDatabase->executeSqlAsync(NULL, NULL, sql.str());
    });

}

void CreatureFactory::PersistInventoryCredits(CreatureObject* creature)
{
	std::stringstream sql;
	auto equip_service = kernel_->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(creature, "inventory"));

	sql << "UPDATE " << mDatabase->galaxy() << ".inventories set credits=" << inventory->getCredits() << " WHERE id=" << inventory->getId() <<";";
	
	gWorldManager->getKernel()->GetDatabase()->executeSqlAsync(NULL, NULL, sql.str());
}

void CreatureFactory::PersistBankCredits(CreatureObject* creature)
{
	std::stringstream sql;
	auto equip_service = kernel_->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto bank = dynamic_cast<Bank*>(equip_service->GetEquippedObject(creature, "bank"));

	sql << "UPDATE " << mDatabase->galaxy() << ".banks set credits=" << bank->getCredits() << " WHERE id=" << bank->getId() <<";";
	
	gWorldManager->getKernel()->GetDatabase()->executeSqlAsync(NULL, NULL, sql.str());
}

void CreatureFactory::PersistHomeBank(CreatureObject* creature)
{
	std::stringstream sql;
	auto equip_service = kernel_->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto bank = dynamic_cast<Bank*>(equip_service->GetEquippedObject(creature, "bank"));

	sql << "UPDATE " << mDatabase->galaxy() << ".banks SET planet_id=" << bank->getPlanet() << " WHERE id=" << bank->getId() << ";";
	
	gWorldManager->getKernel()->GetDatabase()->executeSqlAsync(NULL, NULL, sql.str());
}