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

struct RequestCategoriesMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0xF898E25F;
    }

    std::string abbreviated_language_locale; // e.g. "en" for English

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(abbreviated_language_locale);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        abbreviated_language_locale = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
