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

struct ChatOnReceiveRoomInvitation : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xC17EB06D;
    }

    std::string game_name; // default: SWG
    std::string server_name; // galaxy name
    std::string moderator_name; // moderator
    std::string channel_name;

    ChatOnReceiveRoomInvitation()
        : game_name("SWG")
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(game_name);
        buffer.write(server_name);
        buffer.write(moderator_name);
        buffer.write(channel_name);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        game_name = buffer.read<std::string>();
        server_name = buffer.read<std::string>();
        moderator_name = buffer.read<std::string>();
        channel_name = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
