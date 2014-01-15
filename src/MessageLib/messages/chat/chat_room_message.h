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

class ChatRoomMessage : public BaseSwgMessage
{
public:
    uint16_t Opcount() const
    {
        return 5;
    }
    uint32_t Opcode() const
    {
        return 0xCD4CE444;
    }

    std::string game_name; // arbitrary: "SWG"
    std::string server_name; // galaxy name
    std::string sender_character_name;
    uint32_t channel_id;
    std::wstring message;
    std::wstring out_of_band; // apparently chat room messages usually don't send OutOfBand packages, but implementing just in case

    ChatRoomMessage()
        : game_name("SWG")
    {}

    void AddProsePackage(const swganh::ByteBuffer* prose_package)
    {
        prose_package_ = prose_package;
    }

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(game_name);
        buffer.write(server_name);
        buffer.write(sender_character_name);
        buffer.write(channel_id);
        buffer.write(message);
        buffer.write(out_of_band);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        game_name = buffer.read<std::string>();
        server_name = buffer.read<std::string>();
        sender_character_name = buffer.read<std::string>();
        channel_id = buffer.read<uint32_t>();
        message = buffer.read<std::wstring>();
        out_of_band = buffer.read<std::wstring>();
    }

private:
    const swganh::ByteBuffer* prose_package_;
};

}
} // namespace swganh::messages
