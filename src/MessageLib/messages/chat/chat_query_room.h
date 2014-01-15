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

struct ChatQueryRoom : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x9cf2b192;
    }

    uint32_t request_id;
    std::string channel_path; // path to the channel, e.g. "swg/server/tatooine/<channel_name>" (presumably exclude channel_name)

    ChatQueryRoom()
        : request_id(1)
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<uint32_t>(request_id);
        buffer.write<std::string>(channel_path);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        request_id = buffer.read<uint32_t>();
        channel_path = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
