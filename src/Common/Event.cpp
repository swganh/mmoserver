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

#include "Common/Event.h"

namespace common {

BaseEvent::BaseEvent(uint64_t subject, uint64_t timestamp) 
: subject_(subject)
, timestamp_(timestamp) {}

BaseEvent::~BaseEvent() {}

bool BaseEvent::hasSubject() const {
    return (subject_ != 0) ? true : false;
}

uint64_t BaseEvent::subject() const { 
    return subject_;
}

uint64_t BaseEvent::timestamp() const { 
    return timestamp_;
}

void BaseEvent::serialize(ByteBuffer& out) const {
    out.Write<uint32_t>(event_type().ident());

    onSerialize(out);
}
    
Event::Event()
: event_type_(EventType("null"))
, subject_(nullptr)
, data_(nullptr)
, response_(nullptr)
, callback_(nullptr)
, delay_ms_(0)
, timestamp_(0)
, priority_(0) {}

Event::Event(const EventType& event_type)
: event_type_(event_type)
, subject_(nullptr)
, data_(nullptr)
, response_(nullptr)
, callback_(nullptr)
, delay_ms_(0)
, timestamp_(0)
, priority_(0) {}

Event::Event(const EventType& event_type, EventCallback callback)
: event_type_(event_type)
, subject_(nullptr)
, data_(nullptr)
, response_(nullptr)
, callback_(new EventCallback(callback))
, delay_ms_(0)
, timestamp_(0)
, priority_(0) {}


Event::Event(const EventType& event_type, uint64_t delay_ms)
: event_type_(event_type)
, subject_(nullptr)
, data_(nullptr)
, response_(nullptr)
, callback_(nullptr)
, delay_ms_(delay_ms)
, timestamp_(0)
, priority_(0) {}

Event::Event(const EventType& event_type, uint64_t delay_ms, EventCallback callback)
: event_type_(event_type)
, subject_(nullptr)
, data_(nullptr)
, response_(nullptr)
, callback_(new EventCallback(callback))
, delay_ms_(delay_ms)
, timestamp_(0)
, priority_(0) {}

Event::Event(const EventType& event_type, std::unique_ptr<ByteBuffer>&& subject)
: event_type_(event_type)
, subject_(std::forward<std::unique_ptr<ByteBuffer>>(subject))
, data_(nullptr)
, response_(nullptr)
, callback_(nullptr)
, delay_ms_(0)
, timestamp_(0)
, priority_(0) {}

Event::Event(const EventType& event_type, std::unique_ptr<ByteBuffer>&& subject, std::unique_ptr<ByteBuffer>&& data)
: event_type_(event_type)
, subject_(std::forward<std::unique_ptr<ByteBuffer>>(subject))
, data_(std::forward<std::unique_ptr<ByteBuffer>>(data))
, response_(nullptr)
, callback_(nullptr)
, delay_ms_(0)
, timestamp_(0)
, priority_(0) {}

Event::~Event() {}

Event::Event(const Event& from)
: event_type_(from.event_type_)
, subject_(nullptr)
, data_(nullptr)
, response_(nullptr)
, callback_(nullptr)
, delay_ms_(from.delay_ms_)
, timestamp_(from.timestamp_)
, priority_(from.priority_) {}

Event& Event::operator=(const Event& from) {
	Event tmp(from);
	Swap(tmp);

	return *this;
}

void Event::Swap(Event& from) {
	std::swap(event_type_, from.event_type_);
	std::swap(subject_, from.subject_);
	std::swap(data_, from.data_);
	std::swap(response_, from.response_);
	std::swap(timestamp_, from.timestamp_);
	std::swap(priority_, from.priority_);
}

const EventType& Event::event_type() const {
    return event_type_;
}

bool Event::HasSubject() const {
    return (subject_ != nullptr);
}

std::unique_ptr<ByteBuffer> Event::subject() const {
    std::unique_ptr<ByteBuffer> subject(new ByteBuffer(*subject_));
    return subject;
}

bool Event::HasData() const {
    return (data_ != nullptr);
}

std::unique_ptr<ByteBuffer> Event::data() const {
    std::unique_ptr<ByteBuffer> data(new ByteBuffer(*data_));
    return data;
}

bool Event::HasResponse() const {
    return (response_ != nullptr);
}

std::unique_ptr<ByteBuffer> Event::response() const {
    std::unique_ptr<ByteBuffer> response(new ByteBuffer(*response_));
    return response;
}

void Event::response(std::unique_ptr<ByteBuffer> response) {
    response_ = std::move(response);
}

void Event::timestamp(uint64_t timestamp) {
    timestamp_ = timestamp;
}

uint64_t Event::timestamp() const {
    return timestamp_;
}

void Event::priority(uint8_t priority) {
    priority_ = priority;
}

uint8_t Event::priority() const {
    return priority_;
}

void Event::triggerCallback() const {
    if (callback_) {
        (*callback_)();
    }
}

std::shared_ptr<Event> Event::next(std::shared_ptr<Event> next) {
    next_ = next;
    return next_;
}

std::shared_ptr<Event> Event::next() {
    return next_;
}

uint64_t Event::delay_ms() const {
    return delay_ms_;
}

// Helper function implementations

bool CompareEventWeightLessThan(const Event& lhs, const Event& rhs) {
    return ((lhs.timestamp() + lhs.priority()) < (rhs.timestamp() + rhs.priority()));
}

bool CompareEventWeightLessThan(const std::shared_ptr<Event>& lhs, const std::shared_ptr<Event>& rhs) {
    return CompareEventWeightLessThan(*lhs, *rhs);
}

bool CompareEventWeightGreaterThan(const Event& lhs, const Event& rhs) {
    return ((lhs.timestamp() + lhs.priority()) > (rhs.timestamp() + rhs.priority()));
}

bool CompareEventWeightGreaterThan(const std::shared_ptr<Event>& lhs, const std::shared_ptr<Event>& rhs) {
    return CompareEventWeightGreaterThan(*lhs, *rhs);
}

}  // namespace common
