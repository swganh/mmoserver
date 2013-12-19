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

#ifndef LIBANH_EVENT_DISPATCHER_EVENT_DISPATCHER_INTERFACE_H_
#define LIBANH_EVENT_DISPATCHER_EVENT_DISPATCHER_INTERFACE_H_

#include <cstdint>

#include <deque>
#include <functional>
#include <list>
#include <map>
#include <anh/memory.h>
#include <set>
#include <tuple>

#include <boost/optional.hpp>
#include <tbb/concurrent_queue.h>

#include "anh/hash_string.h"
#include "anh/event_dispatcher/event_interface.h"

namespace anh {
namespace event_dispatcher {
    
typedef std::function<bool (std::shared_ptr<EventInterface>)> EventListenerCallback;

typedef std::pair<uint64_t, EventListenerCallback> EventListener;
typedef std::list<EventListener> EventListenerList;
typedef std::map<EventType, EventListenerList> EventListenerMap;
typedef std::set<EventType> EventTypeSet;

typedef std::function<bool (uint64_t current_time_ms)> TriggerCondition;
typedef std::function<void (std::shared_ptr<EventInterface>, bool)> PostTriggerCallback;

typedef std::tuple<std::shared_ptr<EventInterface>, boost::optional<TriggerCondition>, boost::optional<PostTriggerCallback>> EventQueueItem;
typedef tbb::concurrent_queue<EventQueueItem> EventQueue;
typedef std::deque<EventQueue> EventQueueList;

class EventDispatcherInterface {
public:
    enum constants {
        INFINITE_TIMEOUT = 0xfffffff
    };

public:
    virtual ~EventDispatcherInterface() {}
    
    virtual uint64_t subscribe(const EventType& event_type, EventListenerCallback listener) = 0;
    virtual void unsubscribe(const EventType& event_type, uint64_t listener_id) = 0;
    virtual void unsubscribe(const EventType& event_type) = 0;

    virtual bool trigger(std::shared_ptr<EventInterface> incoming_event) = 0;
    virtual bool trigger(std::shared_ptr<EventInterface> incoming_event, PostTriggerCallback callback) = 0;
    
    virtual void triggerWhen(std::shared_ptr<EventInterface> incoming_event, TriggerCondition condition) = 0;
    virtual void triggerWhen(std::shared_ptr<EventInterface> incoming_event, TriggerCondition condition, PostTriggerCallback callback) = 0;

    virtual bool triggerAsync(std::shared_ptr<EventInterface> incoming_event) = 0;
    virtual bool triggerAsync(std::shared_ptr<EventInterface> incoming_event, PostTriggerCallback callback) = 0;

    virtual bool abort(const EventType& event_type, bool all_of_type = false) = 0;

    virtual bool tick(uint64_t timeout_ms = INFINITE_TIMEOUT) = 0;
};

class NullEventDispatcher : public EventDispatcherInterface {
public:
    ~NullEventDispatcher() {}
    
    uint64_t subscribe(
        const EventType& event_type, 
        EventListenerCallback listener) 
    {
        return 0;
    }

    void unsubscribe(const EventType& event_type, uint64_t listener_id) {}
    void unsubscribe(const EventType& event_type) {}

    bool trigger(std::shared_ptr<EventInterface> incoming_event) {
        return false;
    }

    bool trigger(
        std::shared_ptr<EventInterface> incoming_event, 
        PostTriggerCallback callback) 
    { 
        return false;
    }
    
    void triggerWhen(
        std::shared_ptr<EventInterface> incoming_event, 
        TriggerCondition condition) 
    {}

    void triggerWhen(
        std::shared_ptr<EventInterface> incoming_event, 
        TriggerCondition condition, 
        PostTriggerCallback callback)
    {}

    bool triggerAsync(std::shared_ptr<EventInterface> incoming_event) {
        return false;
    }

    bool triggerAsync(
        std::shared_ptr<EventInterface> incoming_event, 
        PostTriggerCallback callback) 
    {
        return false;
    }

    bool abort(const EventType& event_type, bool all_of_type = false) {
        return false;
    }

    bool tick(uint64_t timeout_ms = INFINITE_TIMEOUT) {
        return false;
    }
};

}  // namespace event_dispatcher
}  // namespace anh

#endif  // LIBANH_EVENT_DISPATCHER_EVENT_DISPATCHER_INTERFACE_H_
