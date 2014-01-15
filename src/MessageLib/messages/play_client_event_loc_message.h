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

struct PlayClientEventLocMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 9;
    }
    uint32_t Opcode() const
    {
        return 0x0A4E222C;
    }

    std::string event_group_string;
    std::string event_string;
    std::string planet_name_string;
    glm::vec3 location_coordinates;
    uint64_t cell_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(event_group_string);
        buffer.write(event_string);
        buffer.write(planet_name_string);
        buffer.write(location_coordinates.x);
        buffer.write(0.0f);
        buffer.write(location_coordinates.z);
        buffer.write(cell_id);
        buffer.write(location_coordinates.y);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        event_group_string = buffer.read<std::string>();
        event_string = buffer.read<std::string>();
        planet_name_string = buffer.read<std::string>();
        location_coordinates.x = buffer.read<float>();
        buffer.read<float>(); //This value is ignored by the client and the server
        location_coordinates.z = buffer.read<float>();
        cell_id = buffer.read<uint64_t>();
        location_coordinates.y = buffer.read<float>();
    }
};

}
} // namespace swganh::messages
