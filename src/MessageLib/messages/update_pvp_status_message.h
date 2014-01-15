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

struct UpdatePvpStatusMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x08A1C126;
    }

    uint32_t pvp_status;
    uint32_t faction;
    uint64_t object_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(pvp_status);
        buffer.write(faction);
        buffer.write(object_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        pvp_status = buffer.read<uint32_t>();
        faction = buffer.read<uint32_t>();
        object_id = buffer.read<uint64_t>();
    }
};

}
}  // namespace swganh::messages

