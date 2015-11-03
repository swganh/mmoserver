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

using common::EventDispatcher;
using common::IEventPtr;
using common::EventType;
using common::EventListener;
using common::EventListenerCallback;
using common::EventListenerType;

namespace {

class MockEvent : public ::common::BaseEvent {
public:
    explicit MockEvent(uint64_t subject_id = 0, uint64_t delay_ms = 0) 
        : BaseEvent(subject_id, delay_ms)
    , some_event_val_(0) {}

    explicit MockEvent(uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback) 
        : BaseEvent(subject_id, delay_ms, callback)
    , some_event_val_(0) {}

    explicit MockEvent(::common::ByteBuffer& in) {
        deserialize(in);
    }

    ~MockEvent() {}

    const ::common::EventType& event_type() const { return event_type_; }

    int some_event_val() const { return some_event_val_; }
    void some_event_val(int some_event_val) { some_event_val_ = some_event_val; }

private:
    void onSerialize(::common::ByteBuffer& out) const {
        out << some_event_val_;
    }

    void onDeserialize(::common::ByteBuffer& in) {
        some_event_val_ = in.read<int>();
    }

    bool onConsume(bool handled) const {
        return true;
    }

    static const ::common::EventType event_type_;
    int some_event_val_;
};

const ::common::EventType MockEvent::event_type_ = ::common::EventType("mock_event");

class MockListener {
public:
    MockListener() : triggered_(false) {}
    ~MockListener() {}

    bool triggered() const { return triggered_; }

    bool HandleEvent(::common::IEventPtr triggered_event) {
        triggered_ = true;
        return true;
    }

private:
    bool triggered_;
};

class MockListenerAlt {
public:
    MockListenerAlt() {}
    ~MockListenerAlt() {}

    bool HandleEvent(::common::IEventPtr triggered_event) {
        return true;
    }
};
    
TEST(EventDispatcherTests, CanConnectListenerToEvent) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener;
   
    // Connect the listener to a test event.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, &listener, std::placeholders::_1));
    dispatcher.Connect(EventType("test_event"), EventListener(EventListenerType("MockListener"), callback));

    // Query the dispatcher for a list of the listeners for the test event.
    std::vector<EventListener> listeners = dispatcher.GetListeners(EventType("test_event")).get();

    // Make sure there's 1 and only 1 listener registered.
    EXPECT_EQ(uint32_t(1), listeners.size());

    // Make sure the one item returned back is the same as the one put in.
    EXPECT_EQ(EventListenerType("MockListener"), listeners[0].first);
}

TEST(EventDispatcherTests, CanConnectListenerToTwoEvents) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener;
   
    // Connect the listener to two test events.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, &listener, std::placeholders::_1));
    dispatcher.Connect(EventType("test_event1"), EventListener(EventListenerType("MockListener"), callback));
    dispatcher.Connect(EventType("test_event2"), EventListener(EventListenerType("MockListener"), callback));

    // Query the dispatcher for a list of the listeners for the test event.
    std::vector<EventListener> listeners1 = dispatcher.GetListeners(EventType("test_event1")).get();
    std::vector<EventListener> listeners2 = dispatcher.GetListeners(EventType("test_event2")).get();

    // Make sure there's 1 listener registered to each event.
    EXPECT_EQ(uint32_t(1), listeners1.size());
    EXPECT_EQ(uint32_t(1), listeners2.size());
}

TEST(EventDispatcherTests, CanConnectTwoListenersToEvent) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener1;
    MockListenerAlt listener2;
   
    // Connect the listeners to a test event.
    EventListenerCallback callback1(std::bind(&MockListener::HandleEvent, &listener1, std::placeholders::_1));
    EventListenerCallback callback2(std::bind(&MockListenerAlt::HandleEvent, &listener2, std::placeholders::_1));
    
    dispatcher.Connect(EventType("test_event"), EventListener(EventListenerType("MockListener"), callback1));
    dispatcher.Connect(EventType("test_event"), EventListener(EventListenerType("MockListenerAlt"), callback2));

    // Query the dispatcher for a list of the listeners for the test event.
    std::vector<EventListener> listeners = dispatcher.GetListeners(EventType("test_event")).get();

    // Make sure there are 2 listeners registered for this type.
    EXPECT_EQ(uint32_t(2), listeners.size());

    // Make sure the items returned back are the same as the ones put in.
    EXPECT_EQ(EventListenerType("MockListener"), listeners[0].first);
    EXPECT_EQ(EventListenerType("MockListenerAlt"), listeners[1].first);
}

