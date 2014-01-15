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

struct OpenedContainerMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x2E11E4AB;
    }

    uint32_t unknown1;
    uint64_t container_object_id;
    std::string container_slot;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(unknown1);
        buffer.write(container_object_id);
        buffer.write(container_slot);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        unknown1 = buffer.read<uint32_t>();
        container_object_id = buffer.read<uint64_t>();
        container_slot = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
