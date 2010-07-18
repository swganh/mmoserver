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

// Use a HashString as the basis for EventListenerType's.
typedef HashString EventListenerType;

// Most of the time spent processing events will be iterating over a collection
// of callbacks to notify about an event so we want to make use of std::list, however,
// very infrequently we also want to be able to add and remove listeners and need a 
// way to identify them in the list so a std::pair is used as the node.
typedef std::pair<EventListenerType, EventListenerCallback> EventListener;
typedef std::list<EventListener> EventListenerList;

class EventDispatcher {
public:
    EventDispatcher();
    ~EventDispatcher();

    /**
     * Connects an event listener to an event. 
     *
     * \param event_type The event type to check for connected listeners.
     * \param listener The listener interested in the event specified.
     */
    void Connect(const EventType& event_type, EventListener listener);

    /**
     * Disconnects an event listener from an event.
     *
     * \param event_type The event type to disconnect from.
     * \param event_listener_type The type of the event listener being disconnected.
     */
    void Disconnect(const EventType& event_type, const EventListenerType& event_listener_type);

    /**
     * Disconnects an event listener from all events.
     *
     * \param event_listener_type The type of the event listener being disconnected.
     */
    void DisconnectFromAll(const EventListenerType& event_listener_type);

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
    bool ValidateEventListenerType_(const EventListenerType& event_listener_type) const;
    bool AddEventType_(const EventType& event_type);

    typedef std::set<EventType> EventTypeSet;
    EventTypeSet event_type_set_;

    typedef std::map<EventType, EventListenerList> EventListenerMap;
    EventListenerMap event_listener_map_;
};

}  // namespace common

#endif  // SRC_COMMON_EVENTDISPATCHER_H_
