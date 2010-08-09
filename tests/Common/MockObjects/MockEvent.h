/*
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

#ifndef SRC_COMMON_MOCKOBJECTS_MOCKEVENT_H_
#define SRC_COMMON_MOCKOBJECTS_MOCKEVENT_H_

#include "Common/Event.h"

class MockEvent : public ::common::BaseEvent {
public:
    explicit MockEvent(uint64_t subject_id = 0, uint64_t timestamp = 0, uint64_t delay_ms = 0) 
        : BaseEvent(subject_id, timestamp, delay_ms)
    , some_event_val_(0) {}

    explicit MockEvent(uint64_t subject_id, uint64_t timestamp, uint64_t delay_ms, ::common::EventCallback callback) 
        : BaseEvent(subject_id, timestamp, delay_ms, callback)
    , some_event_val_(0) {}

    explicit MockEvent(::common::ByteBuffer& in) {
        deserialize(in);
    }
    
    ~MockEvent() {}

    const ::common::EventType& event_type() const { return event_type_; }

    int some_event_val() const { return some_event_val_; }
    void some_event_val(int some_event_val) { some_event_val_ = some_event_val; }

private:
    void onSerialize(::common::ByteBuffer& out) const {
        out << some_event_val_;
    }

    void onDeserialize(::common::ByteBuffer& in) {
        some_event_val_ = in.Read<int>();
    }

    bool onConsume(bool handled) const {
        return true;
    }

    static const ::common::EventType event_type_;
    int some_event_val_;
};

#endif  // SRC_COMMON_MOCKOBJECTS_MOCKEVENT_H_
