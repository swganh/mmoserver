// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct LoginClientId : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x41131F96;
    }

    std::string username;
    std::string password;
    std::string client_version; // <year><month><day>-<hour>:<minute> for publish, e.g. Publish 13.0 = 20050125-12:19

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(username);
        buffer.write(password);
        buffer.write(client_version);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        username = buffer.read<std::string>();
        password = buffer.read<std::string>();
        client_version = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
