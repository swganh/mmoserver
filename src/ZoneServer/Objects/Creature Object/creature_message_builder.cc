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

#include <memory>

#include "ZoneServer\Objects\Object_Enums.h"
#include "ZoneServer\Objects\Creature Object\creature_message_builder.h"
#include "ZoneServer\Objects\Creature Object\CreatureObject.h"

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

}

void CreatureMessageBuilder::BuildStatDefenderDelta(const std::shared_ptr<CreatureObject>& creature)
{
    //if (creature->haso)
	//Defenderlist is item #1
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_6, 1, SWG_CREATURE);
     
	//never ever send empty updates!!!!
	if(creature->SerializeDefender(&message))	{
		gMessageLib->broadcastDelta(message,creature.get());
	}

		
    
}