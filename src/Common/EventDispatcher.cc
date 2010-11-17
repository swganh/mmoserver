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

EventDispatcher::EventDispatcher()
    : current_timestep_(0)
    , active_queue_(0) {}

EventDispatcher::EventDispatcher(uint64_t current_time)
    : current_timestep_(current_time)
    , active_queue_(0) {}

EventDispatcher::~EventDispatcher() {}

void EventDispatcher::Connect(const EventType& event_type, EventListener listener) {
    active_.Send([=] {
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

    } );
}


void EventDispatcher::Disconnect(const EventType& event_type, const EventListenerType& event_listener_type) {
    active_.Send(std::bind(&EventDispatcher::Disconnect_, this, event_type, event_listener_type));
}

void EventDispatcher::DisconnectFromAll(const EventListenerType& event_listener_type) {
    active_.Send([=] {
        // Make sure a valid event listener type was passed in.
        if (! ValidateEventListenerType_(event_listener_type)) {
            return;
        }

        // Use the known type lists to loop and call Disconnect for each.
        for (auto type_it = event_type_set_.begin(), end = event_type_set_.end(); type_it != end; ++type_it) {
            // Call the internal disconnect method so that each disconnect doesn't get queued.
            Disconnect_(*type_it, event_listener_type);
        }
    } );
}

boost::unique_future<std::vector<EventListener>> EventDispatcher::GetListeners(const EventType& event_type) {
    // Create a packaged task for retrieving the value.
    auto task = std::make_shared<boost::packaged_task<std::vector<EventListener>>>([=]()->std::vector<EventListener> {

        if (! ValidateEventType_(event_type)) {
            return std::vector<EventListener>();
        }

        EventListenerMap::const_iterator map_it = event_listener_map_.find(event_type);

        // no listeners currently for this event type, so sad
        if (map_it == event_listener_map_.end()) {
            return std::vector<EventListener>();
        }

        const EventListenerList& listener_list = map_it->second;

        // there was, but is not now, any listeners currently for
        // this event type, so sad
        if (listener_list.size() == 0) {
            return std::vector<EventListener>();
        }

        // Build up the result set to return.
        std::vector<EventListener> result;

        for (auto list_it = listener_list.begin(), end = listener_list.end(); list_it != end; ++list_it) {
            result.push_back(*list_it);
        }

        return result;
    } );

    // Add the message to the active object's queue that runs the task which in turn
    // updates the future.
    active_.Send([task] {
        (*task)();
    });

    // Return the future to the caller.
    return task->get_future();
}

boost::unique_future<std::vector<EventType>> EventDispatcher::GetRegisteredEvents() {
    // Create a packaged task for retrieving the value.
    auto task = std::make_shared<boost::packaged_task<std::vector<EventType>>>([=]()->std::vector<EventType> {

        std::vector<EventType> event_types;
        event_types.reserve(event_type_set_.size());

        // Use the known type lists to loop and call Disconnect for each.
        for (auto type_it = event_type_set_.begin(), end = event_type_set_.end(); type_it != end; ++type_it) {
            event_types.push_back(*type_it);
        }

        return event_types;
    } );

    // Add the message to the active object's queue that runs the task which in turn
    // updates the future.
    active_.Send([task] {
        (*task)();
    });

    // Return the future to the caller.
    return task->get_future();
}

void EventDispatcher::Notify(IEventPtr triggered_event) {
    // Sanity check on the event itself.
    if (!triggered_event) return;

    active_.Send([=] {
        // If the timestamp for the event has not yet been set then set it.
        if (!triggered_event->timestamp()) {
            triggered_event->timestamp(current_timestep_);
        }

        event_queue_[active_queue_].push(triggered_event);
    });
}

boost::unique_future<bool> EventDispatcher::Deliver(IEventPtr triggered_event) {
    // Create a packaged task for retrieving the value.
    auto task = std::make_shared<boost::packaged_task<bool>>(std::bind(&EventDispatcher::Deliver_, this, triggered_event));

    // Add the message to the active object's queue that runs the task which in turn
    // updates the future.
    active_.Send([task] {
        (*task)();
    });

    // Return the future to the caller.
    return task->get_future();
}

boost::unique_future<bool> EventDispatcher::HasEvents() {
    // Create a packaged task for retrieving the value.
    auto task = std::make_shared<boost::packaged_task<bool>>([=] {
        return (event_queue_[active_queue_].size() != 0);
    } );

    // Add the message to the active object's queue that runs the task which in turn
    // updates the future.
    active_.Send([task] {
        (*task)();
    });

    // Return the future to the caller.
    return task->get_future();
}

