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

struct UpdateTransformMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 8;
    }
    uint32_t Opcode() const
    {
        return 0x1B24F808;
    }

    uint64_t object_id;
    glm::vec3 position;
    uint32_t update_counter;
    uint8_t posture_id;
    uint8_t heading;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(object_id);
        buffer.write<int16_t>(static_cast<int16_t>(position.x * 4.0f + 0.5f));
        buffer.write<int16_t>(static_cast<int16_t>(position.y * 4.0f + 0.5f));
        buffer.write<int16_t>(static_cast<int16_t>(position.z * 4.0f + 0.5f));
        buffer.write(update_counter);
        buffer.write(static_cast<uint8_t>(glm::length(position) * 4.0f * 0.5f));
        buffer.write(heading);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        object_id = buffer.read<uint64_t>();
        position.x = buffer.read<int16_t>();
        position.y = buffer.read<int16_t>();
        position.z = buffer.read<int16_t>();
        update_counter = buffer.read<uint32_t>();
        posture_id = buffer.read<uint8_t>();
        heading = buffer.read<uint8_t>();
    }
};

}
}  // namespace swganh::messages
