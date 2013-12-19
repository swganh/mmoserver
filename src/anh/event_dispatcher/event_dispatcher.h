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

#ifndef LIBANH_EVENT_DISPATCHER_EVENT_DISPATCHER_H_
#define LIBANH_EVENT_DISPATCHER_EVENT_DISPATCHER_H_

#include <tbb/atomic.h>

#include "anh/event_dispatcher/basic_event.h"
#include "anh/event_dispatcher/event_dispatcher_interface.h"
#include "anh/event_dispatcher/exceptions.h"

namespace anh {
namespace event_dispatcher {
    
class EventDispatcher : public EventDispatcherInterface {
public:
    enum constants {    
        NUM_QUEUES = 2
    };

public:
    EventDispatcher();
    ~EventDispatcher();

    uint64_t subscribe(const EventType& event_type, EventListenerCallback listener);

    bool hasListeners(const EventType& event_type) const;
    bool hasRegisteredEventType(const EventType& event_type) const;
    bool hasEvents() const;

    bool registerEventType(EventType event_type);
    EventTypeSet registered_event_types() const;

    void unsubscribe(const EventType& event_type, uint64_t listener_id);
    void unsubscribe(const EventType& event_type);

    bool trigger(std::shared_ptr<EventInterface> incoming_event);
    bool trigger(std::shared_ptr<EventInterface> incoming_event, PostTriggerCallback callback);
    
    void triggerWhen(std::shared_ptr<EventInterface> incoming_event, TriggerCondition condition);
    void triggerWhen(std::shared_ptr<EventInterface> incoming_event, TriggerCondition condition, PostTriggerCallback callback);

    bool triggerAsync(std::shared_ptr<EventInterface> incoming_event);
    bool triggerAsync(std::shared_ptr<EventInterface> incoming_event, PostTriggerCallback callback);

    bool abort(const EventType& event_type, bool all_of_type = false);

    bool tick(uint64_t timeout_ms = INFINITE_TIMEOUT);
    
private:
    bool validateEventType_(const EventType& event_type) const;
    uint32_t calculatePlacementQueue_(uint32_t priority = 0) const;

    EventTypeSet registered_event_types_;
    EventListenerMap event_listeners_;
    EventQueueList event_queues_;

    tbb::atomic<uint64_t> next_event_listener_id_;

    int active_queue_;
};

}  // namespace event_dispatcher
}  // namespace anh

#endif  // LIBANH _EVENT_DISPATCHER_EVENT_DISPATCHER_H_
