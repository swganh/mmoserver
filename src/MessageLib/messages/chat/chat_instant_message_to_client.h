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

struct ChatInstantMessageToClient : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x3C565CED;
    }

    std::string game_name; // arbitrary: "SWG"
    std::string server_name; // galaxy name
    std::string sender_character_name; // sender (the recipient receives this packet)
    std::wstring message;

    ChatInstantMessageToClient()
    {}

    ChatInstantMessageToClient(const std::string& game_name,
                               const std::string& server_name,
                               const std::wstring& sender_character_name,
                               const std::wstring& message)
        : game_name(game_name)
        , server_name(server_name)
        , sender_character_name(std::begin(sender_character_name), std::end(sender_character_name))
        , message(message)
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(game_name);
        buffer.write(server_name);
        buffer.write(sender_character_name);
        buffer.write(message);
        buffer.write(uint32_t(0));
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        game_name = buffer.read<std::string>();
        server_name = buffer.read<std::string>();
        sender_character_name = buffer.read<std::string>();
        message = buffer.read<std::wstring>();
        buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
