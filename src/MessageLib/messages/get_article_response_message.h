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

struct GetArticleResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x934BAEE0;
    }

    uint32_t article_text_flag; // 0 = display the article text, 1 = article unable to be retrieved
    std::wstring article_text;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(article_text_flag);
        buffer.write(article_text);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        article_text_flag = buffer.read<uint32_t>();
        article_text = buffer.read<std::wstring>();
    }
};

}
} // namespace swganh::messages
