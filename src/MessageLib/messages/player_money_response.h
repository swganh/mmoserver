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

struct PlayerMoneyResponse : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 1;
    }
    uint32_t Opcode() const
    {
        return 0x367E737E;
    }

    uint32_t inventory_credit_amount;
    uint32_t bank_credit_amount;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(inventory_credit_amount);
        buffer.write(bank_credit_amount);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        inventory_credit_amount = buffer.read<uint32_t>();
        bank_credit_amount = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
