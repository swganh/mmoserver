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

#include "Common/Event.h"

using ::common::ByteBuffer;
using ::common::Event;
using ::common::EventType;

TEST(EventTests, EventRequiresEventType) {
    Event my_event(EventType("my_event_type"));

    EXPECT_EQ(EventType("my_event_type"), my_event.event_type());
}

TEST(EventTests, EventHasNoSubjectByDefault) {
    Event my_event(EventType("my_event_type"));

    EXPECT_EQ(false, my_event.HasSubject());
}

TEST(EventTests, CanSetSubjectForEvent) {
    // Create a byte-buffer to serve as a container for the subject data.
    std::unique_ptr<ByteBuffer> subject(new ByteBuffer());
    subject->Write<std::string>("test_string");

    // Create the event and ask it for the subject.
    Event my_event(EventType("my_event_with_subject"), std::move(subject));
    std::unique_ptr<ByteBuffer> event_subject = my_event.subject();

    // Make sure the subject that we get out is the one that was put in.
    EXPECT_EQ("test_string", event_subject->Read<std::string>());
}

TEST(EventTests, EventHasNoDataByDefault) {
    Event my_event(EventType("my_event_type"));

    EXPECT_EQ(false, my_event.HasData());
}

TEST(EventTests, CanSetDataForEvent) {
    // Create a byte-buffer to serve as a container for the subject data.
    std::unique_ptr<ByteBuffer> data(new ByteBuffer());
    data->Write<std::string>("test_string");

    // Create the event and ask it for the subject.
    Event my_event(EventType("my_event_with_data"), nullptr, std::move(data));
    std::unique_ptr<ByteBuffer> event_data = my_event.data();

    // Make sure the subject that we get out is the one that was put in.
    EXPECT_EQ("test_string", event_data->Read<std::string>());
}

TEST(EventTests, EventHasNoResponseByDefault) {
    Event my_event(EventType("my_event_type"));

    EXPECT_EQ(false, my_event.HasResponse());
}

TEST(EventTests, CanSetResponseForEvent) {
    // Create a byte-buffer to serve as a container for the subject data.
    std::unique_ptr<ByteBuffer> response(new ByteBuffer());
    response->Write<std::string>("test_string");

    // Create the event and ask it for the subject.
    Event my_event(EventType("my_event_with_data"));
    my_event.response(std::move(response));

    std::unique_ptr<ByteBuffer> event_response = my_event.response();

    // Make sure the subject that we get out is the one that was put in.
    EXPECT_EQ("test_string", event_response->Read<std::string>());
}

TEST(EventTests, CanSetTimestampForEvent) {
    uint64_t timestamp = 100;

    // Set a timestamp for the event and make sure the value we get out is what was put in.
    Event my_event(EventType("my_event_type"));
    my_event.timestamp(timestamp);

    EXPECT_EQ(timestamp, my_event.timestamp());
}

TEST(EventTests, EventsHaveDefaultPriorityOfZero) {
    Event my_event(EventType("my_event_type"));

    // Make sure that by default the priority is zero.
    EXPECT_EQ(0, my_event.priority());
}

TEST(EventTests, CanSetPriorityForEvent) {
    Event my_event(EventType("my_event_type"));

    // Set a priority value and make sure what we get out is what was put in.
    my_event.priority(27);
    EXPECT_EQ(27, my_event.priority());
}

TEST(EventTests, ComparingEventsConsidersTimestamp) {
    Event event1(EventType("event1"));
    event1.timestamp(100);

    Event event2(EventType("event2"));
    event2.timestamp(200);

    EXPECT_EQ(true, CompareEventWeightLessThan(event1, event2));
    EXPECT_EQ(true, CompareEventWeightGreaterThan(event2, event1));
}

TEST(EventTests, ComparingEventsConsidersPriority) {
    Event event1(EventType("event1"));
    event1.priority(1);

    Event event2(EventType("event2"));
    event2.priority(2);

    EXPECT_EQ(true, CompareEventWeightLessThan(event1, event2));
    EXPECT_EQ(true, CompareEventWeightGreaterThan(event2, event1));
}

TEST(EventTests, CanSetCallbackForEvent) {
    std::shared_ptr<int> someval = std::make_shared<int>(0);
    
    // Create an event with a callback that updates our local value.
    Event event1(EventType("callback_event"), [=] {
        *someval = 1;
    });

    event1.triggerCallback();
    
    // Make sure the value was updated.
    EXPECT_EQ(1, *someval);
}



