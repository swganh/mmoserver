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

struct BaseBaselinesMessage : public BaseSwgMessage
{
    uint64_t object_id;
    uint32_t object_type;
    uint16_t object_opcount;
    uint8_t view_type;
    swganh::ByteBuffer data;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(object_id);
        buffer.write(object_type);
        buffer.write(view_type);

        buffer.write<uint32_t>(data.size() + 2);
        buffer.write<uint16_t>(object_opcount);
        buffer.write(data.data(), data.size());
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        object_id = buffer.read<uint32_t>();
        object_type = buffer.read<uint32_t>();
        view_type = buffer.read<uint32_t>();
        object_opcount = buffer.read<uint16_t>();

        uint32_t data_size = buffer.read<uint32_t>();
        data = swganh::ByteBuffer(buffer.data() + buffer.read_position(), data_size);
    }
};

}
}  // namespace swganh::messages
