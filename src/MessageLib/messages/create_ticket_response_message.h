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

struct CreateTicketResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x550A407A;
    }

    uint32_t message_flag; // 0 = success, 1 = failure
    uint32_t ticket_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(message_flag);
        buffer.write(ticket_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        message_flag = buffer.read<uint32_t>();
        ticket_id = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
