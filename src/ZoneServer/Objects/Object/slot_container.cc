// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "slot_container.h"
#include "object.h"

using namespace swganh::object;

Object* SlotContainer::insert_object( Object* insertObject)
{
    held_objects_.insert(insertObject);
    return nullptr;
}

void SlotContainer::remove_object(Object* removeObject)
{
    auto itr = held_objects_.find(removeObject);
    if(itr != held_objects_.end())
    {
        held_objects_.erase(itr);
    }
}

void SlotContainer::view_objects(std::function<void( Object*)> walkerFunction)
{
    for_each(held_objects_.begin(), held_objects_.end(), walkerFunction);
}
void SlotContainer::view_objects_if(std::function<bool(Object*)> walkerFunction)
{
    for(auto& v = held_objects_.begin(); v != held_objects_.end(); v++)
    {
        if (walkerFunction(*v))
            return;
    }
}

uint16 SlotContainer::get_size()
{
	return held_objects_.size();
}

