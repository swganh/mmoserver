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

struct ChatOnEnteredRoom : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 5;
    }
    uint32_t Opcode() const
    {
        return 0xE69BDC0A;
    }

    std::string game_name; // default: SWG
    std::string server_name; // galaxy name
    std::string character_name;
    uint32_t success_bitmask;
    uint32_t channel_id;
    uint32_t nothing;

    ChatOnEnteredRoom()
        : game_name("SWG")
        , nothing(0)
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(game_name);
        buffer.write(server_name);
        buffer.write(character_name);
        buffer.write(success_bitmask);
        buffer.write(channel_id);
        buffer.write(nothing);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        game_name = buffer.read<std::string>();
        server_name = buffer.read<std::string>();
        character_name = buffer.read<std::string>();
        success_bitmask = buffer.read<uint32_t>();
        channel_id = buffer.read<uint32_t>();
        nothing = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
