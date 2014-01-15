// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <array>
#include <bitset>

#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct BadgesResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x6D89D25B;
    }

    uint64_t character_id;
    std::array<std::bitset<32>, 15> badge_flags;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(character_id);
        buffer.write<uint32_t>(15);
        for(uint32_t index = 0; index < 15; index++)
        {
            buffer.write<uint32_t>(badge_flags[index].to_ulong());
        }
        buffer.write<uint8_t>(0);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer) {}
};

}
} // namespace swganh::messages
