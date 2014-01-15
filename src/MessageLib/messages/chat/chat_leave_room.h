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

struct ChatLeaveRoom : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x493e3ffa;
    }

    std::string game, galaxy, character;
    std::string channel_path;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<std::string>(game);
        buffer.write<std::string>(galaxy);
        buffer.write<std::string>(character);
        buffer.write<std::string>(channel_path);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        game = buffer.read<std::string>();
        galaxy = buffer.read<std::string>();
        character = buffer.read<std::string>();

        channel_path = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
