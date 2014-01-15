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

struct VerifyPlayerNameResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 9;
    }
    uint32_t Opcode() const
    {
        return 0xF4C498FD;
    }

    uint8_t success_flag; // 0 = name not accepted, 1 = name accepted
    uint32_t attempts_counter; // corresponds to the VerifyPlayerNameMessage packet

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(success_flag);
        buffer.write(attempts_counter);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        success_flag = buffer.read<uint8_t>();
        attempts_counter = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
