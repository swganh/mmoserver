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

struct EnterTicketPurchaseModeMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x904DAE1A;
    }

    std::string planet_name; // planet on which the terminal is located
    std::string city_name; // city in which the terminal is located

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(planet_name);
        buffer.write(city_name);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        planet_name = buffer.read<std::string>();
        city_name = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
