// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <algorithm>
#include <list>
#include <string>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ItemDetail
{
    std::string descriptor;
    std::wstring description;
}

struct GetAuctionDetailsResponse : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 9;
    }
    uint32_t Opcode() const
    {
        return 0xFE0E644B;
    }

    uint64_t item_id;
    std::list<ItemDetail> auction_details;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(item_id);
        buffer.write<uint32_t>(auction_details.size());
        std::for_each(auction_details.begin(), auction_details.end(), [&buffer] (ItemDetail detail)
        {
            buffer.write(detail.descriptor);
            buffer.write(detail.description);
        });
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        item_id = buffer.read<uint64_t>();
        uint32_t auction_details_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < auction_details_count; i++)
        {
            ItemDetail detail;
            detail.descriptor = buffer.read<std::string>();
            detail.description = buffer.read<std::wstring>();
            auction_details.push_back(detail);
        }
    }
};

}
} // namespace swganh::messages
