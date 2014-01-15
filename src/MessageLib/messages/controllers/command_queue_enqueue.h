// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"

#include "MessageLib/messages/obj_controller_message.h"

namespace swganh
{
namespace messages
{
namespace controllers
{

class CommandQueueEnqueue : public ObjControllerMessage
{
public:
    explicit CommandQueueEnqueue(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
    {}

    CommandQueueEnqueue(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    CommandQueueEnqueue(const CommandQueueEnqueue& other)
        : ObjControllerMessage(other)
    {
        action_counter = other.action_counter;
        command_crc = other.command_crc;
        target_id = other.target_id;
        command_options = other.command_options;
    }

    static uint32_t message_type()
    {
        return 0x00000116;
    }

    uint32_t action_counter;
    uint32_t command_crc;
    uint64_t target_id;
    std::wstring command_options;

    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(action_counter);
        buffer.write(command_crc);
        buffer.write(target_id);
        buffer.write(command_options);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        action_counter = buffer.read<uint32_t>();
        command_crc = buffer.read<uint32_t>();
        target_id = buffer.read<uint64_t>();
        command_options = buffer.read<std::wstring>();
    }
};

}
}
}  // namespace swganh::messages::controllers