TEST(EventDispatcherTests, CanDisconnectListenerFromEvent) {
    EventDispatcher dispatcher;
    MockListener listener1;
    MockListenerAlt listener2;
    
    // Connect the listeners to a test event.
    EventListenerCallback callback1(std::bind(&MockListener::HandleEvent, &listener1, std::placeholders::_1));
    EventListenerCallback callback2(std::bind(&MockListenerAlt::HandleEvent, &listener2, std::placeholders::_1));
    
    dispatcher.Connect(EventType("test_event"), EventListener(EventListenerType("MockListener"), callback1));
    dispatcher.Connect(EventType("test_event"), EventListener(EventListenerType("MockListenerAlt"), callback2));
    
    // Query the dispatcher for a list of the listeners for the test event.
    std::vector<EventListener> listeners = dispatcher.GetListeners(EventType("test_event")).get();
    
    // Make sure there are 2 listeners registered for this type.
    EXPECT_EQ(uint32_t(2), listeners.size());

    // Disconnect one of the listeners.
    dispatcher.Disconnect(EventType("test_event"), EventListenerType("MockListener"));
    
    // Query the dispatcher again for a list of the listeners.
    listeners = dispatcher.GetListeners(EventType("test_event")).get();
    
    // Make sure there is only listener registered for this type.
    EXPECT_EQ(uint32_t(1), listeners.size());
    
    // Make sure the correct listener is still registered.
    EXPECT_EQ(EventListenerType("MockListenerAlt"), listeners[0].first);
}

TEST(EventDispatcherTests, CanDisconnectListenerFromAllEvents) {    
    EventDispatcher dispatcher;
    MockListener listener1;
    MockListenerAlt listener2;
    
    // Connect the listeners to 2 different test events.
    EventListenerCallback callback1(std::bind(&MockListener::HandleEvent, &listener1, std::placeholders::_1));
    EventListenerCallback callback2(std::bind(&MockListenerAlt::HandleEvent, &listener2, std::placeholders::_1));
    
    dispatcher.Connect(EventType("test_event1"), EventListener(EventListenerType("MockListener"), callback1));
    dispatcher.Connect(EventType("test_event1"), EventListener(EventListenerType("MockListenerAlt"), callback2));
    
    dispatcher.Connect(EventType("test_event2"), EventListener(EventListenerType("MockListener"), callback1));
    dispatcher.Connect(EventType("test_event2"), EventListener(EventListenerType("MockListenerAlt"), callback2));
    
    // Query for the listeners to both events and make sure there's 2 items in each.
    std::vector<EventListener> listeners1 = dispatcher.GetListeners(EventType("test_event1")).get();
    std::vector<EventListener> listeners2 = dispatcher.GetListeners(EventType("test_event2")).get();

    EXPECT_EQ(uint32_t(2), listeners1.size());
    EXPECT_EQ(uint32_t(2), listeners2.size());

    // Disconnect an listener from all events.
    dispatcher.DisconnectFromAll(EventListenerType("MockListener"));
    
    // Query for the listeners to both events and make sure there's only 1 item each now.
    listeners1 = dispatcher.GetListeners(EventType("test_event1")).get();
    listeners2 = dispatcher.GetListeners(EventType("test_event2")).get();

    EXPECT_EQ(uint32_t(1), listeners1.size());
    EXPECT_EQ(uint32_t(1), listeners2.size());
}

TEST(EventDispatcherTests, CanGetListOfRegisteredEventTypes) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener;
   
    // Connect the listener to two test events.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, &listener, std::placeholders::_1));
    dispatcher.Connect(EventType("test_event1"), EventListener(EventListenerType("MockListener"), callback));
    dispatcher.Connect(EventType("test_event2"), EventListener(EventListenerType("MockListener"), callback));
    dispatcher.Connect(EventType("test_event3"), EventListener(EventListenerType("MockListener"), callback));

    std::vector<EventType> event_types = dispatcher.GetRegisteredEvents().get();
    
    EXPECT_EQ(uint32_t(3), event_types.size());

    EXPECT_EQ(EventType("test_event1"), event_types[0]);
    EXPECT_EQ(EventType("test_event3"), event_types[1]); // When hashed test_event3 happens to be less than test_event2
    EXPECT_EQ(EventType("test_event2"), event_types[2]);
}

