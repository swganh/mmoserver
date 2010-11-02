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

#ifndef SRC_SWGPROTOCOL_OBJECTCONTROLLEREVENTS_H_
#define SRC_SWGPROTOCOL_OBJECTCONTROLLEREVENTS_H_

#include "Common/byte_buffer.h"
#include "Common/Event.h"

namespace swg_protocol {

namespace object_controller {

class PreCommandEvent : public common::BaseEvent {
public:
    static const ::common::EventType type;

public:
    explicit PreCommandEvent(::common::ByteBuffer& in);
    explicit PreCommandEvent(uint64_t subject_id = 0, uint64_t delay_ms = 0);
    PreCommandEvent(uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback);

    ~PreCommandEvent();

    const ::common::EventType& event_type() const;

    uint64_t target_id() const;
    void target_id(uint64_t target_id);

    uint32_t command_crc() const;
    void command_crc(uint32_t command_crc);

private:
    void onSerialize(::common::ByteBuffer& out) const;
    void onDeserialize(::common::ByteBuffer& in);

    bool onConsume(bool handled) const;

    uint64_t target_id_;
    uint32_t command_crc_;
};


class PreCommandExecuteEvent : public ::common::BaseEvent {
public:
    static const ::common::EventType type;

public:
    explicit PreCommandExecuteEvent(::common::ByteBuffer& in);
    explicit PreCommandExecuteEvent(uint64_t subject_id = 0, uint64_t delay_ms = 0);
    PreCommandExecuteEvent(uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback);

    ~PreCommandExecuteEvent();

    const ::common::EventType& event_type() const;

    uint64_t target_id() const;
    void target_id(uint64_t target_id);

    uint32_t command_crc() const;
    void command_crc(uint32_t command_crc);

private:
    void onSerialize(::common::ByteBuffer& out) const;
    void onDeserialize(::common::ByteBuffer& in);

    bool onConsume(bool handled) const;

    uint64_t target_id_;
    uint32_t command_crc_;
};

class PostCommandEvent : public ::common::BaseEvent {
public:
    static const ::common::EventType type;

public:
    explicit PostCommandEvent(::common::ByteBuffer& in);
    explicit PostCommandEvent(uint64_t subject_id = 0, uint64_t delay_ms = 0);
    PostCommandEvent(uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback);

    ~PostCommandEvent();

    const ::common::EventType& event_type() const;

    bool command_processed() const;
    void command_processed(bool command_processed);

private:
    void onSerialize(::common::ByteBuffer& out) const;
    void onDeserialize(::common::ByteBuffer& in);

    bool onConsume(bool handled) const;

    bool command_processed_;
};

}  // namespace object_controller

}  // namespace swg_protocol

#endif  // SRC_SWGPROTOCOL_BURSTRUNEVENTS_H_