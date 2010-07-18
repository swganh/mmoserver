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

#include "Common/EventDispatcher.h"

namespace common {
    
EventDispatcher::EventDispatcher() {}

EventDispatcher::~EventDispatcher() {}

void EventDispatcher::Connect(const EventType& event_type, EventListener listener) {
    if (! ValidateEventType_(event_type)) {
        return;
    }
    
    if (! AddEventType_(event_type)) {
        return;
    }

    // Look for an entry and
    auto map_it = event_listener_map_.find(event_type);
    
    // Somehow the event type doesn't exist.
    if (map_it == event_listener_map_.end()) {
        return;
    }

    // Lookup the listener in the list to see if it already exists.
    EventListenerList& listener_list = (*map_it).second;

    for (auto list_it = listener_list.begin(), end = listener_list.end(); list_it != end; ++list_it) {
        if ((*list_it).first.ident() == listener.first.ident()) {
            return;
        }
    }

    // EventType has been validated, the listener validated and doesn't already exist, add it.
    listener_list.push_back(listener);
}


void EventDispatcher::Disconnect(const EventType& event_type, const EventListenerType& event_listener_type){
    // Make sure a valid event type was passed in.
    if (! ValidateEventType_(event_type)) {
        return;
    }
    // Make sure a valid event listener type was passed in.
    if (! ValidateEventListenerType_(event_type)) {
        return;
    }

    // Look for an entry and
    auto map_it = event_listener_map_.find(event_type);
    
    // Somehow the event type doesn't exist.
    if (map_it == event_listener_map_.end()) {
        return;
    }

    // Lookup the listener in the list to see if it already exists.
    EventListenerList& listener_list = (*map_it).second;
    
    // Loop through until we find it, no need to worry about invalidating the iterator
    // because after the erase it's never used again.
    for (auto list_it = listener_list.begin(), end = listener_list.end(); list_it != end; ++list_it) {
        if ((*list_it).first.ident() == event_listener_type.ident()) {
            listener_list.erase(list_it);
            break; // Item found and there is only one per list, break out.
        }
    }
}

EventListenerList EventDispatcher::GetListeners(const EventType& event_type) const {
    if (! ValidateEventType_(event_type)) {
        return EventListenerList();
    }

    EventListenerMap::const_iterator map_it = event_listener_map_.find(event_type);
    
    // no listerners currently for this event type, so sad
    if (map_it == event_listener_map_.end()) {
        return EventListenerList();
    }
    
    const EventListenerList& listener_list = map_it->second;
    
    // there was, but is not now, any listerners currently for
    // this event type, so sad
    if (listener_list.size() == 0) {
        return EventListenerList();
    }
    
    // Build up the result set to return.
    EventListenerList result;
        
    for (auto list_it = listener_list.begin(), end = listener_list.end(); list_it != end; ++list_it) {
    	result.push_back(*list_it);
    }
    
    return result;
}

bool EventDispatcher::ValidateEventType_(const EventType& event_type) const {
    // Make sure the string isn't empty.
    if (event_type.ident_string().length() == 0) {
        return false;
    }

    // If an event_type already exists verify that the text is the same so
    // that no naming clashes occur.
    EventTypeSet::const_iterator it = event_type_set_.find(event_type);
    
    if (it != event_type_set_.end()) {
        if ((*it).ident() != event_type.ident()) {
            return false;
        }
    }

    // If all the tests have passed then return true for validation.
    return true;
}

bool EventDispatcher::ValidateEventListenerType_(const EventListenerType& event_listener_type) const {
    // Make sure the string isn't empty.
    if (event_listener_type.ident_string().length() == 0) {
        return false;
    }

    // If all the tests have passed then return true for validation.
    return true;
}

bool EventDispatcher::AddEventType_(const EventType& event_type) {    
    // check / update type list
    EventTypeSet::iterator it = event_type_set_.find(event_type);
    
    // EventType already exists. Return true to indicate so.
    if (it != event_type_set_.end()) {
        return true;
    }

    // The EventType hasn't been registered before, attempt to insert it into the set.
    std::pair<EventTypeSet::iterator, bool> set_insert_result = event_type_set_.insert(event_type);
        
    // Insertion failed for some reason.
    if (set_insert_result.second == false) {
        return false;
    }
    
    // Somehow the insertion left the list empty, bail out.
    if (set_insert_result.first == event_type_set_.end()) {
        return false;
    }

    // Now ensure an empty list exists in the listener map for this type.
    std::pair<EventListenerMap::iterator, bool> map_insert_result = event_listener_map_.insert(
        std::pair<EventType, EventListenerList>(event_type, EventListenerList()));

    // Could not insert into map.
    if (map_insert_result.second == false) {
            return false;
    }
    
    // Somehow the insertion left the map empty, bail out.
    if (map_insert_result.first == event_listener_map_.end()) {
        return false;
    }

    // The event type already existed or it was successfully inserted, either
    // way it definitely exists in the set now.
    return true;
}

}  // namespace common
