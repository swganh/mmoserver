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
#include "MockObjects/MockEvent.h"

using ::common::ByteBuffer;
using ::common::IEventPtr;
using ::common::EventType;
using ::common::EventSubject;

namespace {

/*! All events should have a type and a way of returning that type to a caller.
 */
TEST(EventTests, EventHasAnEventType) {
    MockEvent test_event;
    EXPECT_EQ(EventType("mock_event"), test_event.event_type());
}

/*! Code that consumes an event needs a method of determining if a subject has
 * been set yet or not.
 */
TEST(EventTests, EventHasNoSubjectByDefault) {
    MockEvent test_event;
    EXPECT_EQ(0, test_event.subject());
}

/*! An event's subject should be specified at the point of event creation.
 */
TEST(EventTests, CanSetAndRetrieveSubjectForEvent) {
    // Create some subject id number to use for testing and create the MockEvent instance with it.
    EventSubject some_subject_id = 2234;
    MockEvent test_event(some_subject_id);

    // Make sure that the event now has a subject and that it returns the correct value.
    EXPECT_EQ(some_subject_id, test_event.subject());
}

/*! Every event needs to pass through the event dispatcher in order to be 
 * consumed by a listener. The event dispatcher recognizes that not all events
 * are created equal and therefore uses an event's priority as a weight when
 * determining where to place it in the processing queue. By default event's 
 * should have no additional weight.
 */
TEST(EventTests, EventsHaveDefaultPriorityOfZero) {
    MockEvent test_event;

    // Make sure that by default the priority is zero.
    EXPECT_EQ(0, test_event.priority());
}

/*! Setting a priority for an event will most likely not be the responsibility
 * of the source that's generating it, therefore some facility must exist for
 * setting a priority for an event.
 */
TEST(EventTests, CanSetPriorityForEvent) {
    MockEvent test_event;

    // Set a priority value and make sure what we get out is what was put in.
    test_event.priority(27);
    EXPECT_EQ(27, test_event.priority());
}

TEST(EventTests, CanSetTimestampForEvent) {
    uint64_t timestamp = 100;

    // Set a timestamp for the event and make sure the value we get out is what was put in.
    MockEvent test_event;
    test_event.timestamp(timestamp);

    EXPECT_EQ(timestamp, test_event.timestamp());
}

TEST(EventTests, ComparingEventsConsidersTimestamp) {
    // Create two events with differing timestamps.
    EventSubject some_subject_id = 2234;

    MockEvent test_event1(some_subject_id, 100);
    MockEvent test_event2(some_subject_id, 200);

    EXPECT_EQ(true, CompareEventWeightLessThan(test_event1, test_event2));
    EXPECT_EQ(true, CompareEventWeightGreaterThan(test_event2, test_event1));
}

TEST(EventTests, ComparingEventsConsidersPriority) {
    // Create two events with differing priorities.
    MockEvent test_event1;
    test_event1.priority(1);

    MockEvent test_event2;
    test_event2.priority(2);

    EXPECT_EQ(true, CompareEventWeightLessThan(test_event1, test_event2));
    EXPECT_EQ(true, CompareEventWeightGreaterThan(test_event2, test_event1));
}


TEST(EventTests, CanSetCallbackForEvent) {
    std::shared_ptr<int> someval = std::make_shared<int>(0);
    
    // Create an event with a callback that updates our local value.
    MockEvent test_event1(0, 0, 0, [=] {
        *someval = 1;
    });

    test_event1.consume(true);
    
    // Make sure the value was updated.
    EXPECT_EQ(1, *someval);
}

TEST(EventTests, CanDeserializeEventFromBuffer) {
    // Create the buffer with the correct contents.
    ByteBuffer buffer;
    buffer.Write<uint32_t>(0xC3CEA198); // This is the swgcrc of "mock_event"
    buffer.Write<int>(27); // Some random int value.

    MockEvent test_event(buffer);

    EXPECT_EQ(27, test_event.some_event_val());
}

TEST(EventTests, CanSerializeEventToBuffer) {
    // Create the buffer with the correct contents.
    ByteBuffer buffer;

    MockEvent test_event;
    test_event.some_event_val(27);

    // Read the event into the buffer.
    test_event.serialize(buffer);

    EXPECT_EQ(0xC3CEA198, buffer.Read<uint32_t>());
    EXPECT_EQ(27, buffer.Read<int>());
}

}
