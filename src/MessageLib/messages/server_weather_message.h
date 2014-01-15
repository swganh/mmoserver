// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ServerWeatherMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x486356EA;
    }

    uint32_t weather_id;
    glm::vec3 cloud_vector;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(weather_id);
        buffer.write(cloud_vector.x);
        buffer.write(cloud_vector.z);
        buffer.write(cloud_vector.y);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        weather_id = buffer.read<uint32_t>();
        cloud_vector.x = buffer.read<float>();
        cloud_vector.z = buffer.read<float>();
        cloud_vector.y = buffer.read<float>();
    }
};

}
} // namespace swganh::messages
