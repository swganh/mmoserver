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

struct SUIForceClose : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x990B5DE0;
    }

    int32_t window_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(window_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        window_id = buffer.read<int32_t>();
    }
};

}
} // namespace swganh::messages
