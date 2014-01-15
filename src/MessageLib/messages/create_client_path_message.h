// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <algorithm>
#include <glm/glm.hpp>
#include <list>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct CreateClientPathMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 5;
    }
    uint32_t Opcode() const
    {
        return 0x71957628;
    }

    std::list<glm::vec3> path_coordinates;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<uint32_t>(path_coordinates.size());
        std::for_each(path_coordinates.begin(), path_coordinates.end(), [&buffer] (glm::vec3 coordinate)
        {
            buffer.write(coordinate.x);
            buffer.write(coordinate.z);
            buffer.write(coordinate.y);
        });
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        uint32_t coordinate_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < coordinate_count; i++)
        {
            glm::vec3 coordinate;
            coordinate.x = buffer.read<float>();
            coordinate.z = buffer.read<float>();
            coordinate.y = buffer.read<float>();
            path_coordinates.push_back(coordinate);
        }
    }
};

}
} // namespace swganh::messages
