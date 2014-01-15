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

struct ChatPersistentMessageToClient : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x08485E17;
    }

    std::string sender_character_name;
    std::string game_name; // arbitrary: "SWG"
    std::string server_name; // galaxy name
    uint32_t mail_message_id;
    uint8_t request_type_flag; // 01 = listing message subjects in the mail box, 00 = displaying the message body for the message requested in ChatRequestPersistentMessage
    std::wstring mail_message_body;
    std::wstring mail_message_subject;
    uint32_t null_spacer;
    uint8_t status; // N = New, R = Read, U = Unread
    uint32_t timestamp;
    uint32_t unknown;
    std::vector<char> attachment_data;

    ChatPersistentMessageToClient()
        : game_name("SWG")
        , null_spacer(0)
        , unknown(0)
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(sender_character_name);
        buffer.write(game_name);
        buffer.write(server_name);
        buffer.write(mail_message_id);
        buffer.write(request_type_flag);
        buffer.write(mail_message_body);
        buffer.write(mail_message_subject);
        buffer.write(uint32_t(attachment_data.size()));
        buffer.write(attachment_data);
        buffer.write(status);
        buffer.write(timestamp);
        buffer.write(unknown);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        sender_character_name = buffer.read<std::string>();
        game_name = buffer.read<std::string>();
        server_name = buffer.read<std::string>();
        mail_message_id = buffer.read<uint32_t>();
        request_type_flag = buffer.read<uint8_t>();
        mail_message_body = buffer.read<std::wstring>();
        mail_message_subject = buffer.read<std::wstring>();

        auto size = buffer.read<uint32_t>();
        while (size > 0)
        {
            attachment_data.push_back(buffer.read<char>());
            --size;
        }
        status = buffer.read<uint8_t>();
        timestamp = buffer.read<uint32_t>();
        unknown = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
