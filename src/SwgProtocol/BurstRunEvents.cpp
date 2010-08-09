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

#include "SwgProtocol/BurstRunEvents.h"

namespace swg_protocol {

const ::common::EventType BurstRunEndEvent::event_type_ = ::common::EventType("BurstRunEndEvent");

BurstRunEndEvent::BurstRunEndEvent(::common::ByteBuffer& in) {
    deserialize(in);
}

BurstRunEndEvent::BurstRunEndEvent(uint64_t subject_id, uint64_t timestamp, uint64_t delay_ms) :
::common::BaseEvent(subject_id, timestamp, delay_ms) {}

BurstRunEndEvent::BurstRunEndEvent(uint64_t subject_id, uint64_t timestamp, uint64_t delay_ms, ::common::EventCallback callback) 
    : ::common::BaseEvent(subject_id, timestamp, delay_ms, callback) {}
    
BurstRunEndEvent::~BurstRunEndEvent() {}

const ::common::EventType& BurstRunEndEvent::event_type() const { 
    return event_type_; 
}

void BurstRunEndEvent::onSerialize(::common::ByteBuffer& out) const {}
void BurstRunEndEvent::onDeserialize(::common::ByteBuffer& in) {}

bool BurstRunEndEvent::onConsume(bool handled) const {
    return true;
}

}  // namespace swg_protocol
