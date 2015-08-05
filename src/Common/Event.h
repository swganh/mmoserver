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

#include "Common/byte_buffer.h"
#include "Common/HashString.h"

/*! \brief Common is a catch-all library containing primarily base classes and
 * classes used for maintaining application lifetimes.
 */
namespace common {

// Use a HashString as the basis for EventType's.
typedef HashString EventType;
typedef uint64_t EventSubject;
typedef uint32_t EventPriority;
typedef std::function<void ()> EventCallback;

class IEvent;
typedef std::shared_ptr<IEvent> IEventPtr;

/*! \brief Events are messages that are passed from a source location and consumed by
 * one or more listeners. The IEvent defines a common interface for all events.
 */
class IEvent {
public:
    /*! \returns The type of this event.
     * \see EventType
     */
    virtual const EventType& event_type() const = 0;

    /*! \returns The identifier for the subject of the event.
     * \see EventSubject
     */
    virtual EventSubject subject() const = 0;

    /*! \returns The priority of the event.
     * \see EventPriority
     */
    virtual EventPriority priority() const = 0;

    /// \returns The timestamp indicating when the event was triggered.
    virtual uint64_t timestamp() const = 0;

    /*! Manually sets a timestamp for the event.
     *
     * \param timestamp The new timestamp for the event.
     */
    virtual void timestamp(uint64_t timestamp) = 0;

    /// \returns The time in milliseconds that this event should be delayed.
    virtual uint64_t delay_ms() const = 0;

    /// \returns The optional next event in a chain of events.
    virtual IEventPtr next() const = 0;

    /*! This is called after an event has been passed around to all listeners.
     *
     * \param handled Indicates whether or not the event was successfully handled by listeners.
     */
    virtual void consume(bool handled) const = 0;

    /*! Serializes an event, usually for transfer over the network.
     *
     * \param out The ByteBuffer instance to stream the event data to.
     */
    virtual void serialize(ByteBuffer& out) const = 0;

    /*! Deserializes an event, usually from a network message.
     *
     * \param in The buffer to deserialize the message from.
     */
    virtual void deserialize(ByteBuffer& in) = 0;
};


/*! \brief BaseEvent is an implementation of the IEvent interface that provides
 * concrete instances with the ability to specify a callback that gets invoked
 * after the listeners have had an opportunity to process it.
 */
class BaseEvent : public IEvent {
public:
    BaseEvent(EventSubject subject = 0, uint64_t delay_ms = 0);
    BaseEvent(EventSubject subject, uint64_t delay_ms, EventCallback callback);

    virtual ~BaseEvent();

    EventSubject subject() const;
    void subject(EventSubject subject);

    EventPriority priority() const;
    void priority(EventPriority priority);

    uint64_t timestamp() const;
    void timestamp(uint64_t timestamp);

    uint64_t delay_ms() const;
    void delay_ms(uint64_t delay_ms);

    IEventPtr next() const;
    void next(IEventPtr next);

    void serialize(ByteBuffer& out) const;
    void deserialize(ByteBuffer& in);

    void consume(bool handled) const;

protected:
    virtual bool onConsume(bool handled) const = 0;
    virtual void onSerialize(ByteBuffer& out) const = 0;
    virtual void onDeserialize(ByteBuffer& in) = 0;

private:
    EventSubject subject_;
    EventPriority priority_;
    uint64_t timestamp_;
    uint64_t delay_ms_;

    // Win32 complains about stl during linkage, disable the warning.
#ifdef _WIN32
#pragma warning (disable : 4251)
#endif
    IEventPtr next_;

    std::unique_ptr<EventCallback> callback_;
    // Re-enable the warning.
#ifdef _WIN32
#pragma warning (default : 4251)
#endif
};


/*! \brief This is an event that can be used to trigger off trivial events, in
 * particular events that don't have any data related to them.
 *
 * \code
 * using ::common::SimpleEvent;
 * using ::common::EventType;
 *
 * std::shared_ptr<SimpleEvent> some_event = std::make_shared<SimpleEvent>(EventType("custom_event"), 0, 0, [=] {
 *
 * ... // Some callback code to execute at the end of the event.
 *
 * });
 *
 * event_dispatcher.Notify(some_event);
 * \endcode
 */
class SimpleEvent : public BaseEvent {
public:
    explicit SimpleEvent(const common::EventType& event_type, uint64_t subject_id = 0, uint64_t delay_ms = 0);
    SimpleEvent(const common::EventType& event_type, uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback);

    ~SimpleEvent();

    const ::common::EventType& event_type() const;

private:
    void onSerialize(::common::ByteBuffer& out) const;
    void onDeserialize(::common::ByteBuffer& in);

    bool onConsume(bool handled) const;

    ::common::EventType event_type_;
};


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
bool CompareEventWeightLessThan(const IEvent& lhs, const IEvent& rhs);

struct CompareEventWeightLessThanPredicate : public std::binary_function<std::shared_ptr<IEvent>, std::shared_ptr<IEvent>, bool> {
    bool operator() (const std::shared_ptr<const IEvent>& lhs, const std::shared_ptr<const IEvent>& rhs) {
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
bool CompareEventWeightGreaterThan(const IEvent& lhs, const IEvent& rhs);

struct CompareEventWeightGreaterThanPredicate : public std::binary_function<std::shared_ptr<IEvent>, std::shared_ptr<IEvent>, bool> {
    bool operator() (const std::shared_ptr<const IEvent>& lhs, const std::shared_ptr<const IEvent>& rhs) {
        return CompareEventWeightGreaterThan(*lhs, *rhs);
    }
};

}  // namespace common

#endif  // SRC_COMMON_EVENT_H_
