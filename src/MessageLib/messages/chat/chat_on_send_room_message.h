// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"
#include "MessageLib/messages/base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ChatOnSendRoomMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x20E4DBE3;
    }

    ChatOnSendRoomMessage(uint32_t error_code_, uint32_t message_counter_)
        : error_code(error_code_)
        , message_counter(message_counter_)
    {
    }

    uint32_t error_code;
    uint32_t message_counter;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(error_code);
        buffer.write(message_counter);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        error_code = buffer.read<uint32_t>();
        message_counter = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
