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

struct VerifyPlayerNameMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xBB8CAD45;
    }

    std::wstring name;
    uint32_t attempts_counter;
    uint32_t unknown;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(name);
        buffer.write(attempts_counter);
        buffer.write(unknown);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        name = buffer.read<std::wstring>();
        attempts_counter = buffer.read<uint32_t>();
        unknown = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
