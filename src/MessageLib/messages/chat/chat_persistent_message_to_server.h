// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include "anh/byte_buffer.h"
#include "MessageLib/messages/base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ChatPersistentMessageToServer : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 6;
    }
    uint32_t Opcode() const
    {
        return 0x25A29FA6;
    }

    std::wstring message;
    std::vector<char> attachment_data;
    uint32_t mail_id;
    std::wstring subject;
    uint32_t spacer;
    std::string recipient;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(message);
        buffer.write(uint32_t(attachment_data.size()));
        buffer.write(attachment_data);
        buffer.write(mail_id);
        buffer.write(subject);
        buffer.write(uint32_t(0));
        buffer.write(recipient);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        message = buffer.read<std::wstring>();
        auto size = buffer.read<uint32_t>();
        while (size > 0)
        {
            attachment_data.push_back(buffer.read<char>());
            --size;
        }

        mail_id = buffer.read<uint32_t>();
        subject = buffer.read<std::wstring>();
        spacer = buffer.read<uint32_t>();
        recipient = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
