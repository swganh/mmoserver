// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "ZoneServer\Objects\Object\slot_interface.h"
//#include "swganh_core/object/slot_interface.h"

#include <set>

namespace swganh
{
namespace object
{

class SlotContainer : public SlotInterface
{
public:


    virtual Object* insert_object( Object* insertObject);
    virtual void remove_object(Object* removeObject);
    virtual bool is_filled()
    {
        return false;
    }
    virtual void view_objects(std::function<void( Object*)> walkerFunction);
    virtual void view_objects_if(std::function<bool(Object*)> walkerFunction);

	virtual uint16 get_size();

private:
    std::set<Object*> held_objects_;
};

}
}
