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

struct ChatInstantMessageToCharacter : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 5;
    }
    uint32_t Opcode() const
    {
        return 0x84BB21F7;
    }

    std::string game_name; // arbitrary: "SWG"
    std::string server_name; // galaxy name
    std::string recipient_character_name; // recipient (the server will send ChatInstantMessageToClient to the recipient)
    std::wstring message;
    uint32_t unknown;
    uint32_t sequence_number;

    ChatInstantMessageToCharacter()
        : game_name("SWG")
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(game_name);
        buffer.write(server_name);
        buffer.write(recipient_character_name);
        buffer.write(message);
        buffer.write(unknown);
        buffer.write(sequence_number);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        game_name = buffer.read<std::string>();
        server_name = buffer.read<std::string>();
        recipient_character_name = buffer.read<std::string>();
        message = buffer.read<std::wstring>();
        unknown = buffer.read<uint32_t>();
        sequence_number = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
