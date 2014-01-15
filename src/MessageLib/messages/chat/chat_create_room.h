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

struct ChatCreateRoom : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 7;
    }
    uint32_t Opcode() const
    {
        return 0x35366BED;
    }

    uint8_t public_flag; // 0 = private, 1 = public
    uint8_t moderation_flag; // 0 = unmoderated, 1 = moderated
    uint16_t unknown;
    std::string channel_path; // path to the channel, e.g. "swg/<server>/tatooine/<channel_name>"
    std::string channel_title;
    uint32_t attempts_counter;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(public_flag);
        buffer.write(moderation_flag);
        buffer.write(unknown);
        buffer.write(channel_path);
        buffer.write(channel_title);
        buffer.write(attempts_counter);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        public_flag = buffer.read<uint8_t>();
        moderation_flag = buffer.read<uint8_t>();
        unknown = buffer.read<uint16_t>();
        channel_path = buffer.read<std::string>();
        channel_title = buffer.read<std::string>();
        attempts_counter = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
