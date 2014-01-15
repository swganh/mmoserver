// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"

#include "MessageLib/messages/obj_controller_message.h"
#include "radial_options.h"

namespace swganh
{
namespace messages
{
namespace controllers
{



// Radial Request
class ObjectMenuRequest : public ObjControllerMessage
{
public:
    explicit ObjectMenuRequest(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
    {}

    ObjectMenuRequest(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x00000146;
    }

    uint64_t target_id;
    uint64_t owner_id;
    std::vector<RadialOptions> radial_options;
    uint8_t response_count;

    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(target_id);
        buffer.write(owner_id);

        uint32_t size = radial_options.size();
        if (size > 0)
        {
            int counter = 0;
            for(auto& radial : radial_options)
            {
                buffer.write(++counter);
                buffer.write(radial.parent_item);
                buffer.write(static_cast<uint8_t>(radial.identifier));
                buffer.write(radial.action);
                buffer.write(radial.custom_description);
            }
        }
        else
        {
            buffer.write(0);
        }
        buffer.write(response_count);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        target_id = buffer.read<uint64_t>();
        owner_id = buffer.read<uint64_t>();

        uint32_t size = buffer.read<uint32_t>();
        //int counter = 0;
        for(uint32_t i = 0; i < size ; ++i)
        {
            RadialOptions r;
            buffer.read<uint8_t>();
            r.parent_item = buffer.read<uint8_t>();
            r.identifier =	static_cast<RadialIdentifier>(buffer.read<uint8_t>());
            r.action = buffer.read<uint8_t>();
            r.custom_description = buffer.read<std::wstring>();
            radial_options.push_back(r);
        }
        response_count = buffer.read<uint8_t>();
    }
};

}
}
}  // namespace swganh::messages::controllers
