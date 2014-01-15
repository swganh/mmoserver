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

struct NewTicketActivityResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x6EA42D80;
    }

    uint8_t update_status; // 0 = not updated; 1 = updated
    uint64_t ticket_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(update_status);
        buffer.write(ticket_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        update_status = buffer.read<uint8_t>();
        ticket_id = buffer.read<uint64_t>();
    }
};

}
} // namespace swganh::messages
