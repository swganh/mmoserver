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

struct EnterStructurePlacementModeMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xE8A54DC1;
    }

    uint64_t deed_id;
    std::string object_file_path; // e.g. "object/building/player/shared_player_garage_corellia_style_01.iff"

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(deed_id);
        buffer.write(object_file_path);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        deed_id = buffer.read<uint64_t>();
        object_file_path = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
