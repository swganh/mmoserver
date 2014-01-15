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

class Animate : public ObjControllerMessage
{
public:
    explicit Animate(uint32_t controller_type = 0x0000001B)
        : ObjControllerMessage(controller_type, message_type())
    {}

    Animate(const std::string& animation_, uint64_t object_id_)
        : animation(animation_)
        , object_id(object_id_)
    {}

    Animate(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x000000F2;
    }

    uint64_t object_id;
    std::string animation;

    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(object_id);
        buffer.write<uint32_t>(0);
        buffer.write(animation);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        object_id = buffer.read<uint64_t>();
        buffer.read<uint32_t>();
        animation = buffer.read<std::string>();
    }
};

}
}
}  // namespace swganh::messages::controllers
