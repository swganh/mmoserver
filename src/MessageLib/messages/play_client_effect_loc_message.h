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

struct PlayClientEffectLocMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 8;
    }
    uint32_t Opcode() const
    {
        return 0x02949E74;
    }

    std::string client_effect_file; // e.g. "clienteffect/lambda_hover.cef"
    std::string planet_name; // e.g. "naboo"
    glm::vec3 location_coordinates;
    uint64_t cell_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(client_effect_file);
        buffer.write(planet_name);
        buffer.write(location_coordinates.x);
        buffer.write(0.0f);
        buffer.write(location_coordinates.z);
        buffer.write(cell_id);
        buffer.write(location_coordinates.y);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        client_effect_file = buffer.read<std::string>();
        planet_name = buffer.read<std::string>();
        location_coordinates.x = buffer.read<float>();
        buffer.read<float>(); //Both client and server ignore this value
        location_coordinates.z = buffer.read<float>();
        cell_id = buffer.read<uint64_t>();
        location_coordinates.y = buffer.read<float>();
    }
};

}
} // namespace swganh::messages
