// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "slot_exclusive.h"
#include "object.h"

using namespace swganh::object;

Object* SlotExclusive::insert_object( Object* insertObject)
{
    Object* result = held_object_;
    held_object_ = insertObject;
    return result;
}

void SlotExclusive::remove_object(Object* removeObject)
{
    if(held_object_ == removeObject)
    {
        held_object_ = nullptr;
    }
}

void SlotExclusive::view_objects(std::function<void(Object*)> walkerFunction)
{
    if (held_object_ != nullptr)
        walkerFunction(held_object_);
}

void SlotExclusive::view_objects_if(std::function<bool(Object*)> walkerFunction)
{
    if (held_object_ != nullptr)
        walkerFunction(held_object_);
}

uint16 SlotExclusive::get_size()
{
	return 1;
}