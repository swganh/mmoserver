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

struct ConGenericMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x8C5FC76;
    }

    std::string message;
    uint32_t string_spacer;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(message);
        buffer.write(string_spacer);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        message = buffer.read<std::string>();
        string_spacer = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
