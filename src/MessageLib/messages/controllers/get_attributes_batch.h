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

class GetAttributesBatchMessage : public ObjControllerMessage
{
public:
    explicit GetAttributesBatchMessage(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
    {}

    GetAttributesBatchMessage(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x164550EF;
    }


    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<uint32_t>(0);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
    }
};

}
}
} // namespace swganh::messages::controllers
