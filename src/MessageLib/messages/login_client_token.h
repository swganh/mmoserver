// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace login
{
class LoginClientInterface;
}
} // namespace swganh::login

namespace swganh
{
namespace messages
{

struct LoginClientToken : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0xAAB296C6;
    }

    swganh::ByteBuffer session_key;
    uint32_t station_id;
    std::string station_username;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<uint32_t>(session_key.size());
        buffer.append(session_key);
        buffer.write(station_id);
        buffer.write(station_username);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        session_key = swganh::ByteBuffer(buffer.data(), buffer.read<uint32_t>());
        buffer.read_position(buffer.read_position() + session_key.size());
        station_id = buffer.read<int32_t>();
        station_username = buffer.read<std::string>();
    }
};

LoginClientToken BuildLoginClientToken(std::shared_ptr<swganh::login::LoginClientInterface> login_client, const std::string& session_key);

}
} // namespace swganh::messages
