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

#ifndef LIBANH_EVENT_DISPATCHER_BASIC_EVENT_H_
#define LIBANH_EVENT_DISPATCHER_BASIC_EVENT_H_

#include <cstdint>

#include "anh/hash_string.h"

namespace anh {
namespace event_dispatcher {
    
typedef anh::HashString EventType;

class IEvent {
public:
    virtual ~IEvent() {}

    virtual const EventType& type() = 0;
    
    virtual uint32_t priority() const = 0;
    virtual void priority(uint32_t priority) = 0;

    virtual uint64_t timestamp() const = 0;
    virtual void timestamp(uint64_t timestamp) = 0;
};

template<typename T>
class BasicEvent : public T, public IEvent {
public:
    BasicEvent()
        : type_(T::type())
        , timestamp_(0)
        , priority_(T::priority()) {}
    
    explicit BasicEvent(EventType type)
        : type_(std::move(type))
        , timestamp_(0)
        , priority_(0) {}
    
    BasicEvent(EventType type, uint32_t priority)
        : type_(std::move(type))
        , timestamp_(0)
        , priority_(priority) {}

    ~BasicEvent() {}

    const EventType& type() { return type_; }

    uint32_t priority() const {
        return priority_;
    }

    void priority(uint32_t priority) {
        priority_ = priority;
    }
    
    uint64_t timestamp() const {
        return timestamp_;
    }
    
    void timestamp(uint64_t timestamp)  {
        timestamp_ = timestamp;
    }

private:
    EventType type_;
    uint64_t timestamp_;
    uint32_t priority_;
};

struct NullEventData {};

typedef BasicEvent<NullEventData> SimpleEvent;

}  // namespace event_dispatcher
}  // namespace anh

#endif  // LIBANH_EVENT_DISPATCHER_BASIC_EVENT_H_
