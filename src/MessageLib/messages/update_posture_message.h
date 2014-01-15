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
namespace controllers
{

struct UpdatePostureMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x0BDE6B41;
    }

    int8_t posture_id;
    uint64_t object_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(posture_id);
        buffer.write(object_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        posture_id = buffer.read<int8_t>();
        object_id = buffer.read<uint64_t>();
    }
};

}
}
}  // namespace swganh::messages::controllers
