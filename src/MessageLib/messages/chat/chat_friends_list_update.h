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

struct ChatFriendsListUpdate : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x6CD2FCD8;
    }

    std::string game_name_; // default: SWG
    std::string server_name_; // galaxy name
    std::string friend_name_;
    uint8_t status_flag_; // 0 = offline, 1 = online

    ChatFriendsListUpdate(std::string server_name, std::string friend_name, uint8_t status)
        : game_name_("SWG")
        , server_name_(server_name)
        , friend_name_(friend_name)
        , status_flag_(status)
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(game_name_);
        buffer.write(server_name_);
        buffer.write(friend_name_);
        buffer.write(status_flag_);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        game_name_ = buffer.read<std::string>();
        server_name_ = buffer.read<std::string>();
        friend_name_ = buffer.read<std::string>();
        status_flag_ = buffer.read<uint8_t>();
    }
};

}
} // namespace swganh::messages
