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

#include <algorithm>

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
                                 [] (shared_ptr<IEvent> incoming_event) {return true;});

    EXPECT_FALSE(dispatcher.subscribe("some_event_type", my_listener));
}

/// This test checks to see that the dispatcher has a listener after a
/// successful subscription.
TEST(EventDispatcherTest, HasListenersAfterOneSubscribes) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
                                 [] (shared_ptr<IEvent> incoming_event) {return true;});

    EXPECT_TRUE(dispatcher.subscribe("some_event_type", my_listener));
    EXPECT_TRUE(dispatcher.hasListeners("some_event_type"));
}

/// A single listener shouldn't be registered for the same event more than once.
TEST(EventDispatcherTest, SubscribingListenerToEventTwiceFails) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
                                 [] (shared_ptr<IEvent> incoming_event) {return true;});

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
                                 [] (shared_ptr<IEvent> incoming_event) {return true;});

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
                                 [] (shared_ptr<IEvent> incoming_event) {return true;});

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
                                 [] (shared_ptr<IEvent> incoming_event) {return true;});

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
        [&notified] (shared_ptr<IEvent> incoming_event) -> bool {
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
        [] (shared_ptr<IEvent> incoming_event) {return true;});
    
    dispatcher.subscribe("some_event_type", my_listener);

    auto my_event = make_shared<SimpleEvent>("some_event_type");

    EXPECT_TRUE(dispatcher.triggerAsync(my_event));

    EXPECT_TRUE(dispatcher.hasEvents());
}

/// Ticking dispatches queued events.
TEST(EventDispatcherTest, TickingDispatchesQueuedEvents) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
        [] (shared_ptr<IEvent> incoming_event) {return true;});
    
    dispatcher.subscribe("some_event_type", my_listener);

    auto my_event = make_shared<SimpleEvent>("some_event_type");

    EXPECT_TRUE(dispatcher.triggerAsync(my_event));
    EXPECT_TRUE(dispatcher.hasEvents());
    EXPECT_TRUE(dispatcher.tick());
    EXPECT_FALSE(dispatcher.hasEvents());
}

/// Aborting an event type cancels the first occurrance of it in the queue.
TEST(EventDispatcherTest, AbortingEventCancelsFirstOccurance) {    
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
        [] (shared_ptr<IEvent> incoming_event) {return true;});
    
    dispatcher.subscribe("some_event_type", my_listener);

    auto my_event = make_shared<SimpleEvent>("some_event_type");

    EXPECT_TRUE(dispatcher.triggerAsync(my_event));
    EXPECT_TRUE(dispatcher.hasEvents());
    EXPECT_TRUE(dispatcher.abort("some_event_type"));
    EXPECT_FALSE(dispatcher.hasEvents());

    // load up multiples and ensure only one occurance is removed    
    EXPECT_TRUE(dispatcher.triggerAsync(my_event));
    EXPECT_TRUE(dispatcher.triggerAsync(my_event));
    EXPECT_TRUE(dispatcher.hasEvents());
    EXPECT_TRUE(dispatcher.abort("some_event_type"));
    EXPECT_TRUE(dispatcher.hasEvents());
}

/// Can abort all occurrances of an event in the queue.
TEST(EventDispatcherTest, CanAbortAllEventOccurrances) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
        [] (shared_ptr<IEvent> incoming_event) {return true;});
    
    dispatcher.subscribe("some_event_type", my_listener);

    auto my_event = make_shared<SimpleEvent>("some_event_type");

    // load up multiples and ensure all are removed    
    EXPECT_TRUE(dispatcher.triggerAsync(my_event));
    EXPECT_TRUE(dispatcher.triggerAsync(my_event));
    EXPECT_TRUE(dispatcher.triggerAsync(my_event));
    EXPECT_TRUE(dispatcher.hasEvents());
    EXPECT_TRUE(dispatcher.abort("some_event_type", true));
    EXPECT_FALSE(dispatcher.hasEvents());
}

