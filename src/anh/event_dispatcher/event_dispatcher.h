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

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "anh/event_dispatcher/basic_event.h"

namespace anh {
namespace event_dispatcher {

typedef std::function<void (std::shared_ptr<BaseEvent>)> EventListener;
typedef std::list<EventListener> EventListenerList;
typedef std::map<EventType, EventListenerList> EventListenerMap;
typedef std::set<EventType> EventTypeSet;

class EventDispatcher {
public:
    EventDispatcher();
    ~EventDispatcher();

    bool hasListeners(const EventType& event_type) const;
    bool hasRegisteredEventType(const EventType& event_type) const;

    bool registerEventType(EventType event_type);

    bool subscribe(const EventType& event_type, EventListener listener);

private:
    EventTypeSet registered_event_types_;
};

}  // namespace event_dispatcher
}  // namespace anh

#endif  // LIBANH _EVENT_DISPATCHER_EVENT_DISPATCHER_H_
