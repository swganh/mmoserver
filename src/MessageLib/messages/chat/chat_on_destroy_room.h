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

struct ChatOnDestroyRoom : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 5;
    }
    uint32_t Opcode() const
    {
        return 0xE8EC5877;
    }

    std::string game_name; // default: SWG
    std::string server_name; // galaxy name
    std::string owner_name; // arbitrary: "system"
    uint32_t error;
    uint32_t channel_id;
    uint32_t request_id;

    ChatOnDestroyRoom()
        : game_name("SWG")
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(game_name);
        buffer.write(server_name);
        buffer.write(owner_name);
        buffer.write(error);
        buffer.write(channel_id);
        buffer.write(request_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        game_name = buffer.read<std::string>();
        server_name = buffer.read<std::string>();
        owner_name = buffer.read<std::string>();
        error = buffer.read<uint32_t>();
        channel_id = buffer.read<uint32_t>();
        request_id = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
