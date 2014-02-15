// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "ZoneServer\Services\equipment\equipment_service_interface.h"

namespace swganh
{
namespace tre
{
class ResourceManager;
class SlotDefinitionVisitor;
}
}

namespace swganh
{
namespace equipment
{
/**
* A service that provides wrapping functionality for the container system
* Methods on this interface basically convert slot name to slot id
*/
class EquipmentService : public swganh::equipment::EquipmentServiceInterface
{
public:
    /**
    * Creates a new instance
    */
    EquipmentService(swganh::tre::ResourceManager* resourceManager);

    ~EquipmentService();

    virtual void Initialize();

    /**
    * @return the id of a slot by name
    */
    virtual int32_t GetSlotIdByName(std::string slot_name);

    /**
    * @return the name of a slot by id
    */
    virtual std::string GetSlotNameById(int32_t slot_id);

    /**
    * Clears a slot for an object by name
    * @param object the object to manipulate
    * @param slot_name the name of the slot to clear
    */
    virtual void ClearSlot(Object* object, std::string slot_name);

    /**
    * @param object the object to search
    * @param slot_name the name of the slot to return
    * @returns a sub object of object by slot name
    */
    virtual Object* GetEquippedObject(
        Object* object, std::string slot_name);

private:
    std::shared_ptr<swganh::tre::SlotDefinitionVisitor> slot_definitions_;
};
}
}