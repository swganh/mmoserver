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

struct UpdateContainmentMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x56CBDE9E;
    }

    uint64_t object_id;
    uint64_t container_id;
    int32_t containment_type;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(object_id);
        buffer.write(container_id);
        buffer.write(containment_type);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        object_id = buffer.read<uint64_t>();
        container_id = buffer.read<uint64_t>();
        containment_type = buffer.read<int32_t>();
    }
};

}
}  // namespace swganh::messages
