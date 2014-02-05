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

#include <memory>

#include "ZoneServer\Objects\Object_Enums.h"
#include "ZoneServer\Objects\Creature Object\creature_message_builder.h"
#include "ZoneServer\Objects\Creature Object\CreatureObject.h"
#include "ZoneServer\Objects\Player Object\PlayerObject.h"

#include "ZoneServer/Objects/Bank.h"
#include "ZoneServer/Objects/Inventory.h"

#include "anh\event_dispatcher\event_dispatcher.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"
#include "ZoneServer\Objects\Object_Enums.h"

#include "MessageLib/MessageLib.h"

using namespace swganh::event_dispatcher;
using namespace swganh::messages;

void CreatureMessageBuilder::RegisterEventHandlers()
{

	event_dispatcher_->Subscribe("CreatureObject::DefenderList", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
		
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatDefenderDelta(value_event->Get());

    });

	event_dispatcher_->Subscribe("CreatureObject::InventoryCredits", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildInventoryCreditsDelta(value_event->Get());

    });

	event_dispatcher_->Subscribe("CreatureObject::BankCredits", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildInventoryCreditsDelta(value_event->Get());

    });

	event_dispatcher_->Subscribe("CreatureObject::BattleFatigue", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildBattleFatigueDelta(value_event->Get());

    });

	event_dispatcher_->Subscribe("CreatureObject::StatCurrent", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatCurrentDelta(value_event->Get());

    });
    event_dispatcher_->Subscribe("CreatureObject::StatMax", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatMaxDelta(value_event->Get());

    });
	event_dispatcher_->Subscribe("CreatureObject::StatEncumberance", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatEncumberanceDelta(value_event->Get());

    });
	event_dispatcher_->Subscribe("CreatureObject::StatWound", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatWoundDelta(value_event->Get());

    });
	event_dispatcher_->Subscribe("CreatureObject::StatBase", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatBaseDelta(value_event->Get());

    });


}

void CreatureMessageBuilder::BuildStatEncumberanceDelta(CreatureObject* const creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildStatEncumberanceDelta: " << creature->getId();
	DeltasMessage message = CreateDeltasMessage(creature, VIEW_4, 2, SWG_CREATURE);
	if(creature->SerializeStatEncumberances(&message))
		gMessageLib->broadcastDelta(message,creature);
}


void CreatureMessageBuilder::BuildStatCurrentDelta(CreatureObject* const  creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildStatCurrentDelta : " << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_6, 13, SWG_CREATURE);
    if(creature->SerializeCurrentStats(&message))
		gMessageLib->broadcastDelta(message,creature);
}

void CreatureMessageBuilder::BuildStatMaxDelta(CreatureObject* const  creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildStatMaxDelta : " << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_6, 14, SWG_CREATURE);
    if(creature->SerializeMaxStats(&message))
		gMessageLib->broadcastDelta(message,creature);
}

void CreatureMessageBuilder::BuildStatBaseDelta(CreatureObject* const  creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildStatBaseDelta : " << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_1, 2, SWG_CREATURE);
    if(creature->SerializeBaseStats(&message))
		gMessageLib->broadcastDelta(message,creature);
}



void CreatureMessageBuilder::BuildStatWoundDelta(CreatureObject* const  creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildStatWoundDelta : " << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_3, 17, SWG_CREATURE);
    if(creature->SerializeStatWounds(&message))
		gMessageLib->broadcastDelta(message,creature);
}


void CreatureMessageBuilder::BuildBattleFatigueDelta(CreatureObject* const  creature)
{
	PlayerObject* const  player = dynamic_cast<PlayerObject*>(creature);
	if(!player)	{
		return;
	}
	LOG(info) << "CreatureMessageBuilder::BuildBattleFatigueDelta" << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_3, 15, SWG_CREATURE);
    message.data.write<uint32_t>(creature->GetBattleFatigue());
    gMessageLib->sendDelta(message,player);
}


void CreatureMessageBuilder::BuildStatDefenderDelta(CreatureObject* const  creature)
{
    //if (creature->haso)
	//Defenderlist is item #1
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_6, 1, SWG_CREATURE);
     
	//never ever send empty updates!!!!
	if(creature->SerializeDefender(&message))	{
		gMessageLib->broadcastDelta(message,creature);
	}    
}

void CreatureMessageBuilder::BuildInventoryCreditsDelta(CreatureObject* const  creature)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(creature);
	if(!player)	{
		return;
	}

	Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	if(!inventory)	{
		return;
	}

    DeltasMessage message = CreateDeltasMessage(creature, VIEW_1, 1, SWG_CREATURE);
    message.data.write(inventory->getCredits());
	gMessageLib->sendDelta(message,player);
	 
}

void CreatureMessageBuilder::BuildBankCreditsDelta(CreatureObject* const  creature)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(creature);
	if(!player)	{
		return;
	}

	Bank* bank = dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
	if(!bank)	{
		return;
	}

    DeltasMessage message = CreateDeltasMessage(creature, VIEW_1, 0, SWG_CREATURE);
    message.data.write(bank->getCredits());
	gMessageLib->sendDelta(message,player);
	 
}