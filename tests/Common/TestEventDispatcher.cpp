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

#include <gtest/gtest.h>

#include "Common/EventDispatcher.h"
#include "MockObjects/MockListener.h"
#include "MockObjects/MockListenerAlt.h"

using ::common::EventDispatcher;
using ::common::Event;
using ::common::EventType;
using ::common::EventListener;
using ::common::EventListenerCallback;
using ::common::EventListenerType;

namespace {

TEST(EventDispatcherTests, CanConnectListenerToEvent) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener;
   
    // Connect the listener to a test event.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, listener, std::placeholders::_1));
    dispatcher.Connect(EventType("test_event"), EventListener(EventListenerType("MockListener"), callback));

    // Query the dispatcher for a list of the listeners for the test event.
    std::vector<EventListener> listeners = dispatcher.GetListeners(EventType("test_event")).get();

    // Make sure there's 1 and only 1 listener registered.
    EXPECT_EQ(1, listeners.size());

    // Make sure the one item returned back is the same as the one put in.
    EXPECT_EQ(EventListenerType("MockListener"), listeners[0].first);
    EXPECT_EQ(callback, listeners[0].second);
}

TEST(EventDispatcherTests, CanConnectListenerToTwoEvents) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener;
   
    // Connect the listener to two test events.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, listener, std::placeholders::_1));
    dispatcher.Connect(EventType("test_event1"), EventListener(EventListenerType("MockListener"), callback));
    dispatcher.Connect(EventType("test_event2"), EventListener(EventListenerType("MockListener"), callback));

    // Query the dispatcher for a list of the listeners for the test event.
    std::vector<EventListener> listeners1 = dispatcher.GetListeners(EventType("test_event1")).get();
    std::vector<EventListener> listeners2 = dispatcher.GetListeners(EventType("test_event2")).get();

    // Make sure there's 1 listener registered to each event.
    EXPECT_EQ(1, listeners1.size());
    EXPECT_EQ(1, listeners2.size());
}

TEST(EventDispatcherTests, CanConnectTwoListenersToEvent) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener1;
    MockListenerAlt listener2;
   
    // Connect the listeners to a test event.
    EventListenerCallback callback1(std::bind(&MockListener::HandleEvent, listener1, std::placeholders::_1));
    EventListenerCallback callback2(std::bind(&MockListenerAlt::HandleEvent, listener2, std::placeholders::_1));
    
    dispatcher.Connect(EventType("test_event"), EventListener(EventListenerType("MockListener"), callback1));
    dispatcher.Connect(EventType("test_event"), EventListener(EventListenerType("MockListenerAlt"), callback2));

    // Query the dispatcher for a list of the listeners for the test event.
    std::vector<EventListener> listeners = dispatcher.GetListeners(EventType("test_event")).get();

    // Make sure there are 2 listeners registered for this type.
    EXPECT_EQ(2, listeners.size());

    // Make sure the items returned back are the same as the ones put in.
    EXPECT_EQ(EventListenerType("MockListener"), listeners[0].first);
    EXPECT_EQ(callback1, listeners[0].second);

    EXPECT_EQ(EventListenerType("MockListenerAlt"), listeners[1].first);
    EXPECT_EQ(callback2, listeners[1].second);
}

TEST(EventDispatcherTests, CanDisconnectListenerFromEvent) {
    EventDispatcher dispatcher;
    MockListener listener1;
    MockListenerAlt listener2;
    
    // Connect the listeners to a test event.
    EventListenerCallback callback1(std::bind(&MockListener::HandleEvent, listener1, std::placeholders::_1));
    EventListenerCallback callback2(std::bind(&MockListenerAlt::HandleEvent, listener2, std::placeholders::_1));
    
    dispatcher.Connect(EventType("test_event"), EventListener(EventListenerType("MockListener"), callback1));
    dispatcher.Connect(EventType("test_event"), EventListener(EventListenerType("MockListenerAlt"), callback2));
    
    // Query the dispatcher for a list of the listeners for the test event.
    std::vector<EventListener> listeners = dispatcher.GetListeners(EventType("test_event")).get();
    
    // Make sure there are 2 listeners registered for this type.
    EXPECT_EQ(2, listeners.size());

    // Disconnect one of the listeners.
    dispatcher.Disconnect(EventType("test_event"), EventListenerType("MockListener"));
    
    // Query the dispatcher again for a list of the listeners.
    listeners = dispatcher.GetListeners(EventType("test_event")).get();
    
    // Make sure there is only listener registered for this type.
    EXPECT_EQ(1, listeners.size());
    
    // Make sure the correct listener is still registered.
    EXPECT_EQ(EventListenerType("MockListenerAlt"), listeners[0].first);
    EXPECT_EQ(callback2, listeners[0].second);
}

