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

struct ChatSendToRoom : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 5;
    }
    uint32_t Opcode() const
    {
        return 0x20E4DBE3;
    }

    std::wstring message;

    uint32_t channel_id;
    uint32_t message_counter;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(message);
        buffer.write<uint32_t>(0); //spacer
        buffer.write(channel_id);
        buffer.write(message_counter);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        message = buffer.read<std::wstring>();
        buffer.read<uint32_t>(); //spacer
        channel_id = buffer.read<uint32_t>();
        message_counter = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
