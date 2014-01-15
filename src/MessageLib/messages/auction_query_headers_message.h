// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct AuctionQueryHeadersMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 14;
    }
    uint32_t Opcode() const
    {
        return 0x679E0D00;
    }

    uint32_t region_flag; // 0 = Galaxy, 1 = This planet, 2 = This region, 3 = Vendor only
    uint32_t attempts_counter;
    uint32_t window_type; // 2 = All auctions, 3 = My sales, 4 = My bids, 5 = Available items, 6 = Offers, 7 = For sale (vendor) / Vendor locations (bazaar), 8 = Stockroom, 9 = Offers to vendor
    uint8_t subcategory;
    uint8_t category1;
    uint8_t category2;
    uint8_t category3;
    uint32_t item_type;
    std::wstring search_text;
    uint32_t unknown;
    uint32_t search_min_price;
    uint32_t search_max_price;
    uint8_t include_entrance_fee_flag;
    uint64_t vendor_id;
    uint8_t vendor_flag; // 0 = bazaar terminal, 1 = vendor
    uint16_t first_auction_to_display; // number of the first auction to display

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(region_flag);
        buffer.write(attempts_counter);
        buffer.write(window_type);
        buffer.write(subcategory);
        buffer.write(category1);
        buffer.write(category2);
        buffer.write(category3);
        buffer.write(item_type);
        buffer.write(search_text);
        buffer.write(unknown);
        buffer.write(search_min_price);
        buffer.write(search_max_price);
        buffer.write(include_entrance_fee_flag);
        buffer.write(vendor_id);
        buffer.write(vendor_flag);
        buffer.write(first_auction_to_display);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        region_flag = buffer.read<uint32_t>();
        attempts_counter = buffer.read<uint32_t>();
        window_type = buffer.read<uint32_t>();
        subcategory = buffer.read<uint8_t>();
        category1 = buffer.read<uint8_t>();
        category2 = buffer.read<uint8_t>();
        category3 = buffer.read<uint8_t>();
        item_type = buffer.read<uint32_t>();
        search_text = buffer.read<std::wstring>();
        unknown = buffer.read<uint32_t>();
        search_min_price = buffer.read<uint32_t>();
        search_max_price = buffer.read<uint32_t>();
        include_entrance_fee_flag = buffer.read<uint8_t>();
        vendor_id = buffer.read<uint64_t>();
        vendor_flag = buffer.read<uint8_t>();
        first_auction_to_display = buffer.read<uint16_t>();
    }
};

}
} // namespace swganh::messages