TEST(EventDispatcherTests, CanDisconnectListenerFromAllEvents) {    
    EventDispatcher dispatcher;
    MockListener listener1;
    MockListenerAlt listener2;
    
    // Connect the listeners to 2 different test events.
    EventListenerCallback callback1(std::bind(&MockListener::HandleEvent, listener1, std::placeholders::_1));
    EventListenerCallback callback2(std::bind(&MockListenerAlt::HandleEvent, listener2, std::placeholders::_1));
    
    dispatcher.Connect(EventType("test_event1"), EventListener(EventListenerType("MockListener"), callback1));
    dispatcher.Connect(EventType("test_event1"), EventListener(EventListenerType("MockListenerAlt"), callback2));
    
    dispatcher.Connect(EventType("test_event2"), EventListener(EventListenerType("MockListener"), callback1));
    dispatcher.Connect(EventType("test_event2"), EventListener(EventListenerType("MockListenerAlt"), callback2));
    
    // Query for the listeners to both events and make sure there's 2 items in each.
    std::vector<EventListener> listeners1 = dispatcher.GetListeners(EventType("test_event1")).get();
    std::vector<EventListener> listeners2 = dispatcher.GetListeners(EventType("test_event2")).get();

    EXPECT_EQ(2, listeners1.size());
    EXPECT_EQ(2, listeners2.size());

    // Disconnect an listener from all events.
    dispatcher.DisconnectFromAll(EventListenerType("MockListener"));
    
    // Query for the listeners to both events and make sure there's only 1 item each now.
    listeners1 = dispatcher.GetListeners(EventType("test_event1")).get();
    listeners2 = dispatcher.GetListeners(EventType("test_event2")).get();

    EXPECT_EQ(1, listeners1.size());
    EXPECT_EQ(1, listeners2.size());
}

TEST(EventDispatcherTests, CanGetListOfRegisteredEventTypes) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener;
   
    // Connect the listener to two test events.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, listener, std::placeholders::_1));
    dispatcher.Connect(EventType("test_event1"), EventListener(EventListenerType("MockListener"), callback));
    dispatcher.Connect(EventType("test_event2"), EventListener(EventListenerType("MockListener"), callback));
    dispatcher.Connect(EventType("test_event3"), EventListener(EventListenerType("MockListener"), callback));

    std::vector<EventType> event_types = dispatcher.GetRegisteredEvents().get();
    
    EXPECT_EQ(3, event_types.size());

    EXPECT_EQ(EventType("test_event1"), event_types[0]);
    EXPECT_EQ(EventType("test_event3"), event_types[1]); // When hashed test_event3 happens to be less than test_event2
    EXPECT_EQ(EventType("test_event2"), event_types[2]);
}

TEST(EventDispatcherTests, TriggeringEventQueuesItForProcessing) {
    // Create the EventDispatcher.
    EventDispatcher dispatcher;

    // Create a new event.
    std::shared_ptr<Event> my_event = std::make_shared<Event>(EventType("test_event"));

    // Trigger the event and block on the future until the result is returned.
    dispatcher.Trigger(my_event);

    // Make sure the event has been marked as handled.
    EXPECT_EQ(true, dispatcher.HasEvents().get());
}

TEST(EventDispatcherTests, TriggeringEventSetsTimestamp) {
    // Create the EventDispatcher and initialize it with a current timestamp.
    EventDispatcher dispatcher(100);

    // Create a new event.
    std::shared_ptr<Event> my_event = std::make_shared<Event>(EventType("test_event"));

    // Trigger the event and block on the future until the result is returned.
    dispatcher.Trigger(my_event);    

    // Make sure the event has been marked as handled.
    EXPECT_NE(0, my_event->timestamp());
}

}
