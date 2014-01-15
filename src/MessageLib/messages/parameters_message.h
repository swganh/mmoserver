// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ParametersMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x487652DA;
    }

    uint32_t parameter_flag; // default: 900 = decimal, 384 = hex, 1110000100 = binary

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(parameter_flag);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        parameter_flag = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
