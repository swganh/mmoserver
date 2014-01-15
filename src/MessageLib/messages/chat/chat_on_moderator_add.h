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

struct ChatOnModeratorAdd : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 6;
    }
    uint32_t Opcode() const
    {
        return 0x36A03858;
    }

    std::string game;
    std::string server;
    std::string target;

    std::string moderator;

    //0=success,
    //4=avatar not found,
    //5 = room not exist,
    //9 = not moderated
    //16 = not moderator
    uint32_t error;

    std::string room_path;

    uint32_t request_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<std::string>(game);
        buffer.write<std::string>(server);
        buffer.write<std::string>(target);
        buffer.write<std::string>(moderator);
        buffer.write<uint32_t>(error);
        buffer.write<std::string>(room_path);
        buffer.write<uint32_t>(request_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        game = buffer.read<std::string>();
        server = buffer.read<std::string>();
        target = buffer.read<std::string>();
        moderator = buffer.read<std::string>();
        error = buffer.read<uint32_t>();
        room_path = buffer.read<std::string>();
        request_id = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
