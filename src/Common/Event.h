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

#ifndef SRC_COMMON_EVENT_H_
#define SRC_COMMON_EVENT_H_

#include <functional>
#include <memory>

#include "Common/ByteBuffer.h"
#include "Common/HashString.h"

namespace common {
    
// Use a HashString as the basis for EventType's.
typedef HashString EventType;

class IEvent;
typedef std::shared_ptr<IEvent> IEventPtr;

class IEvent {
public:
    virtual const EventType& event_type() const = 0;
    virtual uint64_t timestamp() const = 0;
  
    virtual void onSerialize(ByteBuffer& out) const = 0;
};

class BaseEvent : public IEvent {
public:
    BaseEvent(uint64_t subject = 0, uint64_t timestamp = 0);

    virtual ~BaseEvent();
    
    bool hasSubject() const;
    uint64_t subject() const;

    uint64_t timestamp() const;

    void serialize(ByteBuffer& out) const;
    
protected:
    const uint64_t subject_;
    const uint64_t timestamp_;
};

class Event;

/**
 * Compares the weight of two events based on priority and timestamp.
 *
 * This helper is intended for use with a std::priority_queue<> to determine where
 * an event should be placed.
 *
 * \param lhs The "left-hand side" of the comparison (eg. x > z, x would be the lhs).
 * \param rhs The "right-hand side" of the comparison (eg. x > z, z would be the rhs).
 * \returns Returns true if left-hand side is less than the right-hand side.
 */
bool CompareEventWeightLessThan(const Event& lhs, const Event& rhs);

struct CompareEventWeightLessThanPredicate : public std::binary_function<std::shared_ptr<Event>, std::shared_ptr<Event>, bool> {
    bool operator() (const std::shared_ptr<const Event>& lhs, const std::shared_ptr<const Event>& rhs) {
        return CompareEventWeightLessThan(*lhs, *rhs);
    }
};

/**
 * Compares the weight of two events based on priority and timestamp.
 *
 * This helper is intended for use with a std::priority_queue<> to determine where
 * an event should be placed.
 *
 * \param lhs The "left-hand side" of the comparison (eg. x > z, x would be the lhs).
 * \param rhs The "right-hand side" of the comparison (eg. x > z, z would be the rhs).
 * \returns Returns true if left-hand side is greater than the right-hand side.
 */
bool CompareEventWeightGreaterThan(const Event& lhs, const Event& rhs);

struct CompareEventWeightGreaterThanPredicate : public std::binary_function<std::shared_ptr<Event>, std::shared_ptr<Event>, bool> {
    bool operator() (const std::shared_ptr<const Event>& lhs, const std::shared_ptr<const Event>& rhs) {
        return CompareEventWeightGreaterThan(*lhs, *rhs);
    }
};

class Event {
public:
    typedef std::function<void ()> EventCallback;

public:
    Event();
    explicit Event(const EventType& event_type);
    Event(const EventType& event_type, EventCallback);
    Event(const EventType& event_type, uint64_t delay_ms);
    Event(const EventType& event_type, uint64_t delay_ms, EventCallback);
    Event(const EventType& event_type, std::unique_ptr<ByteBuffer>&& subject);
    Event(const EventType& event_type, std::unique_ptr<ByteBuffer>&& subject, std::unique_ptr<ByteBuffer>&& data);

    virtual ~Event();

    /**
     * Copy constructor, used to copy one Event to another. This destroy's the from object.
     * 
     * \param from The Event to use as the source in the copy.
     */
	Event(const Event& from);

    /**
     * Assignment operator, assigns one Event's contents to another. This destroy's the from object.
     *
     * \param from The Event to use as the source in the assignment.
     */
	Event& operator=(const Event& from);

    /**
     * A no-throw swap used for swapping the contents of two Events.
     *
     * @param from The source Event to swap contents with.
     */
	void Swap(Event& from);

    /**
     * Returns the type of the event.
     *
     * \returns The type of the event.
     */
    const EventType& event_type() const;
    
