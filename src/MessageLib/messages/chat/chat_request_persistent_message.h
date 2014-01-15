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

struct ChatRequestPersistentMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x07E3559F;
    }

    uint32_t spacer;
    uint32_t mail_message_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(spacer);
        buffer.write(mail_message_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        spacer = buffer.read<uint32_t>();
        mail_message_id = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
