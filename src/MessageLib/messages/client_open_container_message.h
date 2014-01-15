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

struct ClientOpenContainerMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0xDCA57409;
    }

    uint64_t container_object_id; // object_id of the container to open
    uint32_t unknown1;
    uint32_t unknown2;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(container_object_id);
        buffer.write(unknown1);
        buffer.write(unknown2);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        container_object_id = buffer.read<uint64_t>();
        unknown1 = buffer.read<uint32_t>();
        unknown2 = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