TEST(EventDispatcherTests, NotifyingListenersQueuesEventForProcessing) {
    // Create the EventDispatcher.
    EventDispatcher dispatcher;

    // Create a new event.
    IEventPtr my_event = std::make_shared<MockEvent>();
    
    // Make sure the dispatcher does not have events waiting.
    EXPECT_FALSE(dispatcher.HasEvents().get());

    // Trigger the event.
    dispatcher.Notify(my_event);

    // Make sure the dispatcher has events waiting.
    EXPECT_TRUE(dispatcher.HasEvents().get());
}

TEST(EventDispatcherTests, DeliveringEventCallsAppropriateListener) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener;
   
    // Connect the listener to a test event.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, &listener, std::placeholders::_1));
    dispatcher.Connect(EventType("mock_event"), EventListener(EventListenerType("MockListener"), callback));
    
    // Create a new event.
    IEventPtr my_event = std::make_shared<MockEvent>();

    // Deliver the event.
    EXPECT_TRUE(dispatcher.Deliver(my_event).get());
    EXPECT_TRUE(listener.triggered());
}

TEST(EventDispatcherTests, DeliveringEventOfUnknownTypeIsSuccessful) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener;
   
    // Connect the listener to a test event.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, &listener, std::placeholders::_1));
    dispatcher.Connect(EventType("some_alt_event"), EventListener(EventListenerType("MockListener"), callback));
    
    // Create a new event.
    IEventPtr my_event = std::make_shared<MockEvent>();

    // Deliver the event.
    EXPECT_TRUE(dispatcher.Deliver(my_event).get());
}

TEST(EventDispatcherTests, DeliveringEventCallsGlobalListeners) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener;
   
    // Connect the listener to a test event.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, &listener, std::placeholders::_1));
    dispatcher.Connect(EventType(::common::kWildCardHashString), EventListener(EventListenerType("MockListener"), callback));
    
    // Create a new event.
    IEventPtr my_event = std::make_shared<MockEvent>();

    // Deliver the event and verify the global listener was called.
    dispatcher.Deliver(my_event).get();
    EXPECT_TRUE(listener.triggered());
}

TEST(EventDispatcherTests, CallingTickProcessesQueuedEvents) {
    // Create the EventDispatcher.
    EventDispatcher dispatcher;  
    MockListener listener;
   
    // Connect the listener to a test event.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, &listener, std::placeholders::_1));
    dispatcher.Connect(EventType("mock_event"), EventListener(EventListenerType("MockListener"), callback));
    
    // Create a new event.
    IEventPtr my_event = std::make_shared<MockEvent>();

    // Trigger the event.
    dispatcher.Notify(my_event);

    // Make sure the dispatcher has waiting events.
    EXPECT_TRUE(dispatcher.HasEvents().get());

    // Call tick on the dispatcher.
    dispatcher.Tick(1);
    
    // Make sure there are no waiting events.
    EXPECT_FALSE(dispatcher.HasEvents().get());
    EXPECT_TRUE(listener.triggered());
}

TEST(EventDispatcherTests, CallingTickUpdatesTimestamp) {
    EventDispatcher dispatcher;

    EXPECT_EQ(uint64_t(0), dispatcher.current_timestep().get());

    dispatcher.Tick(10);

    EXPECT_EQ(uint64_t(10), dispatcher.current_timestep().get());
}

TEST(EventDispatcherTests, SuccessfullDeliveryInvokesEventCallback) {
    // Create some data to be modified in the callback.
    std::shared_ptr<int> someval = std::make_shared<int>(0);

    // Create the EventDispatcher.
    EventDispatcher dispatcher;

    // Create a new event.
    IEventPtr my_event = std::make_shared<MockEvent>(0, 0, [=] {
        *someval = 1;
    });

    // Deliver the event.
    dispatcher.Deliver(my_event).get();
    
    // Make sure the value was updated.
    EXPECT_EQ(1, *someval);
}


