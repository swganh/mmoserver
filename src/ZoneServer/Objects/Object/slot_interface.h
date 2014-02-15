// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <memory>
#include <functional>
#include "Utils\typedefs.h"

class Object;

namespace swganh
{
namespace object
{

class SlotInterface
{
public:
    virtual ~SlotInterface() {}
    virtual Object* insert_object( Object* insertObject) = 0;
    virtual void remove_object(Object* removeObject) = 0;
    virtual void view_objects(std::function<void( Object*)> walkerFunction) = 0;
    virtual void view_objects_if(std::function<bool(Object*)> walkerFunction) = 0;
    virtual bool is_filled() = 0;
	virtual uint16 get_size() = 0;
};

}
}