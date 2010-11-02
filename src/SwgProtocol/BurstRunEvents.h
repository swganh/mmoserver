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

/** \file SwgProtocol/BurstRunEvents.h
 * \brief Defines the events used during the burst run command.
 */

#ifndef SRC_SWGPROTOCOL_BURSTRUNEVENTS_H_
#define SRC_SWGPROTOCOL_BURSTRUNEVENTS_H_

#include "Common/Event.h"

namespace swg_protocol {

class BurstRunEndEvent : public ::common::BaseEvent {
public:
    explicit BurstRunEndEvent(::common::ByteBuffer& in);
    explicit BurstRunEndEvent(uint64_t subject_id = 0, uint64_t delay_ms = 0);
    BurstRunEndEvent(uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback);

    ~BurstRunEndEvent();

    const ::common::EventType& event_type() const;

private:
    void onSerialize(::common::ByteBuffer& out) const;
    void onDeserialize(::common::ByteBuffer& in);

    bool onConsume(bool handled) const;

    static const ::common::EventType event_type_;
};

class BurstRunCooldownEndEvent : public ::common::BaseEvent {
public:
    explicit BurstRunCooldownEndEvent(::common::ByteBuffer& in);
    explicit BurstRunCooldownEndEvent(uint64_t subject_id = 0, uint64_t delay_ms = 0);
    BurstRunCooldownEndEvent(uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback);

    ~BurstRunCooldownEndEvent();

    const ::common::EventType& event_type() const;

private:
    void onSerialize(::common::ByteBuffer& out) const;
    void onDeserialize(::common::ByteBuffer& in);

    bool onConsume(bool handled) const;

    static const ::common::EventType event_type_;
};

}  // namespace swg_protocol

#endif  // SRC_SWGPROTOCOL_BURSTRUNEVENTS_H_
