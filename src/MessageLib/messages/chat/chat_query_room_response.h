// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"
#include "MessageLib/messages/base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ChatQueryRoomResponse : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 7;
    }
    uint32_t Opcode() const
    {
        return 0xC4DE864E;
    }

    swganh::ByteBuffer data;

    ChatQueryRoomResponse()
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.append(data);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer) {}
};

}
} // namespace swganh::messages
