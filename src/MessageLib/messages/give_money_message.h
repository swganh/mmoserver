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

struct GiveMoneyMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0xD1527EE8;
    }

    uint32_t credit_amount;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(credit_amount);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        credit_amount = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
