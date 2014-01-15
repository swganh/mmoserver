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

class CommandQueueRemove : public ObjControllerMessage
{
public:
    explicit CommandQueueRemove(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
    {}

    CommandQueueRemove(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x00000117;
    }

    uint32_t action_counter;
    float timer;
    uint32_t error;
    uint32_t action;

    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(action_counter);
        buffer.write(timer);
        buffer.write(error);
        buffer.write(action);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        action_counter = buffer.read<uint32_t>();
        timer = buffer.read<float>();
        error = buffer.read<uint32_t>();
        action = buffer.read<uint32_t>();
    }
};

}
}
}  // namespace swganh::messages::controllers
