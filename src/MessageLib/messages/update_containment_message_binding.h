// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "MessageLib/messages/update_containment_message.h"

#include <boost/python.hpp>

using namespace boost::python;
using namespace std;
using namespace swganh::messages;

struct UpdateContainmentMessageWrapper : UpdateContainmentMessage, wrapper<UpdateContainmentMessage>
{

};

void exportUpdateContainmentMessage()
{
    class_<UpdateContainmentMessageWrapper, boost::noncopyable>("UpdateContainmentMessage");
}