boost::unique_future<bool> EventDispatcher::Tick(uint64_t new_timestep) {
    // Create a packaged task for retrieving the value.
    auto task = std::make_shared<boost::packaged_task<bool>>([=]()->bool {
        // If we were passed the same time or a time in the past return false.
        if (current_timestep_ >= new_timestep) return false;

        current_timestep_ = new_timestep;

        int queue_to_process = active_queue_;
        active_queue_ = (active_queue_ + 1) % kNumQueues;

        while(event_queue_[queue_to_process].size() > 0) {
            IEventPtr event_to_process = event_queue_[queue_to_process].top();
            event_queue_[queue_to_process].pop();

            // Check to to see if the event is ready for processing yet. If so deliver it, if not put it on the new queue.
            if ((event_to_process->timestamp() + event_to_process->delay_ms()) <= current_timestep_) {
                Deliver_(event_to_process);
            } else {
                // Else push it back onto the next queue for processing.
                event_queue_[active_queue_].push(event_to_process);
            }
        }

        return true;
    } );

    // Add the message to the active object's queue that runs the task which in turn
    // updates the future.
    active_.Send([task] {
        (*task)();
    });

    // Return the future to the caller.
    return task->get_future();
}

boost::unique_future<uint64_t> EventDispatcher::current_timestep() {
    // Create a packaged task for retrieving the value.
    auto task = std::make_shared<boost::packaged_task<uint64_t>>([=] {
        return current_timestep_;
    } );

    // Add the message to the active object's queue that runs the task which in turn
    // updates the future.
    active_.Send([task] {
        (*task)();
    });

    // Return the future to the caller.
    return task->get_future();
}

bool EventDispatcher::ValidateEventType_(const EventType& event_type) const {
    // Make sure the string isn't empty.
    if (event_type.ident_string().length() == 0) {
        return false;
    }

    // If an event_type already exists verify that the text is the same so
    // that no naming clashes occur.
    EventTypeSet::const_iterator it = event_type_set_.find(event_type);

    // Check whether or not the event is known.
    if (it != event_type_set_.end()) {
        // If the ident's don't match for whatever reason we failed.
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

void EventDispatcher::Disconnect_(const EventType& event_type, const EventListenerType& event_listener_type) {
    // Make sure a valid event type was passed in.
    if (! ValidateEventType_(event_type)) {
        return;
    }
    // Make sure a valid event listener type was passed in.
    if (! ValidateEventListenerType_(event_listener_type)) {
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

bool EventDispatcher::Deliver_(IEventPtr triggered_event) {
    // Sanity check to make sure the event is valid.
    if (!triggered_event) {
        return false;
    }

    // Ensure the triggered event is a known valid type.
    if (!ValidateEventType_(triggered_event->event_type())) {
        return false;
    }

    // If the event's timestamp has not been set then set it.
    if (!triggered_event->timestamp()) {
        triggered_event->timestamp(current_timestep_);
    }

    // By default if an event isn't handled this method returns true.
    bool delivered = true;

    // Find the list of global listeners.
    auto listener_it = event_listener_map_.find(EventType(kWildCardHashString));

    if (listener_it != event_listener_map_.end()) {
        // Get the list of global listeners to iterate over.
        auto listeners = listener_it->second;

        // Allow each global listener the opportunity to process the event.
        for (auto it = listeners.begin(), end = listeners.end(); it != end; ++it) {
            (*it).second(triggered_event);
        }
    }

    // Find the list of listeners for this event type.
    listener_it = event_listener_map_.find(triggered_event->event_type());

    // If no listeners were found return false.
    if (listener_it == event_listener_map_.end()) {
        // Callbacks should be called here if no specific listeners were found.
        triggered_event->consume(delivered);

        IEventPtr next_event = triggered_event->next();

        if (next_event) {
            Notify(next_event);
        }

        return delivered;
    }

    // Get the list of listeners to iterate over.
    auto listeners = listener_it->second;

    // Allow each listener the opportunity to process the event, if it does set processed to true.
    for (auto it = listeners.begin(), end = listeners.end(); it != end; ++it) {
        if (!(*it).second(triggered_event)) {
            delivered = false;
        }
    }

    // If processing got this far then nothing failed so invoke the callback on the event.
    triggered_event->consume(delivered);

    IEventPtr next_event = triggered_event->next();

    if (next_event) {
        Notify(next_event);
    }

    return delivered;
}

}  // namespace common