/// Verify that we can get a copy of the currently registered events.
TEST(EventDispatcherTest, CanGetListOfRegisteredEventTypes) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("event_type_1");
    dispatcher.registerEventType("event_type_2");
    dispatcher.registerEventType("event_type_3");

    const EventTypeSet& event_types = dispatcher.registered_event_types();

    EXPECT_EQ(uint32_t(3), event_types.size());

    auto find_it = std::find(event_types.begin(), event_types.end(), "event_type_1");
    EXPECT_FALSE(find_it == event_types.end());
    
    find_it = std::find(event_types.begin(), event_types.end(), "event_type_2");
    EXPECT_FALSE(find_it == event_types.end());
    
    find_it = std::find(event_types.begin(), event_types.end(), "event_type_3");
    EXPECT_FALSE(find_it == event_types.end());
}

/// Trigger a callback after processing an event.
TEST(EventDispatcherTest, CallbackIsTriggeredAfterEventProcessing) {    
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
        [] (shared_ptr<IEvent> incoming_event) {return true;});
    
    dispatcher.subscribe("some_event_type", my_listener);

    auto my_event = make_shared<SimpleEvent>("some_event_type");

    bool callback_triggered = false;
    dispatcher.trigger(my_event, [&callback_triggered] (shared_ptr<IEvent> triggered_event, bool processed) {
        callback_triggered = true;
    });

    EXPECT_TRUE(callback_triggered);
}

/// Trigger a callback after queued event is processed.
TEST(EventDispatcherTest, CallbackIsTriggeredAfterQueuedEventProcessing) {    
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
        [] (shared_ptr<IEvent> incoming_event) {return true;});    
    dispatcher.subscribe("some_event_type", my_listener);

    auto my_event = make_shared<SimpleEvent>("some_event_type");

    // load up multiples and ensure all are removed    
    bool callback_triggered = false;
    EXPECT_TRUE(dispatcher.triggerAsync(my_event, [&callback_triggered] (shared_ptr<IEvent> triggered_event, bool processed) {
        callback_triggered = true;
    }));

    dispatcher.tick();
    
    EXPECT_TRUE(callback_triggered);
}

/// Can conditionally wait for a condition to be true for an event to fire.
TEST(EventDispatcherTest, EventWaitsForCondition) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
        [] (shared_ptr<IEvent> incoming_event) {return true;});
    
    dispatcher.subscribe("some_event_type", my_listener);

    auto my_event = make_shared<SimpleEvent>("some_event_type");

    bool callback_triggered = false;
    bool proceed = false;
    dispatcher.triggerWhen(my_event, 
        [&proceed] (uint32_t current_time_ms) {
            return proceed;
        },
        [&callback_triggered] (shared_ptr<IEvent> triggered_event, bool processed) {
            callback_triggered = true;
        });

    EXPECT_FALSE(callback_triggered);
    
    // Tick forward and make sure it still hasn't processed
    dispatcher.tick();
    EXPECT_FALSE(callback_triggered);

    // Allow our callback to continue.
    proceed = true;
    dispatcher.tick();
    EXPECT_TRUE(callback_triggered);
}

/// Triggering sets timestamp for event.
TEST(EventDispatcherTest, TriggeringEventSetsTimestamp) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");
    
    auto my_listener = make_pair(EventListenerType("some_listener_type"), 
        [] (shared_ptr<IEvent> incoming_event) {return true;});
    
    dispatcher.subscribe("some_event_type", my_listener);

    auto my_event1 = make_shared<SimpleEvent>("some_event_type");
    auto my_event2 = make_shared<SimpleEvent>("some_event_type");

    EXPECT_TRUE(dispatcher.trigger(my_event1));
    EXPECT_TRUE(dispatcher.triggerAsync(my_event2));
    EXPECT_GT(my_event1->timestamp(), 0);
    EXPECT_GT(my_event2->timestamp(), 0);
}
