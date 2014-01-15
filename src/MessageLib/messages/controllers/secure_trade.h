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

class SecureTrade : public ObjControllerMessage
{
public:
    explicit SecureTrade(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
    {}

    SecureTrade(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x00000115;
    }

    uint32_t error;
    uint64_t trader_id; // sender
    uint64_t target_id; // recipient

    SecureTrade()
        : error(0)
        , trader_id(0)
        , target_id(0)
    {}

    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(error);
        buffer.write(trader_id);
        buffer.write(target_id);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        error = buffer.read<uint32_t>();
        trader_id = buffer.read<uint64_t>();
        target_id = buffer.read<uint64_t>();
    }
};

}
}
} // namespace swganh::messages::controllers
