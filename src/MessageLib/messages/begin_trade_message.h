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

struct BeginTradeMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 1;
    }
    uint32_t Opcode() const
    {
        return 0x325932D8;
    }

    uint64_t target_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(target_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        target_id = buffer.read<uint64_t>();
    }
};

}
} // namespace swganh::messages
