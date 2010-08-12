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

#include "ZoneServer/HamService.h"

#include "Common/LogManager.h"

#include "SwgProtocol/ObjectControllerEvents.h"

using ::common::ApplicationService;
using ::common::IEventPtr;
using ::common::EventType;
using ::common::EventListener;
using ::common::EventListenerType;

namespace zone {

HamService::HamService(::common::EventDispatcher& event_dispatcher)
: ::common::ApplicationService(event_dispatcher)
, command_property_map_(gObjControllerCmdPropertyMap) {}

HamService::~HamService() {}

void HamService::onInitialize() {
    event_dispatcher_.Connect(::swg_protocol::object_controller::PreCommandEvent::type, EventListener(EventListenerType("HamService::handleSuccessfulObjectControllerCommand"), std::bind(&HamService::handlePreCommandEvent, this, std::placeholders::_1)));
}

bool HamService::handlePreCommandEvent(IEventPtr triggered_event) {
    // Cast the IEvent to the PreCommandEvent.
    std::shared_ptr<::swg_protocol::object_controller::PreCommandEvent> pre_event = std::dynamic_pointer_cast<::swg_protocol::object_controller::PreCommandEvent>(triggered_event);

    if (!pre_event) {
        assert(!"Received an invalid event!");
        return false;
    }
    
    // This command doesn't exist in the properties map and shouldn't be executed further.
    CmdPropertyMap::const_iterator it = command_property_map_.find(pre_event->command_crc());
    if (it == command_property_map_.end()) {
        return false;
    }

    gLogger->log(LogManager::CRITICAL, "HAM Service - PreCommandEvent successfully processed");

    return true;
}

}  // namespace zone
