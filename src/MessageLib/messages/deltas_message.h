// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include "anh/byte_buffer.h"
#include "base_deltas_message.h"

namespace swganh
{
namespace messages
{

struct DeltasMessage : public BaseDeltasMessage
{
    const static uint32_t opcode = 0x12862153;

    uint16_t Opcount() const
    {
        return 5;
    }
    uint32_t Opcode() const
    {
        return opcode;
    }
};

}
}  // namespace swganh::messages
