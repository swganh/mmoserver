// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once
//#include "MessageLib\messages\base_swg_message.h"
#include "MessageLib\messages\baselines_message.h"
#include "MessageLib\messages\deltas_message.h"

//#include "Utils/typedefs.h"
//#include <string>


namespace swganh
{
namespace object
{

class EquipmentNotFound : public std::runtime_error
{
public:
    EquipmentNotFound()
        : runtime_error("Equipment not found")
    {
    }
};

struct EquipmentItem
{
    EquipmentItem()
    {
    }

    EquipmentItem(uint64_t object_id_, uint32_t template_crc_ = 0, std::string customization_ = std::string(""), uint32_t containment_type_ = 4)
        : customization(customization_)
        , containment_type(containment_type_)
        , object_id(object_id_)
        , template_crc(template_crc_)
    {
    }

    ~EquipmentItem()
    {
    }

    void Serialize(swganh::messages::BaselinesMessage& message)
    {
        message.data.write<std::string>(customization);
        message.data.write<uint32_t>(containment_type);
        message.data.write<uint64_t>(object_id);
        message.data.write<uint32_t>(template_crc);
    }

    void Serialize(swganh::messages::DeltasMessage& message)
    {
        message.data.write<std::string>(customization);
        message.data.write<uint32_t>(containment_type);
        message.data.write<uint64_t>(object_id);
        message.data.write<uint32_t>(template_crc);
    }

    bool operator==(const EquipmentItem& other)
    {
        return (object_id != other.object_id);
    }

    std::string customization;
    uint32_t containment_type;
    uint64_t object_id;
    uint32_t template_crc;
};

}
}