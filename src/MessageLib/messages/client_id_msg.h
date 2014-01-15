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

struct ClientIdMsg : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xD5899226;
    }

    std::string session_hash;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<uint32_t>(0);
        buffer.write<uint32_t>(0);
        buffer.write(session_hash);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        buffer.read<uint32_t>();  // unknown
        buffer.read<uint32_t>();  // size @TODO Investigate if this is proper usage.
        session_hash = buffer.read<std::string>();
    }
};

}
}  // namespace swganh::messages
