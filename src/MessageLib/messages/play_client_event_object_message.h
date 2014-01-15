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

struct PlayClientEventObjectMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0xAF83C3F2;
    }

    std::string event_string;
    std::string hardpoint_string;
    uint64_t object_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(event_string);
        buffer.write(hardpoint_string);
        buffer.write(object_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        event_string = buffer.read<std::string>();
        hardpoint_string = buffer.read<std::string>();
        object_id = buffer.read<uint64_t>();
    }
};

}
} // namespace swganh::messages
