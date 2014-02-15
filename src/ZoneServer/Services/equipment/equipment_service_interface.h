// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include <anh/service/service_interface.h>

class Object;
namespace swganh	{
namespace equipment
{
class EquipmentServiceInterface : public swganh::service::BaseService
{
public:
    virtual ~EquipmentServiceInterface() {}

    virtual int32_t GetSlotIdByName(std::string slot_name) = 0;
    virtual std::string GetSlotNameById(int32_t slot_id) = 0;

    virtual void ClearSlot(Object* object, std::string slot_name) = 0;

    template<typename T>
    T* GetEquippedObject( Object* object, std::string slot_name)
    {
        auto found = GetEquippedObject(object, slot_name);
#ifdef _DEBUG
        return dynamic_cast<T>(found);
#else
        return std::static_cast<T>(found);
#endif
    }

    virtual Object* GetEquippedObject(Object* object, std::string slot_name) = 0;
};
}

}