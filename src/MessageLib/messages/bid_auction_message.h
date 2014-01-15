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

struct BidAuctionMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x91125453;
    }

    uint64_t item_id;
    uint32_t bid_price;
    uint32_t price_proxy;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(item_id);
        buffer.write(bid_price);
        buffer.write(price_proxy);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        item_id = buffer.read<uint64_t>();
        bid_price = buffer.read<uint32_t>();
        price_proxy = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
