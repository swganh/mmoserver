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

#include <cstdint>
#include <functional>
//#include <future>
#include <memory>
#include <unordered_map>
#include <vector>

#include "anh/event_dispatcher/basic_event.h"
#include "anh/event_dispatcher/event_dispatcher_interface.h"
#include "anh/event_dispatcher/exceptions.h"

#include <boost/asio/strand.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "anh/hash_string.h"


namespace boost {
namespace asio {
    class io_service;
}}  // namespace boost::asio


namespace swganh {
namespace event_dispatcher {
	
	class EventInterface;

    typedef uint32_t CallbackId;
    typedef HashString EventType;
    typedef std::function<void (const std::shared_ptr<EventInterface>&)> EventHandlerCallback;

    class EventInterface
    {
    public:
        virtual ~EventInterface() {}

        virtual EventType Type() const = 0;
    };

    class BaseEvent : public EventInterface
    {
    public:
        explicit BaseEvent(EventType type);

        EventType Type() const;

    private:
        EventType type_;
    };

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

        CallbackId Subscribe(EventType type, EventHandlerCallback callback);
        void Unsubscribe(EventType type, CallbackId identifier);

		std::shared_ptr<EventInterface>  DispatchMainThread(const std::shared_ptr<EventInterface>& dispatch_event);
        boost::unique_future<std::shared_ptr<EventInterface>> Dispatch(const std::shared_ptr<EventInterface>& dispatch_event);

		void Shutdown();

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


}  // namespace anh
 
}  // namespace event_dispatcher

#endif  // LIBANH _EVENT_DISPATCHER_EVENT_DISPATCHER_H_