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

class WatchEntertainer : public ObjControllerMessage
{
public:
    explicit WatchEntertainer(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
        , entertainer_id(0)
    {}

    WatchEntertainer(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x0000022B;
    }

    uint64_t entertainer_id;


    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(entertainer_id);
        buffer.write<uint32_t>(0);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        entertainer_id = buffer.read<uint64_t>();
    }
};

}
}
}  // namespace swganh::messages::controllers
