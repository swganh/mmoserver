// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include "anh/byte_buffer.h"
#include "MessageLib/messages/base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ChatOnSendInstantMessage : public BaseSwgMessage
{
    enum RESPONSE_TYPES
    {
        OK = 0,
        FAILED = 4
    };

    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x88DBB381;
    }

    uint32_t success_flag; // 0x00 = success, 0x04 = failure
    uint32_t sequence_number;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(success_flag);
        buffer.write(sequence_number);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        success_flag = buffer.read<uint32_t>();
        sequence_number = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
