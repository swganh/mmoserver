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

#include "ZoneServer\Objects\Object\Object_Enums.h"
#include "ZoneServer\Objects\Creature Object\creature_message_builder.h"
#include "ZoneServer\Objects\Creature Object\CreatureObject.h"
#include "ZoneServer\Objects\Player Object\PlayerObject.h"

#include "ZoneServer/Objects/Bank.h"
#include "ZoneServer/Objects/Inventory.h"

#include "anh\event_dispatcher\event_dispatcher.h"
#include "ZoneServer\Services\equipment\equipment_service.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include "ZoneServer/WorldManager.h"

#include "MessageLib/MessageLib.h"

using namespace swganh::event_dispatcher;
using namespace swganh::messages;

void TangibleMessageBuilder::RegisterEventHandlers()
{

	event_dispatcher_->Subscribe("CreatureObject::DefenderList", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
		
        auto value_event = std::static_pointer_cast<TangibleObjectEvent>(incoming_event);
        BuildStatDefenderDelta(value_event->Get());

    });
}

void TangibleMessageBuilder::BuildStatDefenderDelta(TangibleObject* const  tangible)
{
    //if (creature->haso)
	//Defenderlist is item #1
    DeltasMessage message = CreateDeltasMessage(tangible, VIEW_6, 1, SWG_CREATURE);
     
	//never ever send empty updates!!!!
	if(tangible->SerializeDefender(&message))	{
		gMessageLib->broadcastDelta(message,tangible);
	}    
}

// baselines
boost::optional<BaselinesMessage> TangibleMessageBuilder::BuildBaseline3(TangibleObject* const tangible, boost::unique_lock<boost::mutex>& lock)
{
    auto message = CreateBaselinesMessage(tangible, lock, VIEW_3, 11);
    message.data.append((*ObjectMessageBuilder::BuildBaseline3(tangible, lock)).data);
	message.data.write<std::string>(tangible->getCustomizationStr().getAnsi());
    //tangible->SerializeComponentCustomization(&message, lock);
	message.data.write<uint64_t>(0);//componentcustomization
    
	message.data.write<uint32_t>(tangible->getTypeOptions());


	uint32 uses = 0;
	if(tangible->hasAttribute("counter_uses_remaining"))
    {
        float fUses = tangible->getAttribute<float>(std::string("counter_uses_remaining"));
        uses = (int) fUses;
    }

    if(tangible->hasAttribute("stacksize"))
    {
        uses = tangible->getAttribute<int>(std::string("stacksize"));
    }
    if(tangible->getTimer() != 0)
        uses = tangible->getTimer();

    message.data.write<uint32_t>(uses);
    message.data.write<uint32_t>(tangible->getDamage());
    message.data.write<uint32_t>(tangible->getMaxCondition());
    message.data.write<uint8_t>(tangible->getStatic());
    return BaselinesMessage(std::move(message));
}

boost::optional<BaselinesMessage> TangibleMessageBuilder::BuildBaseline6(TangibleObject* const tangible, boost::unique_lock<boost::mutex>& lock)
{
    auto message = CreateBaselinesMessage(tangible, lock,  VIEW_6, 2);
    message.data.append((*ObjectMessageBuilder::BuildBaseline6(tangible, lock)).data);
    tangible->SerializeDefender(&message, lock);
    return BaselinesMessage(std::move(message));
}

boost::optional<BaselinesMessage> TangibleMessageBuilder::BuildBaseline7(TangibleObject* const tangible, boost::unique_lock<boost::mutex>& lock)
{
    auto message = CreateBaselinesMessage(tangible, lock, VIEW_7, 2);
    message.data.write<uint64_t>(0);
    message.data.write<uint64_t>(0);
    return BaselinesMessage(std::move(message));
}