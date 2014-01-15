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

struct ChatDestroyRoom : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x94B2A77;
    }

    uint32_t channel_id;
    uint32_t attempts_counter;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(channel_id);
        buffer.write(attempts_counter);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        channel_id = buffer.read<uint32_t>();
        attempts_counter = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
