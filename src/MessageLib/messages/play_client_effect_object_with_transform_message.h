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

struct PlayClientEffectObjectWithTransformMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x4F5E09B6;
    }

    std::string effect_string;
    glm::quat orientation;
    glm::vec3 offset;
    uint64_t object_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(effect_string);
        buffer.write(orientation.x);
        buffer.write(orientation.y);
        buffer.write(orientation.z);
        buffer.write(orientation.w);
        buffer.write(offset.x);
        buffer.write(offset.y);
        buffer.write(offset.z);
        buffer.write(object_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        effect_string = buffer.read<std::string>();
        orientation.x = buffer.read<float>();
        orientation.y = buffer.read<float>();
        orientation.z = buffer.read<float>();
        orientation.w = buffer.read<float>();
        offset.x = buffer.read<float>();
        offset.y = buffer.read<float>();
        offset.z = buffer.read<float>();
        object_id = buffer.read<uint64_t>();
    }
};

}
} // namespace swganh::messages
