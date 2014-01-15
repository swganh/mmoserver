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

struct ClientPermissionsMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0xE00730E5;
    }

    uint8_t galaxy_available;
    uint8_t available_character_slots;
    uint8_t unlimited_characters;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(galaxy_available);
        buffer.write(available_character_slots);
        buffer.write(unlimited_characters);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        galaxy_available = buffer.read<uint8_t>();
        available_character_slots = buffer.read<uint8_t>();
        unlimited_characters = buffer.read<uint8_t>();
    }
};

}
}  // namespace swganh::messages
