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

TEST(EventDispatcherTest, HasNoListenersByDefault) {
    EventDispatcher dispatcher;

    EXPECT_FALSE(dispatcher.hasListeners("some_event_type"));
}


TEST(EventDispatcherTest, HasNoRegisteredEventTypesByDefault) {
    EventDispatcher dispatcher;

    EXPECT_FALSE(dispatcher.hasRegisteredEventType("some_event_type"));
}


TEST(EventDispatcherTest, CanRegisterEventType) {
    EventDispatcher dispatcher;

    dispatcher.registerEventType("some_event_type");

    EXPECT_TRUE(dispatcher.hasRegisteredEventType("some_event_type"));
}


TEST(EventDispatcherTest, CannotSubscribeToUnregisteredEvent) {
    EventDispatcher dispatcher;

    EXPECT_FALSE(dispatcher.subscribe("some_event_type", [] (shared_ptr<BaseEvent> incoming_event) {}));
}


//TEST(EventDispatcherTest, HasListenersAfterOneSubscribes) {
//    EventDispatcher dispatcher;
//
//    dispatcher.subscribe("some_event_type", [] (shared_ptr<BaseEvent> incoming_event) {});
//
//    EXPECT_TRUE(dispatcher.hasListeners("some_event_type"));
//}
//