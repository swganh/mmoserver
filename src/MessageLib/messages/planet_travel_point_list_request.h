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

struct PlanetTravelPointListRequest : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x96405D4D;
    }

    uint64_t travel_terminal_id;
    std::string planet_name;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(travel_terminal_id);
        buffer.write(planet_name);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        travel_terminal_id = buffer.read<uint64_t>();
        planet_name = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
