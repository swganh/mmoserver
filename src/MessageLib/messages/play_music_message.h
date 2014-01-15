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

struct PlayMusicMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 5;
    }
    uint32_t Opcode() const
    {
        return 0x04270D8A;
    }

    std::string music_file; // e.g. "sound/music_id_tent_naboo_loop.snd"
    uint64_t unused_1;
    uint32_t channel;
    uint8_t unused_2;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(music_file);
        buffer.write(unused_1);
        buffer.write(channel);
        buffer.write(unused_2);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        music_file = buffer.read<std::string>();
        unused_1 = buffer.read<uint64_t>();
        channel = buffer.read<uint32_t>();
        unused_2 = buffer.read<uint8_t>();
    }
};

}
} // namespace swganh::messages
