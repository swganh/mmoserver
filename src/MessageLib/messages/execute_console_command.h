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

struct ExecuteConsoleCommand : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0xB1CFCE1C;
    }

    std::string command;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(command);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        command = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