TEST(EventDispatcherTests, CallingTickWithNonSequentialIntervalFails) {
    EventDispatcher dispatcher(10);

    // Try to tick with a time in the past.
    EXPECT_FALSE(dispatcher.Tick(9).get());
}

TEST(EventDispatcherTests, ChainedEventsAreAddedToQueueOnSuccessfulDelivery) {
    // Create some data to be modified in the callback.
    std::shared_ptr<int> someval = std::make_shared<int>(0);

    // Create the EventDispatcher.
    EventDispatcher dispatcher;

    // Create a new event.
    
    auto my_event1 = std::make_shared<MockEvent>(0, 0, [=] {
        *someval = 1;
    });
        
    auto my_event2 = std::make_shared<MockEvent>(0, 0, [=] {
        *someval = 2;
    });

    my_event1->next(my_event2);
    
    // Make sure the dispatcher has no events waiting.
    EXPECT_FALSE(dispatcher.HasEvents().get());

    // Deliver the event.
    dispatcher.Deliver(my_event1).get();
    
    // Make sure the value was updated.
    EXPECT_EQ(1, *someval);

    // Make sure that the event dispatcher now has something in the queue.
    EXPECT_TRUE(dispatcher.HasEvents().get());

    // Tick forward so the queued event is processed.
    dispatcher.Tick(1).get();
    
    // Make sure the value was updated.
    EXPECT_EQ(2, *someval);
}

TEST(EventDispatcherTests, DelayedEventsAreOnlyProcessedAfterTimeoutHasBeenReached) {
    // Create the EventDispatcher.
    EventDispatcher dispatcher;  
    MockListener listener;
   
    // Connect the listener to a test event.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, &listener, std::placeholders::_1));
    dispatcher.Connect(EventType("mock_event"), EventListener(EventListenerType("MockListener"), callback));
    
    // Create a new event with a delay of 5 milliseconds.
    auto my_event = std::make_shared<MockEvent>(0, 5);

    // Trigger the event.
    dispatcher.Notify(my_event);

    // Make sure the dispatcher has waiting events.
    EXPECT_TRUE(dispatcher.HasEvents().get());

    // Call tick on the dispatcher.
    dispatcher.Tick(1);
    
    // Make sure the dispatcher still has the waiting event and the listener hasn't been triggered.
    EXPECT_TRUE(dispatcher.HasEvents().get());
    EXPECT_FALSE(listener.triggered());
    
    // Call tick on the dispatcher.
    dispatcher.Tick(5);
    
    // Make sure the dispatcher no events and the listener has been triggered.
    EXPECT_FALSE(dispatcher.HasEvents().get());
    EXPECT_TRUE(listener.triggered());
}

TEST(EventDispatcherTests, DeliveringNullEventReturnsFalse) {
    // Create the EventDispatcher.
    EventDispatcher dispatcher;

    // Deliver the event.
    EXPECT_FALSE(dispatcher.Deliver(nullptr).get());
}

TEST(EventDispatcherTests, TriggeringNullEventDoesNothing) {
    // Create the EventDispatcher.
    EventDispatcher dispatcher;

    // Notify the dispatcher with a null event.
    dispatcher.Notify(nullptr);
}

TEST(EventDispatcherTests, NotifyingListenersSetsTimestamp) {
    // Create the EventDispatcher and initialize it with a current timestamp.
    EventDispatcher dispatcher(100);

    // Create a new event.
    auto my_event = std::make_shared<MockEvent>();

    // Trigger the event and block on the future until the result is returned.
    dispatcher.Notify(my_event);    
    dispatcher.Tick(1).get();

    EXPECT_EQ(uint64_t(100), my_event->timestamp());
}

TEST(EventDispatcherTests, DeliveringEventsSetsTimestamp) {
    // Create the EventDispatcher and initialize it with a current timestamp.
    EventDispatcher dispatcher(100);

    // Create a new event.
    auto my_event = std::make_shared<MockEvent>();

    // Deliver the event.
    dispatcher.Deliver(my_event).get();    

    EXPECT_EQ(uint64_t(100), my_event->timestamp());
}

}
