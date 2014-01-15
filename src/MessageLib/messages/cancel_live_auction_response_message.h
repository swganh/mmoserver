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

struct CancelLiveAuctionResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x7DA2246C;
    }

    uint64_t item_id;
    uint32_t error; // 0 = success, 1 = not allowed, 2 = item invalid, 3 = blank, 8 = must be owner of item to cancel, 15 = too late/auction already completed
    uint8_t unknown;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(item_id);
        buffer.write(error);
        buffer.write(unknown);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        item_id = buffer.read<uint64_t>();
        error = buffer.read<uint32_t>();
        unknown = buffer.read<uint8_t>();
    }
};

}
} // namespace swganh::messages
