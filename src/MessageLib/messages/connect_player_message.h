// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ConnectPlayerMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x2E365218;
    }

    uint32_t unknown; // always 0

    ConnectPlayerMessage()
        : unknown(0)
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(unknown);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        unknown = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
