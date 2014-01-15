// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"

#include "MessageLib/messages/obj_controller_message.h"

namespace swganh
{
namespace messages
{
namespace controllers
{

enum FlyTextColor
{
    RED = 0,
    GREEN,
    BLUE,
    WHITE,
    MIX
};

class ShowFlyText : public ObjControllerMessage
{
public:
    explicit ShowFlyText(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
    {}

    ShowFlyText(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x000001BD;
    }

    uint64_t object_id;
    std::string stf_location; // string file without extention ie: combat_effects
    std::string text; // text to display ie: center_start_fly
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t display_flag; // when true text is displayed in chat history

    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(object_id);
        buffer.write(stf_location);
        buffer.write<uint32_t>(0);
        buffer.write(text);
        buffer.write<uint32_t>(0);
        buffer.write(red);
        buffer.write(green);
        buffer.write(blue);
        buffer.write(display_flag);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        object_id = buffer.read<uint64_t>();
        //buffer.read<uint16_t>();
        stf_location = buffer.read<std::string>();
        buffer.read<uint32_t>();
        text = buffer.read<std::string>();
        buffer.read<uint32_t>();
        red = buffer.read<uint8_t>();
        green = buffer.read<uint8_t>();
        blue = buffer.read<uint8_t>();
        display_flag = buffer.read<uint8_t>();
    }
};

}
}
}  // namespace swganh::messages::controllers
