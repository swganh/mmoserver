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

#include <memory>

#include "Common/EventDispatcher.h"
#include "MockObjects/MockListener.h"
#include "MockObjects/MockListenerAlt.h"

using ::common::EventDispatcher;
using ::common::EventType;
using ::common::EventListenerCallback;
using ::common::EventListenerList;

TEST(EventDispatcherTests, CanConnectListenerToEvent) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener;
   
    // Connect the listener to a test event.
    EventListenerCallback callback(std::bind(&MockListener::HandleEvent, std::ref(listener), std::placeholders::_1));
    dispatcher.Connect(EventType("test_event"), callback);

    // Query the dispatcher for a list of the listeners for the test event.
    EventListenerList listeners = dispatcher.GetListeners(EventType("test_event"));

    // Make sure there's 1 and only 1 listener registered.
    EXPECT_EQ(1, listeners.size());

    // Make sure the one item returned back is the same as the one put in.
    EXPECT_EQ(callback, listeners.front());
}

TEST(EventDispatcherTests, CanConnectTwoListenersToEvent) {
    // Create the EventDispatcher and a MockListener to use for testing.
    EventDispatcher dispatcher;   
    MockListener listener1;
    MockListenerAlt listener2;
   
    // Connect the listeners to a test event.
    EventListenerCallback callback1(std::bind(&MockListener::HandleEvent, std::ref(listener1), std::placeholders::_1));
    EventListenerCallback callback2(std::bind(&MockListenerAlt::HandleEvent, std::ref(listener2), std::placeholders::_1));

    dispatcher.Connect(EventType("test_event"), callback1);
    dispatcher.Connect(EventType("test_event"), callback2);

    // Query the dispatcher for a list of the listeners for the test event.
    EventListenerList listeners = dispatcher.GetListeners(EventType("test_event"));

    // Make sure there's 1 and only 1 listener registered.
    EXPECT_EQ(2, listeners.size());

    // Make sure the items returned back are the same as the ones put in.
    EXPECT_EQ(callback1, listeners.front());
    listeners.pop_front();
    EXPECT_EQ(callback2, listeners.front());
}
