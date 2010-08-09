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
#include "Common/CommonDeclspec.h"

namespace common {
    
// Use a HashString as the basis for EventType's.
typedef HashString EventType;
typedef uint64_t EventSubject;
typedef uint32_t EventPriority;
typedef std::function<void ()> EventCallback;

class IEvent;
typedef std::shared_ptr<IEvent> IEventPtr;

class COMMON_API IEvent {
public:  
    virtual const EventType& event_type() const = 0;
    virtual EventSubject subject() const = 0;
    virtual EventPriority priority() const = 0;
    virtual uint64_t timestamp() const = 0;
    virtual uint64_t delay_ms() const = 0;

    virtual void next(IEventPtr) = 0;
    virtual IEventPtr next() const = 0;

    virtual void consume(bool handled) const = 0;
    virtual void serialize(ByteBuffer& out) const = 0;
    virtual void deserialize(ByteBuffer& in) = 0;
};

class COMMON_API BaseEvent : public IEvent {
public:
    BaseEvent(EventSubject subject = 0, uint64_t timestamp = 0, uint64_t delay_ms = 0);
    BaseEvent(EventSubject subject, uint64_t timestamp, uint64_t delay_ms, EventCallback callback);

    virtual ~BaseEvent();

    bool hasSubject() const;
    EventSubject subject() const;
    
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

    IEventPtr next_;

    std::unique_ptr<EventCallback> callback_;
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
COMMON_API bool CompareEventWeightLessThan(const IEvent& lhs, const IEvent& rhs);

struct COMMON_API CompareEventWeightLessThanPredicate : public std::binary_function<std::shared_ptr<IEvent>, std::shared_ptr<IEvent>, bool> {
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
COMMON_API bool CompareEventWeightGreaterThan(const IEvent& lhs, const IEvent& rhs);

struct COMMON_API CompareEventWeightGreaterThanPredicate : public std::binary_function<std::shared_ptr<IEvent>, std::shared_ptr<IEvent>, bool> {
    bool operator() (const std::shared_ptr<const IEvent>& lhs, const std::shared_ptr<const IEvent>& rhs) {
        return CompareEventWeightGreaterThan(*lhs, *rhs);
    }
};

}  // namespace common

#endif  // SRC_COMMON_EVENT_H_
