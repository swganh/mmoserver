/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2010 The SWG:ANH Team

 MMOServer is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 MMOServer is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with MMOServer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "anh/event_dispatcher/event_dispatcher.h"

#include <cassert>

using namespace anh::event_dispatcher;

EventDispatcher::EventDispatcher()
{}

EventDispatcher::~EventDispatcher()
{}

bool EventDispatcher::hasListeners(const EventType& event_type) const {
    return false;
}

bool EventDispatcher::hasRegisteredEventType(const EventType& event_type) const {
    auto it = registered_event_types_.find(event_type);
    if (it != registered_event_types_.end()) {
        return true;
    }

    return false;
}

bool EventDispatcher::registerEventType(EventType event_type) {
    if (hasRegisteredEventType(event_type)) {
        assert(false && "Attempt to register an existing event!");
        return false;
    }
    
    registered_event_types_.insert(std::move(event_type));
    return true;
}

bool EventDispatcher::subscribe(const EventType& event_type, EventListener listener) {
    return false;
}
