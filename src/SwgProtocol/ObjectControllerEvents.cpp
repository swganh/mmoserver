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

#include "SwgProtocol/ObjectControllerEvents.h"

namespace swg_protocol {

namespace object_controller {

const ::common::EventType PreCommandEvent::type         = ::common::EventType("PreCommandEvent");
const ::common::EventType PreCommandExecuteEvent::type  = ::common::EventType("PreCommandExecuteEvent");
const ::common::EventType PostCommandEvent::type        = ::common::EventType("PostCommandEvent");

PreCommandEvent::PreCommandEvent(::common::ByteBuffer& in) {
    deserialize(in);
}

PreCommandEvent::PreCommandEvent(uint64_t subject_id, uint64_t timestamp, uint64_t delay_ms) :
::common::BaseEvent(subject_id, timestamp, delay_ms) {}

PreCommandEvent::PreCommandEvent(uint64_t subject_id, uint64_t timestamp, uint64_t delay_ms, ::common::EventCallback callback) 
    : ::common::BaseEvent(subject_id, timestamp, delay_ms, callback) {}
    
PreCommandEvent::~PreCommandEvent() {}

const ::common::EventType& PreCommandEvent::event_type() const { 
    return type; 
}

void PreCommandEvent::onSerialize(::common::ByteBuffer& out) const {
    out << target_id_;
    out << command_crc_;
}

void PreCommandEvent::onDeserialize(::common::ByteBuffer& in) {
    target_id_ = in.Read<uint64_t>();
    command_crc_ = in.Read<uint32_t>();
}

bool PreCommandEvent::onConsume(bool handled) const {
    return true;
}

uint64_t PreCommandEvent::target_id() const {
    return target_id_;
}

void PreCommandEvent::target_id(uint64_t target_id) {
    target_id_ = target_id;
}

uint32_t PreCommandEvent::command_crc() const {
    return command_crc_;
}

void PreCommandEvent::command_crc(uint32_t command_crc) {
    command_crc_ = command_crc;
}




PreCommandExecuteEvent::PreCommandExecuteEvent(::common::ByteBuffer& in) {
    deserialize(in);
}

PreCommandExecuteEvent::PreCommandExecuteEvent(uint64_t subject_id, uint64_t timestamp, uint64_t delay_ms) :
::common::BaseEvent(subject_id, timestamp, delay_ms) {}

PreCommandExecuteEvent::PreCommandExecuteEvent(uint64_t subject_id, uint64_t timestamp, uint64_t delay_ms, ::common::EventCallback callback) 
    : ::common::BaseEvent(subject_id, timestamp, delay_ms, callback) {}
    
PreCommandExecuteEvent::~PreCommandExecuteEvent() {}

const ::common::EventType& PreCommandExecuteEvent::event_type() const { 
    return type; 
}

void PreCommandExecuteEvent::onSerialize(::common::ByteBuffer& out) const {
    out << target_id_;
    out << command_crc_;
}

void PreCommandExecuteEvent::onDeserialize(::common::ByteBuffer& in) {
    target_id_ = in.Read<uint64_t>();
    command_crc_ = in.Read<uint32_t>();
}

bool PreCommandExecuteEvent::onConsume(bool handled) const {
    return true;
}

uint64_t PreCommandExecuteEvent::target_id() const {
    return target_id_;
}

void PreCommandExecuteEvent::target_id(uint64_t target_id) {
    target_id_ = target_id;
}

uint32_t PreCommandExecuteEvent::command_crc() const {
    return command_crc_;
}

void PreCommandExecuteEvent::command_crc(uint32_t command_crc) {
    command_crc_ = command_crc;
}



PostCommandEvent::PostCommandEvent(::common::ByteBuffer& in) {
    deserialize(in);
}

PostCommandEvent::PostCommandEvent(uint64_t subject_id, uint64_t timestamp, uint64_t delay_ms) :
::common::BaseEvent(subject_id, timestamp, delay_ms) {}

PostCommandEvent::PostCommandEvent(uint64_t subject_id, uint64_t timestamp, uint64_t delay_ms, ::common::EventCallback callback) 
    : ::common::BaseEvent(subject_id, timestamp, delay_ms, callback) {}
    
PostCommandEvent::~PostCommandEvent() {}

const ::common::EventType& PostCommandEvent::event_type() const { 
    return type; 
}

void PostCommandEvent::onSerialize(::common::ByteBuffer& out) const {}
void PostCommandEvent::onDeserialize(::common::ByteBuffer& in) {}

bool PostCommandEvent::onConsume(bool handled) const {
    return true;
}

}  // namespace object_controller

}  // namespace swg_protocol
