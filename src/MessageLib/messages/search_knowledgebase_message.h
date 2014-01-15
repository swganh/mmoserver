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

struct SearchKnowledgebaseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x962E8B9B;
    }

    std::wstring search_text;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(search_text);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        search_text = buffer.read<std::wstring>();
    }
};

}
} // namespace swganh::messages
