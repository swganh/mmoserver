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

struct ItemSoldMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x0E61CC92;
    }

    uint64_t item_id;
    uint32_t status_flag; // See wiki.opengalaxies.org/ItemSoldMessage for status flags

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
