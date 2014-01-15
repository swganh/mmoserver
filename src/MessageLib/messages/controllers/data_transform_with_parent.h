// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "anh/byte_buffer.h"

#include "MessageLib/messages/obj_controller_message.h"

namespace swganh
{
namespace messages
{
namespace controllers
{

class DataTransformWithParent : public ObjControllerMessage
{
public:
    explicit DataTransformWithParent(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
    {}

    DataTransformWithParent(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x000000F1;
    }
    uint32_t counter;
    uint64_t cell_id;
    glm::quat orientation;
    glm::vec3 position;
    float speed;

    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(counter);
        buffer.write(cell_id);
        buffer.write(orientation.x);
        buffer.write(orientation.y);
        buffer.write(orientation.z);
        buffer.write(orientation.w);
        buffer.write(position.x);
        buffer.write(position.y);
        buffer.write(position.z);
        buffer.write(speed);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        counter = buffer.read<uint32_t>();
        cell_id = buffer.read<uint64_t>();
        orientation.x = buffer.read<float>();
        orientation.y = buffer.read<float>();
        orientation.z = buffer.read<float>();
        orientation.w = buffer.read<float>();
        position.x = buffer.read<float>();
        position.y = buffer.read<float>();
        position.z = buffer.read<float>();
        speed = buffer.read<float>();
    }
};

}
}
}  // namespace swganh::messages::controllers
