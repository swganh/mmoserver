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

#include "anh/event_dispatcher/event_dispatcher.h"
#include "anh/logger.h"

#include <cassert>
#include <algorithm>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/chrono.hpp>
#include <boost/asio/io_service.hpp>

using namespace swganh::event_dispatcher;
using namespace boost::posix_time;
using namespace std;


using namespace boost::chrono;


using namespace swganh;
using namespace std;

namespace ba = boost::asio;

BaseEvent::BaseEvent(EventType type)
    : type_(type)
{}

EventType BaseEvent::Type() const
{
    return type_;
}

EventDispatcher::EventDispatcher(ba::io_service& io_service)
    : io_service_(io_service)
{}

EventDispatcher::~EventDispatcher()
{
    Shutdown();
}

CallbackId EventDispatcher::Subscribe(EventType type, EventHandlerCallback callback)
{
    auto handler_id = GenerateCallbackId();

    boost::upgrade_lock<boost::shared_mutex> lg(event_handlers_mutex_);

    auto find_iter = event_handlers_.find(type);

    if (find_iter == end(event_handlers_))
    {
        find_iter = event_handlers_.insert(make_pair(type, EventHandlerList())).first;
    }

    find_iter->second.insert(std::make_pair(handler_id, move(callback)));

    return handler_id;
}

void EventDispatcher::Unsubscribe(EventType type, CallbackId identifier)
{
    boost::upgrade_lock<boost::shared_mutex> lg(event_handlers_mutex_);

    auto event_type_iter = event_handlers_.find(type);

    if (event_type_iter != end(event_handlers_))
    {
        auto type_handlers = event_type_iter->second;
        type_handlers.erase(identifier);
        type_handlers.clear();
    }
}

shared_ptr<EventInterface> EventDispatcher::DispatchMainThread(const shared_ptr<EventInterface>& dispatch_event)
{
  
  InvokeCallbacks(dispatch_event);

  return (dispatch_event);
  
    
  //  return task->get_future();
}


boost::unique_future<shared_ptr<EventInterface>> EventDispatcher::Dispatch(const shared_ptr<EventInterface>& dispatch_event)
{
    auto task = make_shared<boost::packaged_task<shared_ptr<EventInterface>>>(
                    [this, dispatch_event] () -> shared_ptr<EventInterface>
    {
        InvokeCallbacks(dispatch_event);

        return dispatch_event;
    });

    io_service_.post([task] ()
    {
        (*task)();
    });

    return task->get_future();
}

CallbackId EventDispatcher::GenerateCallbackId()
{
    return CallbackId(steady_clock::now().time_since_epoch().count());
}

void EventDispatcher::InvokeCallbacks(const shared_ptr<EventInterface>& dispatch_event)
{
    boost::shared_lock<boost::shared_mutex> lg(event_handlers_mutex_);

    auto event_type_iter = event_handlers_.find(dispatch_event->Type());

    if (event_type_iter != end(event_handlers_))
    {
        for_each(
            begin(event_type_iter->second),
            end(event_type_iter->second),
            [&dispatch_event] (const EventHandlerList::value_type& handler)
        {
            try
            {
                handler.second(dispatch_event);
            }
            catch(...)
            {
                LOG(warning) << "A handler callback caused an exception.";
            }
        });
    }
}

void EventDispatcher::Shutdown()
{
    boost::lock_guard<boost::shared_mutex> lg(event_handlers_mutex_);
	EventHandlerMap::iterator it;
	for ( it = event_handlers_.begin(); it != event_handlers_.end(); it++)
		{
			(*it).second.clear();
		}
    event_handlers_.clear();
}



