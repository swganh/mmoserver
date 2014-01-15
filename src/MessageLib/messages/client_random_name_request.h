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

struct ClientRandomNameRequest : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0xD6D1B6D1;
    }

    std::string player_race_iff;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(player_race_iff);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        player_race_iff = buffer.read<std::string>();
    }
};

}
}  // namespace swganh::messages
