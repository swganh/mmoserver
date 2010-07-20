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

Event::Event(const EventType& event_type)
: event_type_(event_type)
, subject_(nullptr)
, data_(nullptr)
, response_(nullptr)
, timestamp_(0)
, priority_(0) {}

Event::Event(const EventType& event_type, std::unique_ptr<ByteBuffer>&& subject)
: event_type_(event_type)
, subject_(std::forward<std::unique_ptr<ByteBuffer>>(subject))
, data_(nullptr)
, response_(nullptr)
, timestamp_(0)
, priority_(0) {}

Event::Event(const EventType& event_type, std::unique_ptr<ByteBuffer>&& subject, std::unique_ptr<ByteBuffer>&& data)
: event_type_(event_type)
, subject_(std::forward<std::unique_ptr<ByteBuffer>>(subject))
, data_(std::forward<std::unique_ptr<ByteBuffer>>(data))
, response_(nullptr)
, timestamp_(0)
, priority_(0) {}

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

// Helper function implementations

bool CompareEventWeightLessThan(const Event& lhs, const Event& rhs) {
    return ((lhs.timestamp() + lhs.priority()) < (rhs.timestamp() + rhs.priority()));
}

bool CompareEventWeightGreaterThan(const Event& lhs, const Event& rhs) {
    return ((lhs.timestamp() + lhs.priority()) > (rhs.timestamp() + rhs.priority()));
}

}  // namespace common
