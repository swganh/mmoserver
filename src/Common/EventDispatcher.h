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

#ifndef SRC_COMMON_EVENTDISPATCHER_H_
#define SRC_COMMON_EVENTDISPATCHER_H_

#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <set>

#include "Common/Event.h"

namespace common {

typedef std::function<bool (Event*)> EventListenerCallback; 
typedef std::list<EventListenerCallback> EventListenerList;

class EventDispatcher {
public:
    EventDispatcher();
    ~EventDispatcher();

    /**
     * Connects an event listener to the specified event. 
     *
     * \param event_type The event type to check for connected listeners.
     * \param callback The callback method provided by the event listener to 
     *                 be called when an event is triggered.
     * \returns Returns true if the listener was successfully connected, false if not.
     */
    bool Connect(const EventType& event_type, EventListenerCallback callback);

    /**
     * Gets all of the listeners connected to a specific event type.
     *
     * \param event_type The event type to check for connected listeners.
     * \return A list of the connected listeners to the specified event.
     */
    EventListenerList GetListeners(const EventType& event_type) const;

private:
    /// Disable the default copy constructor.
    EventDispatcher(const EventDispatcher&);

    /// Disable the default assignment operator.
    EventDispatcher& operator=(const EventDispatcher&);

    bool ValidateEventType_(const EventType& event_type) const;
    bool AddEventType_(const EventType& event_type);

    typedef std::set<EventType> EventTypeSet;
    EventTypeSet event_type_set_;

    typedef std::map<EventType, EventListenerList> EventListenerMap;
    EventListenerMap event_listener_map_;
};

}  // namespace common

#endif  // SRC_COMMON_EVENTDISPATCHER_H_
