// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct SceneCreateObjectByCrc : public BaseSwgMessage
{
    SceneCreateObjectByCrc() {}
    SceneCreateObjectByCrc(SceneCreateObjectByCrc* other)
    {
        this->object_id = other->object_id;
        this->orientation = other->orientation;
        this->position = other->position;
        this->object_crc = other->object_crc;
        this->byte_flag = other->byte_flag;
    }

    const static uint32_t opcode = 0xFE89DDEA;
    uint16_t Opcount() const
    {
        return 5;
    }
    uint32_t Opcode() const
    {
        return opcode;
    }

    uint64_t object_id;
    glm::quat orientation;
    glm::vec3 position;
    uint32_t object_crc;
    uint8_t byte_flag;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(object_id);
        buffer.write(orientation.x);
        buffer.write(orientation.y);
        buffer.write(orientation.z);
        buffer.write(orientation.w);
        buffer.write(position.x);
        buffer.write(position.y);
        buffer.write(position.z);
        buffer.write(object_crc);
        buffer.write(byte_flag);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        object_id = buffer.read<uint64_t>();
        orientation.x = buffer.read<float>();
        orientation.y = buffer.read<float>();
        orientation.z = buffer.read<float>();
        orientation.w = buffer.read<float>();
        position.x = buffer.read<float>();
        position.y = buffer.read<float>();
        position.z = buffer.read<float>();
        object_crc = buffer.read<uint32_t>();
        byte_flag = buffer.read<uint8_t>();
    }
};

}
}  // namespace swganh::messages
