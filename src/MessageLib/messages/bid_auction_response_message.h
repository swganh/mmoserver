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

struct BidAuctionResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xC58A446E;
    }

    uint64_t item_id;
    uint32_t status_flag; // 0 = Succeeded, 1 = Auctioneer is Invalid, 2 = Invalid Item, 9 = Not Enough Credits

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(item_id);
        buffer.write(status_flag);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        item_id = buffer.read<uint64_t>();
        status_flag = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
