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

struct ChatRoomList : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x70DEB197;
    }

    swganh::ByteBuffer data;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.append(data);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer) {}
};

}
} // namespace swganh::messages
