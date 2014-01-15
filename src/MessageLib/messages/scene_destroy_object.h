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

struct SceneDestroyObject : public BaseSwgMessage
{
    SceneDestroyObject() {}
    SceneDestroyObject(SceneDestroyObject* other)
    {
        this->object_id = other->object_id;
    }

    const static uint32_t opcode = 0x4D45D504;
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return opcode;
    }

    uint64_t object_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(object_id);
        buffer.write<uint8_t>(0);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        object_id = buffer.read<uint64_t>();
        buffer.read<uint8_t>();
    }
};

}
}  // namespace swganh::messages
