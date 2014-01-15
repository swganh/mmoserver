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

struct GuildResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 1;
    }
    uint32_t Opcode() const
    {
        return 0x32263F20;
    }

    uint64_t guild_id;
    std::string guild_name;
    uint16_t unknown;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(guild_id);
        buffer.write(guild_name);
        buffer.write(unknown);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        guild_id = buffer.read<uint64_t>();
        guild_name = buffer.read<std::string>();
        unknown = buffer.read<uint16_t>();
    }
};

}
} // namespace swganh::messages
