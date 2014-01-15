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

struct DeleteCharacterReplyMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x8268989B;
    }

    int32_t failure_flag;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<int32_t>(failure_flag);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        failure_flag = buffer.read<int32_t>();
    }
};

}
}  // namespace swganh::messages
