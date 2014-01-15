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

struct ChatServerStatus : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x7102B15F;
    }

    uint8_t status;

    ChatServerStatus(uint8_t status_)
        : status(status_)
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<uint8_t>(status);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        status = buffer.read<uint8_t>();
    }
};

}
} // namespace swganh::messages
