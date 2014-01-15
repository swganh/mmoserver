// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ItemInfo
{
    uint64_t item_id;
    uint8_t item_string_number;
    uint32_t price1;
    uint32_t expiration_time;
    uint32_t auction_flag;
    uint16_t location_index;
    uint64_t seller_object_id;
    uint8_t seller_pointer; // unknown
    uint64_t buyer_object_id;
    uint8_t buyer_pointer; // uncertain
    uint32_t unknown1; // unknown
    uint32_t price2;
    uint32_t item_type;
    uint32_t unknown2; // unknown
    uint32_t vendor_entrance_fee;
};

struct AuctionQueryHeadersResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 8;
    }
    uint32_t Opcode() const
    {
        return 0xFA500E52;
    }

    uint32_t counter;
    uint32_t vendor_screen_flag; // 2 = All items, 3 = My sales, 4 = My bids, 5 = Available items, 7 = For sale, 9 = Offers to vendor
    std::vector<std::string> location_names;
    std::vector<std::wstring> item_names;
    std::vector<ItemInfo> item_info_list;
    uint16_t item_list_start_offset; // WHen the continuation flag is enabled, this offset could have a value other than 0
    uint8_t continuation_flag; // Determines whether or not more items are to be added to the list

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(counter);
        buffer.write(vendor_screen_flag);
        buffer.write<uint32_t>(location_names.size());
        std::for_each(location_names.begin(), location_names.end(), [&buffer] (std::string location_name)
        {
            buffer.write(location_name);
        });
        buffer.write<uint32_t>(item_names.size());
        std::for_each(item_names.begin(), item_names.end(), [&buffer] (std::wstring item_name)
        {
            buffer.write(item_name);
        });
        buffer.write<uint32_t>(item_info_list.size());
        std::for_each(item_info_list.begin(), item_info_list.end(), [&buffer] (ItemInfo item)
        {
            buffer.write(item.item_id);
            buffer.write(item.item_string_number);
            buffer.write(item.price1);
            buffer.write(item.expiration_time);
            buffer.write(item.auction_flag);
            buffer.write(item.location_index);
            buffer.write(item.seller_object_id);
            buffer.write(item.seller_pointer);
            buffer.write(item.buyer_object_id);
            buffer.write(item.buyer_pointer);
            buffer.write(item.unknown1);
            buffer.write(item.price2);
            buffer.write(item.item_type);
            buffer.write(item.unknown2);
            buffer.write(item.vendor_entrance_fee);
        });
        buffer.write(item_list_start_offset);
        buffer.write(continuation_flag);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        counter = buffer.read<uint32_t>();
        vendor_screen_flag = buffer.read<uint32_t>();
        uint32_t location_names_count = buffer.read<uint32_t>();
        for(uint32_t i = 0; i < location_names_count; i++)
        {
            std::string location_name;
            location_name = buffer.read<std::string>();
            location_names.push_back(location_name);
        }
        uint32_t item_names_count = buffer.read<uint32_t>();
        for(uint32_t i = 0; i < item_names_count; i++)
        {
            std::wstring item_name;
            item_name = buffer.read<std::wstring>();
            item_names.push_back(item_name);
        }
        uint32_t item_info_count = buffer.read<uint32_t>();
        for(uint32_t i = 0; i < item_info_count; i++)
        {
            ItemInfo item;
            item.item_id = buffer.read<uint64_t>();
            item.item_string_number = buffer.read<uint8_t>();
            item.price1 = buffer.read<uint32_t>();
            item.expiration_time = buffer.read<uint32_t>();
            item.auction_flag = buffer.read<uint32_t>();
            item.location_index = buffer.read<uint16_t>();
            item.seller_object_id = buffer.read<uint64_t>();
            item.seller_pointer = buffer.read<uint8_t>();
            item.buyer_object_id = buffer.read<uint64_t>();
            item.buyer_pointer = buffer.read<uint8_t>();
            item.unknown1 = buffer.read<uint32_t>();
            item.price2 = buffer.read<uint32_t>();
            item.item_type = buffer.read<uint32_t>();
            item.unknown2 = buffer.read<uint32_t>();
            item.vendor_entrance_fee = buffer.read<uint32_t>();
            item_info_list.push_back(item);
        }
        item_list_start_offset = buffer.read<uint16_t>();
        continuation_flag = buffer.read<uint8_t>();
    }
};

}
} // namespace swganh::messages
