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

#include <gtest/gtest.h>

#include "anh/event_dispatcher/basic_event.h"
#include "anh/event_dispatcher/event_dispatcher.h"

using namespace std;
using namespace anh::event_dispatcher;

/// By default a new instance of the dispatcher should not have any listeners.
TEST(EventDispatcherTest, HasNoListenersByDefault) {
    EventDispatcher dispatcher;

    EXPECT_FALSE(dispatcher.hasListeners("some_event_type"));
}

/// By default a new instance of the dispatcher should not have any event types
/// registered to it.
TEST(EventDispatcherTest, HasNoRegisteredEventTypesByDefault) {
    EventDispatcher dispatcher;

    EXPECT_FALSE(dispatcher.hasRegisteredEventType("some_event_type"));
}

/// Verify event types can be registered to the event dispatcher.
TEST(EventDispatcherTest, CanRegisterEventType) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");

    EXPECT_TRUE(dispatcher.hasRegisteredEventType("some_event_type"));
}

/// Ensure that a listener cannot subscribe to an event that has not yet been registered.
TEST(EventDispatcherTest, CannotSubscribeToUnregisteredEvent) {
    EventDispatcher dispatcher;

    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
                                 [] (shared_ptr<BaseEvent> incoming_event) {return true;});

    EXPECT_FALSE(dispatcher.subscribe("some_event_type", my_listener));
}

/// This test checks to see that the dispatcher has a listener after a
/// successful subscription.
TEST(EventDispatcherTest, HasListenersAfterOneSubscribes) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
                                 [] (shared_ptr<BaseEvent> incoming_event) {return true;});

    EXPECT_TRUE(dispatcher.subscribe("some_event_type", my_listener));
    EXPECT_TRUE(dispatcher.hasListeners("some_event_type"));
}

/// A single listener shouldn't be registered for the same event more than once.
TEST(EventDispatcherTest, SubscribingListenerToEventTwiceFails) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
                                 [] (shared_ptr<BaseEvent> incoming_event) {return true;});

    // Should succeed the first time.
    EXPECT_TRUE(dispatcher.subscribe("some_event_type", my_listener));

    // Then fail on subsequent requests.
    EXPECT_FALSE(dispatcher.subscribe("some_event_type", my_listener));
    EXPECT_FALSE(dispatcher.subscribe("some_event_type", my_listener));
    EXPECT_FALSE(dispatcher.subscribe("some_event_type", my_listener));
}

/// Verify that a listener can subscribe to multiple events.
TEST(EventDispatcherTest, CanSubscribeToMultipleEvents) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("event_type_1");
    dispatcher.registerEventType("event_type_2");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
                                 [] (shared_ptr<BaseEvent> incoming_event) {return true;});

    EXPECT_TRUE(dispatcher.subscribe("event_type_1", my_listener));
    EXPECT_TRUE(dispatcher.hasListeners("event_type_1"));
    
    EXPECT_TRUE(dispatcher.subscribe("event_type_2", my_listener));
    EXPECT_TRUE(dispatcher.hasListeners("event_type_2"));
}

/// Verify that a listener can be unsubscribed from a certain event type.
TEST(EventDispatcherTest, CanUnsubscribeFromEventType) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
                                 [] (shared_ptr<BaseEvent> incoming_event) {return true;});

    // Should succeed the first time.
    EXPECT_TRUE(dispatcher.subscribe("some_event_type", my_listener));
    EXPECT_TRUE(dispatcher.hasListeners("some_event_type"));
    
    dispatcher.unsubscribe("some_event_type", "some_listener_type");

    EXPECT_FALSE(dispatcher.hasListeners("some_event_type"));
}

/// Verify that a listener can be unsubscribed from all event types.
TEST(EventDispatcherTest, CanUnsubscribeFromAllEventTypes) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("event_type_1");
    dispatcher.registerEventType("event_type_2");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
                                 [] (shared_ptr<BaseEvent> incoming_event) {return true;});

    EXPECT_TRUE(dispatcher.subscribe("event_type_1", my_listener));
    EXPECT_TRUE(dispatcher.hasListeners("event_type_1"));
    
    EXPECT_TRUE(dispatcher.subscribe("event_type_2", my_listener));
    EXPECT_TRUE(dispatcher.hasListeners("event_type_2"));

    dispatcher.unsubscribe("some_listener_type");

    EXPECT_FALSE(dispatcher.hasListeners("event_type_1"));
    EXPECT_FALSE(dispatcher.hasListeners("event_type_2"));
}


/// Triggering an event should notify listeners subscribed to its type.
TEST(EventDispatcherTest, TriggeringEventNotifiesListeners) {
    EventDispatcher dispatcher;

    // We'll use this to signal whether or not we've been notified.
    bool notified = false;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
        [&notified] (shared_ptr<BaseEvent> incoming_event) -> bool {
            notified = true;
            return true;
        });
    
    dispatcher.subscribe("some_event_type", my_listener);

    auto my_event = make_shared<SimpleEvent>("some_event_type");

    EXPECT_TRUE(dispatcher.trigger(my_event));

    EXPECT_TRUE(notified);
}



/// Triggering an event asyncronously fires the event during the primary
/// event processing time.
TEST(EventDispatcherTest, TriggeringAsyncQueuesEvent) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
        [] (shared_ptr<BaseEvent> incoming_event) {return true;});
    
    dispatcher.subscribe("some_event_type", my_listener);

    auto my_event = make_shared<SimpleEvent>("some_event_type");

    EXPECT_TRUE(dispatcher.triggerAsync(my_event));

    EXPECT_TRUE(dispatcher.hasEvents());
}
