// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct SUIEventNotification : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x092D3564;
    }

    int window_id;
    int event_type;
    std::vector<std::wstring> returnList;
    int update_count;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(window_id);
        buffer.write(event_type);
        buffer.write(returnList.size());
        buffer.write(update_count);
        for(auto& val : returnList)
        {
            buffer.write(val);
        }
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        window_id = buffer.read<int32_t>();
        event_type = buffer.read<int32_t>();
        int count = buffer.read<int32_t>();
        update_count = buffer.read<int32_t>();
        for(int i=0; i < count; ++i)
        {
            returnList.push_back(buffer.read<std::wstring>());
        }
    }
};

}
} // namespace swganh::messages
