/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "SwgProtocol/BurstRunEvents.h"

using ::common::BaseEvent;
using ::common::ByteBuffer;
using ::common::EventCallback;
using ::common::EventType;

namespace swg_protocol {

const EventType BurstRunEndEvent::event_type_         = EventType("BurstRunEndEvent");
const EventType BurstRunCooldownEndEvent::event_type_ = EventType("BurstRunCooldownEndEvent");

BurstRunEndEvent::BurstRunEndEvent(ByteBuffer& in)
    : BaseEvent() {
    deserialize(in);
}

BurstRunEndEvent::BurstRunEndEvent(uint64_t subject_id, uint64_t delay_ms)
    : BaseEvent(subject_id, delay_ms) {}

BurstRunEndEvent::BurstRunEndEvent(uint64_t subject_id, uint64_t delay_ms, EventCallback callback)
    : BaseEvent(subject_id, delay_ms, callback) {}

BurstRunEndEvent::~BurstRunEndEvent() {}

const EventType& BurstRunEndEvent::event_type() const {
    return event_type_;
}

void BurstRunEndEvent::onSerialize(ByteBuffer& out) const {}
void BurstRunEndEvent::onDeserialize(ByteBuffer& in) {}

bool BurstRunEndEvent::onConsume(bool handled) const {
    return true;
}

BurstRunCooldownEndEvent::BurstRunCooldownEndEvent(ByteBuffer& in) {
    deserialize(in);
}

BurstRunCooldownEndEvent::BurstRunCooldownEndEvent(uint64_t subject_id, uint64_t delay_ms)
    : BaseEvent(subject_id, delay_ms) {}

BurstRunCooldownEndEvent::BurstRunCooldownEndEvent(uint64_t subject_id, uint64_t delay_ms, EventCallback callback)
    : BaseEvent(subject_id, delay_ms, callback) {}

BurstRunCooldownEndEvent::~BurstRunCooldownEndEvent() {}

const EventType& BurstRunCooldownEndEvent::event_type() const {
    return event_type_;
}

void BurstRunCooldownEndEvent::onSerialize(ByteBuffer& out) const {}
void BurstRunCooldownEndEvent::onDeserialize(ByteBuffer& in) {}

bool BurstRunCooldownEndEvent::onConsume(bool handled) const {
    return true;
}

}  // namespace swg_protocol
