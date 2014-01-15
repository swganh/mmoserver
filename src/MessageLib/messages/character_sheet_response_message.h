// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct CharacterSheetResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 1;
    }
    uint32_t Opcode() const
    {
        return 0x9B3A17C4;
    }

    uint32_t unknown1;
    uint32_t unknown2;
    glm::vec3 bind_location;
    std::string bind_planet_name; // planet the player is bound to
    glm::vec3 bank_location;
    std::string bank_planet_name; // planet the player's bank is located on
    glm::vec3 home_location;
    std::string home_planet_name; // planet the player's home is located on
    std::wstring spouse_name; // name of character's spouse
    uint32_t available_lots;
    uint32_t faction_crc;
    uint32_t faction_status; // 00 = neutral, 01 = covert, 02 = overt

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(unknown1);
        buffer.write(unknown2);
        buffer.write(bind_location.x);
        buffer.write(bind_location.y);
        buffer.write(bind_location.z);
        buffer.write(bind_planet_name);
        buffer.write(bank_location.x);
        buffer.write(bank_location.y);
        buffer.write(bank_location.z);
        buffer.write(bank_planet_name);
        buffer.write(home_location.x);
        buffer.write(home_location.y);
        buffer.write(home_location.z);
        buffer.write(home_planet_name);
        buffer.write(spouse_name);
        buffer.write(available_lots);
        buffer.write(faction_crc);
        buffer.write(faction_status);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        unknown1 = buffer.read<uint32_t>();
        unknown2 = buffer.read<uint32_t>();
        bind_location.x = buffer.read<float>();
        bind_location.y = buffer.read<float>();
        bind_location.z = buffer.read<float>();
        bind_planet_name = buffer.read<std::string>();
        bank_location.x = buffer.read<float>();
        bank_location.y = buffer.read<float>();
        bank_location.z = buffer.read<float>();
        bank_planet_name = buffer.read<std::string>();
        home_location.x = buffer.read<float>();
        home_location.y = buffer.read<float>();
        home_location.z = buffer.read<float>();
        home_planet_name = buffer.read<std::string>();
        spouse_name = buffer.read<std::wstring>();
        available_lots = buffer.read<uint32_t>();
        faction_crc = buffer.read<uint32_t>();
        faction_status = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
