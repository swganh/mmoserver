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

struct ErrorMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xB5ABF91A;
    }

    std::string type;
    std::string message;
    bool force_fatal;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(type);
        buffer.write(message);
        buffer.write(force_fatal);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        type = buffer.read<std::string>();
        message = buffer.read<std::string>();
        force_fatal = buffer.read<bool>();
    }
};

}
} // namespace swganh::messages
