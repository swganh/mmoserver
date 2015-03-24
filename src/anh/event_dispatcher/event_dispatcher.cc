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

<<<<<<< HEAD
<<<<<<< HEAD

using namespace boost::chrono;


using namespace swganh;
using namespace std;

namespace ba = boost::asio;
=======
=======
>>>>>>> parent of 5bd772a... got rid of google log
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
<<<<<<< HEAD
        return false;
    }
>>>>>>> parent of 5bd772a... got rid of google log

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

<<<<<<< HEAD
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
=======
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
=======
>>>>>>> parent of 5bd772a... got rid of google log
        return false;
    }

    listener_list.push_back(listener);

    return true;
}

<<<<<<< HEAD
void EventDispatcher::unsubscribe(const EventType& event_type, const EventListenerType& listener_type) {    
=======
EventTypeSet EventDispatcher::registered_event_types() const {
    return registered_event_types_;
}

bool EventDispatcher::subscribe(const EventType& event_type, EventListener listener) {
    if (!validateEventType_(event_type)) {
        return false;
    }

>>>>>>> parent of 5bd772a... got rid of google log
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

<<<<<<< HEAD
    auto remove_it = remove_if(listener_list.begin(), listener_list.end(), [&listener_type] (const EventListener& list_listener) {
        return list_listener.first == listener_type;
=======
    // Look for the listener in the list before adding.
    auto find_it = find_if(listener_list.begin(), listener_list.end(), [&listener] (const EventListener& list_listener) {
        return list_listener.first == listener.first;
>>>>>>> parent of 5bd772a... got rid of google log
    });

    if (find_it != listener_list.end()) {
        return false;
    }

    listener_list.push_back(listener);

    return true;
}

<<<<<<< HEAD
=======
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

>>>>>>> parent of 5bd772a... got rid of google log
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
>>>>>>> parent of 5bd772a... got rid of google log
}

<<<<<<< HEAD
shared_ptr<EventInterface> EventDispatcher::DispatchMainThread(const shared_ptr<EventInterface>& dispatch_event)
{
  
  InvokeCallbacks(dispatch_event);

<<<<<<< HEAD
  return (dispatch_event);
  
=======
=======

>>>>>>> parent of 5bd772a... got rid of google log
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
>>>>>>> parent of 5bd772a... got rid of google log
    
	//return task->get_future();
}


<<<<<<< HEAD
<<<<<<< HEAD
boost::unique_future<shared_ptr<EventInterface>> EventDispatcher::Dispatch(const shared_ptr<EventInterface>& dispatch_event)
{
    auto task = make_shared<boost::packaged_task<shared_ptr<EventInterface>>>(
                    [this, dispatch_event] () -> shared_ptr<EventInterface>
    {
        InvokeCallbacks(dispatch_event);
=======
=======
>>>>>>> parent of 5bd772a... got rid of google log
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
>>>>>>> parent of 5bd772a... got rid of google log

        return dispatch_event;
    });

    io_service_.post([task] ()
    {
        (*task)();
    });

    return task->get_future();
}

<<<<<<< HEAD
CallbackId EventDispatcher::GenerateCallbackId()
{
    return CallbackId(steady_clock::now().time_since_epoch().count());
=======

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
>>>>>>> parent of 5bd772a... got rid of google log
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

<<<<<<< HEAD
=======
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
>>>>>>> parent of 5bd772a... got rid of google log


