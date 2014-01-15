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

struct ServerTimeMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x2EBC3BD9;
    }

    uint64_t galactic_time; // number of seconds from server start to present galactic time

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(galactic_time);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        galactic_time = buffer.read<uint64_t>();
    }
};

}
} // namespace swganh::messages
