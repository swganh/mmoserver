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

struct DeleteCharacterMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xE87AD031;
    }

    int32_t server_id;
    uint64_t character_id;

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        server_id = buffer.read<int32_t>();
        character_id = buffer.read<uint64_t>();
    }

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(server_id);
        buffer.write(character_id);
    }
};

}
}  // namespace swganh::messages
