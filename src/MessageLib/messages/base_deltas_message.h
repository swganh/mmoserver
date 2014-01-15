// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <iostream>

#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct BaseDeltasMessage : public BaseSwgMessage
{
    uint64_t object_id;
    uint32_t object_type;
    uint8_t view_type;
    uint16_t update_count;
    uint16_t update_type;
    swganh::ByteBuffer data;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(object_id);
        buffer.write(object_type);
        buffer.write(view_type);
        
		//this is actually hardcoded shit - the size is 
		//+2 for update count and +2 for every different member descriptor
		//to their defense with the current setup there can only be one member per delta
		buffer.write<uint32_t>(data.size() + 4);

        buffer.write<uint16_t>(update_count);
        buffer.write<uint16_t>(update_type);
        buffer.write(data.data(), data.size());
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        object_id = buffer.read<uint32_t>();
        object_type = buffer.read<uint32_t>();
        view_type = buffer.read<uint32_t>();
        uint32_t data_size = buffer.read<uint32_t>() - 4;
        update_count = buffer.read<uint16_t>();
        update_type = buffer.read<uint16_t>();
        data = swganh::ByteBuffer(buffer.data() + buffer.read_position(), data_size);
    }
};

}
}  // namespace swganh::messages
