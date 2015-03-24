/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2014 The SWG:ANH Team

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

#include <cstdint>
#include <functional>
//#include <future>
#include <memory>
#include <unordered_map>
#include <vector>

#include "anh/event_dispatcher/basic_event.h"

#include <boost/asio/strand.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "anh/hash_string.h"


namespace boost {
namespace asio {
    class io_service;
}}  // namespace boost::asio

class Object;
class PlayerObject;

namespace swganh {
namespace event_dispatcher {
<<<<<<< HEAD
	
	class EventInterface;

    typedef uint32_t CallbackId;
    typedef HashString EventType;
    typedef std::function<void (const std::shared_ptr<EventInterface>&)> EventHandlerCallback;
=======

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
>>>>>>> parent of 5bd772a... got rid of google log

    class EventInterface
    {
    public:
        virtual ~EventInterface() {}

        virtual EventType Type() const = 0;
    };

<<<<<<< HEAD
    class BaseEvent : public EventInterface
    {
    public:
        explicit BaseEvent(EventType type);

        EventType Type() const;
=======
    bool hasListeners(const EventType& event_type) const;
    bool hasRegisteredEventType(const EventType& event_type) const;
    bool hasEvents() const;
>>>>>>> parent of 5bd772a... got rid of google log

    private:
        EventType type_;
    };

<<<<<<< HEAD
    template<typename T>
    class ValueEvent : public BaseEvent
    {
    public:
        ValueEvent(EventType type)
            : BaseEvent(type)
        {}

        ValueEvent(EventType type, T data)
            : BaseEvent(type)
            , data_(std::move(data))
        {}

        const T& Get() const
        {
            return data_;
        }

        T& Get()
        {
            return data_;
        }

    private:
        T data_;
    };

	struct ObserverEvent : BaseEvent
	{
		ObserverEvent(EventType type, Object* object_, PlayerObject* observer_)
			: BaseEvent(type)
			, object(object_)
			, observer(observer_)
		{}
		Object* object;
		PlayerObject* observer;
	};


	template<typename T>
    class ChainEvent : public BaseEvent
    {
    public:
        ChainEvent(EventType type)
            : BaseEvent(type)
        {chained_ = ""}

        ChainEvent(EventType type, EventType chained_event, T data)
            : BaseEvent(type)
			, chained_(chained_event)
            , data_(std::move(data))
        {}

        const T& Get() const
        {
            return data_;
        }

        T& Get()
        {
            return data_;
        }

    private:
        T data_;
		EventType chained_;
    };
    
    class EventDispatcherInterface
    {
    public:
        virtual ~EventDispatcherInterface() {}
        
        virtual CallbackId Subscribe(EventType type, EventHandlerCallback callback) = 0;
        virtual void Unsubscribe(EventType type, CallbackId identifier) = 0;

        virtual boost::unique_future<std::shared_ptr<EventInterface>> Dispatch(const std::shared_ptr<EventInterface>& dispatch_event) = 0;
		// Shutdown the event dispatcher and stops dispatching events immediately
		virtual void Shutdown() = 0;
    };

    class EventDispatcher : public EventDispatcherInterface
    {
    public:
        explicit EventDispatcher(boost::asio::io_service& io_service);
        ~EventDispatcher();
=======
    bool subscribe(const EventType& event_type, EventListener listener);
    void unsubscribe(const EventType& event_type, const EventListenerType& listener_type);
    void unsubscribe(const EventListenerType& listener_type);

    bool trigger(std::shared_ptr<IEvent> incoming_event);
    bool trigger(std::shared_ptr<IEvent> incoming_event, PostTriggerCallback callback);
    
    void triggerWhen(std::shared_ptr<IEvent> incoming_event, TriggerCondition condition);
    void triggerWhen(std::shared_ptr<IEvent> incoming_event, TriggerCondition condition, PostTriggerCallback callback);

    bool triggerAsync(std::shared_ptr<IEvent> incoming_event);
    bool triggerAsync(std::shared_ptr<IEvent> incoming_event, PostTriggerCallback callback);
>>>>>>> parent of 5bd772a... got rid of google log

        CallbackId Subscribe(EventType type, EventHandlerCallback callback);
        void Unsubscribe(EventType type, CallbackId identifier);

		std::shared_ptr<EventInterface>  DispatchMainThread(const std::shared_ptr<EventInterface>& dispatch_event);
        boost::unique_future<std::shared_ptr<EventInterface>> Dispatch(const std::shared_ptr<EventInterface>& dispatch_event);

		void Shutdown();

<<<<<<< HEAD
    private:
        typedef std::unordered_map<CallbackId, EventHandlerCallback> EventHandlerList;
        
        typedef std::unordered_map<
            EventType, 
            EventHandlerList
        > EventHandlerMap;

        CallbackId GenerateCallbackId();
        void InvokeCallbacks(const std::shared_ptr<EventInterface>& dispatch_event);

        boost::shared_mutex event_handlers_mutex_;
        EventHandlerMap event_handlers_;
        boost::asio::io_service& io_service_;
    };

=======
    int active_queue_;
};
>>>>>>> parent of 5bd772a... got rid of google log

}  // namespace anh
 
}  // namespace event_dispatcher

#endif  // LIBANH _EVENT_DISPATCHER_EVENT_DISPATCHER_H_
