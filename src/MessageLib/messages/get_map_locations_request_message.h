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

struct GetMapLocationsRequestMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 5;
    }
    uint32_t Opcode() const
    {
        return 0x1A7AB839;
    }

    std::string planet_name;
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t unknown3;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(planet_name);
        buffer.write(unknown1);
        buffer.write(unknown2);
        buffer.write(unknown3);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        planet_name = buffer.read<std::string>();
        unknown1 = buffer.read<uint32_t>();
        unknown2 = buffer.read<uint32_t>();
        unknown3 = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