    /**
     * Checks to see if a subject has been set or not.
     *
     * \returns True if a subject has been set, false if not.
     */
    bool HasSubject() const;

    /**
     * Returns a throw-away copy of the subject.
     *
     * ByteBuffer's read functionality is non-const so a throw away copy is instead 
     * created and returned. Although this copy can be modified it does not affect
     * the source subject in the event itself.
     *
     * \returns A throw-away copy of the subject.
     */
    std::unique_ptr<ByteBuffer> subject() const;
    
    /**
     * Checks to see if any data has been set for the event or not.
     *
     * \returns True if data has been set, false if not.
     */
    bool HasData() const;

    /**
     * Returns a throw-away copy of the event data.
     *
     * ByteBuffer's read functionality is non-const so a throw away copy is instead 
     * created and returned. Although this copy can be modified it does not affect
     * the source data in the event itself.
     *
     * \returns A throw-away copy of the event data.
     */
    std::unique_ptr<ByteBuffer> data() const;
    
    /**
     * Checks to see if any response data has been set for the event or not.
     *
     * \returns True if response data has been set, false if not.
     */
    bool HasResponse() const;

    /**
     * Returns a throw-away copy of the event's response data.
     *
     * ByteBuffer's read functionality is non-const so a throw away copy is instead 
     * created and returned. Although this copy can be modified it does not affect
     * the source response data in the event itself.
     *
     * \returns A throw-away copy of the event's response data.
     */
    std::unique_ptr<ByteBuffer> response() const;

    /**
     * Sets response data for the event.
     *
     * \param response This is the response data. Implemented as a unique pointer it "consumes" anything passed into it
     *                 meaning that it takes control of the object's lifetime including when to call delete.
     */
    void response(std::unique_ptr<ByteBuffer> response);

    /**
     * Sets a timestamp for the event to indicate when it was triggered. 
     * 
     * Timestamps are generally set via the EventDispatcher when it receives an event,
     * however, if an event specifies a timestamp itself it will be honored and placed in the queue
     * accordingly.
     *
     * \param timestamp The value to set the timestamp to.
     */
    void timestamp(uint64_t timestamp);

    /**
     * Returns a timestamp indicating when the event was triggered.
     *
     * \returns A timestamp indicating when the event was triggered.
     */
    uint64_t timestamp() const;    

    /**
     * Sets a priority for the event to use when concidering where placing it in a queue. 
     * 
     * Not all events are equal, some are simply more important than others and therefore
     * need to be processed as soon as possible. Placing a higher priority value on these
     * items will ensure they are placed higher in the queue.
     *
     * \param priority The value to set the priority to.
     */
    void priority(uint8_t timestamp);

    /**
     * Returns the current priority of the event.
     *
     * \returns Returns the current priority of the event.
     */
    uint8_t priority() const;

    /**
     * Trigger's the event's callback if set.
     */
    void triggerCallback() const;

    /**
     * Chain's an event to be called after the current one finishes executing.
     *
     * Easy daisy-chaining of events supported such as:
     * \code 
     *      my_event->next(some_event)->next(some_second_event)->next(some_third_event);
     *
     * \param next The next event in the chain.
     * \returns The next event in the chain for easy "daisy-chaining" of events.
     */
    std::shared_ptr<Event> next(std::shared_ptr<Event> next);

    /**
     * Returns the next event in the chain.
     *
     * \param The next event in the chain.
     */
    std::shared_ptr<Event> next();

    /**
     * Returns the amount of time in milliseconds the event is to be delayed.
     *
     * \param The amount of time in milliseconds the event is to be delayed.
     */
    uint64_t delay_ms() const;
    
private:
    EventType event_type_;
    std::unique_ptr<ByteBuffer> subject_;
    std::unique_ptr<ByteBuffer> data_;
    std::unique_ptr<ByteBuffer> response_;
    std::unique_ptr<EventCallback> callback_;
    std::shared_ptr<Event> next_;
    uint64_t delay_ms_;
    uint64_t timestamp_;
    uint8_t priority_;
};

}  // namespace common

#endif  // SRC_COMMON_EVENT_H_
