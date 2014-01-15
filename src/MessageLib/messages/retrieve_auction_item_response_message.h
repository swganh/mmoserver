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

struct RetrieveAuctionItemResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x9499EF8C;
    }

    uint64_t item_id;
    uint32_t success_flag; // 00 = success, 01 = You are not allowed to retrieve that item, 12 = Inventory full

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(item_id);
        buffer.write(success_flag);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        item_id = buffer.read<uint64_t>();
        success_flag = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
