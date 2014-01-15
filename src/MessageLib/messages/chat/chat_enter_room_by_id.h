// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include "anh/byte_buffer.h"
#include "MessageLib/messages/base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ChatEnterRoomById : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xBC6BDDF2;
    }

    uint32_t attempts_counter;
    uint32_t channel_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(attempts_counter);
        buffer.write(channel_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        attempts_counter = buffer.read<uint32_t>();
        channel_id = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
