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

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace anh::event_dispatcher;
using namespace boost::posix_time;
using namespace std;

IEventDispatcher::~IEventDispatcher() {}

EventDispatcher::EventDispatcher()
    : event_queues_(NUM_QUEUES)
    , active_queue_(0)
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
    return !event_queues_.front().empty();
}

bool EventDispatcher::registerEventType(EventType event_type) {
    if (hasRegisteredEventType(event_type)) {
        assert(false && "Attempt to register an existing event!");
        return false;
    }
    
    registered_event_types_.insert(std::move(event_type));
    return true;
}

EventTypeSet EventDispatcher::registered_event_types() const {
    return registered_event_types_;
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

bool EventDispatcher::trigger(std::shared_ptr<IEvent> incoming_event) {
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

    if (!incoming_event->timestamp()) {
        time_duration duration = microsec_clock::local_time().time_of_day();
        incoming_event->timestamp(duration.total_milliseconds());
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

bool EventDispatcher::trigger(std::shared_ptr<IEvent> incoming_event, PostTriggerCallback callback) {
    bool processed = trigger(incoming_event);

    callback(incoming_event, processed);

    return processed;
}


void EventDispatcher::triggerWhen(std::shared_ptr<IEvent> incoming_event, TriggerCondition condition) {
    // Do a few quick sanity checks in debug mode to ensure our queue cycling is always on track.
    assert(active_queue_ >= 0);
    assert(active_queue_ < NUM_QUEUES);
    
    const EventType& event_type = incoming_event->type();
    if (!validateEventType_(event_type)) {
        assert(false && "Event was triggered before its type was registered");
        return;
    }

    auto map_it = event_listeners_.find(event_type);
    if (map_it == event_listeners_.end()) {
        return;
    }

    if (!incoming_event->timestamp()) {
        time_duration duration = microsec_clock::local_time().time_of_day();
        incoming_event->timestamp(duration.total_milliseconds());
    }
    
    uint32_t placement_queue = calculatePlacementQueue_(incoming_event->priority());
    event_queues_[placement_queue].push(make_tuple(incoming_event, condition, boost::optional<PostTriggerCallback>()));
}


void EventDispatcher::triggerWhen(std::shared_ptr<IEvent> incoming_event, TriggerCondition condition, PostTriggerCallback callback) {
    // Do a few quick sanity checks in debug mode to ensure our queue cycling is always on track.
    assert(active_queue_ >= 0);
    assert(active_queue_ < NUM_QUEUES);
    
    const EventType& event_type = incoming_event->type();
    if (!validateEventType_(event_type)) {
        assert(false && "Event was triggered before its type was registered");
        return;
    }

    auto map_it = event_listeners_.find(event_type);
    if (map_it == event_listeners_.end()) {
        return;
    }

    if (!incoming_event->timestamp()) {
        time_duration duration = microsec_clock::local_time().time_of_day();
        incoming_event->timestamp(duration.total_milliseconds());
    }
    
    uint32_t placement_queue = calculatePlacementQueue_(incoming_event->priority());
    event_queues_[placement_queue].push(make_tuple(incoming_event, condition, callback));
}

bool EventDispatcher::triggerAsync(std::shared_ptr<IEvent> incoming_event) {
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

    if (!incoming_event->timestamp()) {
        time_duration duration = microsec_clock::local_time().time_of_day();
        incoming_event->timestamp(duration.total_milliseconds());
    }
    
    uint32_t placement_queue = calculatePlacementQueue_(incoming_event->priority());
    event_queues_[placement_queue].push(make_tuple(incoming_event, boost::optional<TriggerCondition>(), boost::optional<PostTriggerCallback>()));

    return true;
}


bool EventDispatcher::triggerAsync(std::shared_ptr<IEvent> incoming_event, PostTriggerCallback callback) {
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

    if (!incoming_event->timestamp()) {
        time_duration duration = microsec_clock::local_time().time_of_day();
        incoming_event->timestamp(duration.total_milliseconds());
    }

    uint32_t placement_queue = calculatePlacementQueue_(incoming_event->priority());
    event_queues_[placement_queue].push(make_tuple(incoming_event, boost::optional<TriggerCondition>(), callback));

    return true;
}


bool EventDispatcher::abort(const EventType& event_type, bool all_of_type) {
    // Do a few quick sanity checks in debug mode to ensure our queue cycling is always on track.
    assert(active_queue_ >= 0);
    assert(active_queue_ < NUM_QUEUES);
    
    if (!validateEventType_(event_type)) {
        assert(false && "Event was triggered before its type was registered");
        return false;
    }

    // See if any events have registered for this type of event.
    auto find_it = event_listeners_.find(event_type);
    if (find_it == event_listeners_.end()) {
        return false;
    }    

    bool removed = false;
    
    // cycle through each of the queues starting with the current
    for (uint32_t i = 0, current = 0; i < NUM_QUEUES; ++i) {
        // If we've already found an item and we're not search for all events break now.
        if (removed && !all_of_type) {
            break;
        }

        current = calculatePlacementQueue_(i);       
        EventQueue tmp_queue = std::move(event_queues_[current]);
        event_queues_[current].clear();

        // Only place items back in the queue as needed.
        for_each(tmp_queue.unsafe_begin(), tmp_queue.unsafe_end(), [this, current, &event_type, all_of_type, &removed] (EventQueueItem& queued_event) {
            if (!removed || all_of_type) {
                if (get<0>(queued_event)->type() == event_type) {
                    removed = true;
                    return;
                }
            }
            
            event_queues_[current].push(queued_event);
        });
    }

    return removed;    
}

bool EventDispatcher::tick(uint64_t timeout_ms) {
    // Create a new empty queue and swap it with the current active queue.
    EventQueue process_queue = event_queues_[active_queue_];
    event_queues_[active_queue_].clear();
    
    active_queue_ = (active_queue_ + 1) % NUM_QUEUES;

    ptime current_time = microsec_clock::local_time();
    time_duration max_time = milliseconds(timeout_ms);
    time_period tick_period(current_time, current_time + max_time);
    
    EventQueueItem tick_event;
    uint32_t new_placement_queue = 0;

    while(!process_queue.empty()) {
        if (!process_queue.try_pop(tick_event)) {
            continue;
        }

        // Check to see if we have a conditional for our event and if so test it
        if (get<1>(tick_event).is_initialized()) {
            time_duration duration = microsec_clock::local_time().time_of_day();
            if (!get<1>(tick_event).get()(duration.total_milliseconds())) {
                new_placement_queue = calculatePlacementQueue_(get<0>(tick_event)->priority());

                // If the condition failed put the event back to wait.
                event_queues_[new_placement_queue].push(tick_event);
                continue;
            }
        }

        // If there was a callback given with the event call the appropriate trigger.
        if (get<2>(tick_event).is_initialized()) {
            trigger(get<0>(tick_event), get<2>(tick_event).get());
        } else {
            trigger(get<0>(tick_event));
        }

        if (timeout_ms == INFINITE_TIMEOUT) {
            continue;
        }
            
        current_time = microsec_clock::local_time();

        if (current_time >= tick_period.end()) {
            break;
        }
    }

    bool queue_flushed = process_queue.empty();
    if (!queue_flushed) {
        while (!process_queue.empty()) {
            if (process_queue.try_pop(tick_event)) {
                new_placement_queue = calculatePlacementQueue_(get<0>(tick_event)->priority());
                event_queues_[new_placement_queue].push(tick_event);
            }
        }
    }

    return queue_flushed;
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

uint32_t EventDispatcher::calculatePlacementQueue_(uint32_t priority) const {
    priority = min(priority, static_cast<uint32_t>(NUM_QUEUES));
    return (active_queue_ + priority) % NUM_QUEUES;
}

