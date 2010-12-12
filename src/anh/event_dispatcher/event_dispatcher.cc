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
#include <algorithm>

using namespace anh::event_dispatcher;
using namespace std;

EventDispatcher::EventDispatcher()
    : active_queue_(0)
{}

EventDispatcher::~EventDispatcher() {
    active_queue_ = 0;
}

bool EventDispatcher::hasListeners(const EventType& event_type) const {
    auto map_it = event_listeners_.find(event_type);
    if (map_it == event_listeners_.end()) {
        return false;
    }

    const EventListenerList& listener_list = (*map_it).second;

    return !listener_list.empty();
}

bool EventDispatcher::hasRegisteredEventType(const EventType& event_type) const {
    auto it = registered_event_types_.find(event_type);
    if (it != registered_event_types_.end()) {
        return true;
    }

    return false;
}

bool EventDispatcher::hasEvents() const {
    return !event_queues_[active_queue_].empty();
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
    if (!validateEventType_(event_type)) {
        return false;
    }

    auto map_it = event_listeners_.find(event_type);
    if (map_it == event_listeners_.end()) {
        auto insert_result = event_listeners_.insert(make_pair(event_type, EventListenerList()));

        // Check if there was an insertion failure
        if (insert_result.second == false) {
            return false;
        }

        // Cache the iterator and verify we didn't somehow create an empty map.
        if ((map_it = insert_result.first) == event_listeners_.end()) {
            return false;
        }
    }

    EventListenerList& listener_list = (*map_it).second;    

    // Look for the listener in the list before adding.
    auto find_it = find_if(listener_list.begin(), listener_list.end(), [&listener] (const EventListener& list_listener) {
        return list_listener.first == listener.first;
    });

    if (find_it != listener_list.end()) {
        return false;
    }

    listener_list.push_back(listener);

    return true;
}

void EventDispatcher::unsubscribe(const EventType& event_type, const EventListenerType& listener_type) {    
    auto map_it = event_listeners_.find(event_type);
    if (map_it == event_listeners_.end()) {
        return;
    }

    EventListenerList& listener_list = (*map_it).second;

    auto remove_it = remove_if(listener_list.begin(), listener_list.end(), [&listener_type] (const EventListener& list_listener) {
        return list_listener.first == listener_type;
    });

    listener_list.erase(remove_it, listener_list.end());
}

void EventDispatcher::unsubscribe(const EventListenerType& listener_type) {
    std::for_each(registered_event_types_.begin(), registered_event_types_.end(), [this, &listener_type] (const EventType& event_type) {        
        unsubscribe(event_type, listener_type);
    });
}

bool EventDispatcher::trigger(std::shared_ptr<BaseEvent> incoming_event) {
    const EventType& event_type = incoming_event->type();
    if (!validateEventType_(event_type)) {
        assert(false && "Event was triggered before its type was registered");
        return false;
    }
    
    auto map_it = event_listeners_.find(event_type);
    if (map_it == event_listeners_.end()) {        
        assert(false && "Inconsistency between listener map and event type set found");
        return false;
    }
    
    EventListenerList& listener_list = (*map_it).second;

    bool processed = false;

    for_each(listener_list.begin(), listener_list.end(), [incoming_event, &processed] (EventListener& list_listener) {
        if (list_listener.second(incoming_event)) {
            processed = true;
        }
    });

    return processed;
}

bool EventDispatcher::triggerAsync(std::shared_ptr<BaseEvent> incoming_event) {
    // Do a few quick sanity checks in debug mode to ensure our queue cycling is always on track.
    assert(active_queue_ >= 0);
    assert(active_queue_ < NUM_QUEUES);
    
    const EventType& event_type = incoming_event->type();
    if (!validateEventType_(event_type)) {
        assert(false && "Event was triggered before its type was registered");
        return false;
    }

    auto map_it = event_listeners_.find(event_type);
    if (map_it == event_listeners_.end()) {
        return false;
    }

    event_queues_[active_queue_].push(incoming_event);

    return true;
}

bool EventDispatcher::validateEventType_(const EventType& event_type) const {
    if (! event_type.ident_string().length()) {
        return false;
    }

    auto type_it = registered_event_types_.find(event_type); 
    if (type_it == registered_event_types_.end()) {
        return false;
    }

    return true;
}
