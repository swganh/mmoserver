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

#include <cstdint>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>

#include <boost/optional.hpp>
#include <tbb/concurrent_queue.h>

#include "anh/event_dispatcher/basic_event.h"

namespace anh {
namespace event_dispatcher {

typedef std::function<bool (std::shared_ptr<IEvent>)> EventListenerCallback;
typedef anh::HashString EventListenerType;
typedef std::pair<EventListenerType, EventListenerCallback> EventListener;


typedef std::list<EventListener> EventListenerList;
typedef std::map<EventType, EventListenerList> EventListenerMap;
typedef std::set<EventType> EventTypeSet;

typedef std::function<bool (uint64_t current_time_ms)> TriggerCondition;
typedef std::function<void (std::shared_ptr<IEvent>, bool)> PostTriggerCallback;

typedef std::tuple<std::shared_ptr<IEvent>, boost::optional<TriggerCondition>, boost::optional<PostTriggerCallback>> EventQueueItem;
typedef tbb::concurrent_queue<EventQueueItem> EventQueue;
typedef std::deque<EventQueue> EventQueueList;

class IEventDispatcher {
public:
    enum constants {
        INFINITE_TIMEOUT = 0xfffffff
    };

public:
    virtual ~IEventDispatcher();
    
    virtual bool subscribe(const EventType& event_type, EventListener listener) = 0;
    virtual void unsubscribe(const EventType& event_type, const EventListenerType& listener_type) = 0;
    virtual void unsubscribe(const EventListenerType& listener_type) = 0;

    virtual bool trigger(std::shared_ptr<IEvent> incoming_event) = 0;
    virtual bool trigger(std::shared_ptr<IEvent> incoming_event, PostTriggerCallback callback) = 0;
    
    virtual void triggerWhen(std::shared_ptr<IEvent> incoming_event, TriggerCondition condition) = 0;
    virtual void triggerWhen(std::shared_ptr<IEvent> incoming_event, TriggerCondition condition, PostTriggerCallback callback) = 0;

    virtual bool triggerAsync(std::shared_ptr<IEvent> incoming_event) = 0;
    virtual bool triggerAsync(std::shared_ptr<IEvent> incoming_event, PostTriggerCallback callback) = 0;

    virtual bool abort(const EventType& event_type, bool all_of_type = false) = 0;

    virtual bool tick(uint64_t timeout_ms = INFINITE_TIMEOUT) = 0;
};

class EventDispatcher : public IEventDispatcher {
public:
    enum constants {    
        NUM_QUEUES = 2
    };

public:
    EventDispatcher();
    ~EventDispatcher();

    bool hasListeners(const EventType& event_type) const;
    bool hasRegisteredEventType(const EventType& event_type) const;
    bool hasEvents() const;

    bool registerEventType(EventType event_type);
    EventTypeSet registered_event_types() const;

    bool subscribe(const EventType& event_type, EventListener listener);
    void unsubscribe(const EventType& event_type, const EventListenerType& listener_type);
    void unsubscribe(const EventListenerType& listener_type);

    bool trigger(std::shared_ptr<IEvent> incoming_event);
    bool trigger(std::shared_ptr<IEvent> incoming_event, PostTriggerCallback callback);
    
    void triggerWhen(std::shared_ptr<IEvent> incoming_event, TriggerCondition condition);
    void triggerWhen(std::shared_ptr<IEvent> incoming_event, TriggerCondition condition, PostTriggerCallback callback);

    bool triggerAsync(std::shared_ptr<IEvent> incoming_event);
    bool triggerAsync(std::shared_ptr<IEvent> incoming_event, PostTriggerCallback callback);

    bool abort(const EventType& event_type, bool all_of_type = false);

    bool tick(uint64_t timeout_ms = INFINITE_TIMEOUT);
    
private:
    bool validateEventType_(const EventType& event_type) const;
    uint32_t calculatePlacementQueue_(uint32_t priority = 0) const;

    EventTypeSet registered_event_types_;
    EventListenerMap event_listeners_;
    EventQueueList event_queues_;

    int active_queue_;
};

}  // namespace event_dispatcher
}  // namespace anh

#endif  // LIBANH _EVENT_DISPATCHER_EVENT_DISPATCHER_H_
