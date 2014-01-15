// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>

#include "anh/byte_buffer.h"

#include "MessageLib/messages/obj_controller_message.h"

namespace swganh
{
namespace messages
{
namespace controllers
{

class Posture : public ObjControllerMessage
{
public:
    explicit Posture(uint32_t controller_type = 0x0000001B)
        : ObjControllerMessage(controller_type, message_type())
    {}

    Posture(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x00000131;
    }

    uint8_t posture_id;

    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(posture_id);
        buffer.write<uint8_t>(1);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        posture_id = buffer.read<uint8_t>();
    }
};

}
}
}  // namespace swganh::messages::controllers
