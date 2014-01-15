// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "MessageLib/messages/update_transform_with_parent_message.h"

#include <boost/python.hpp>

using namespace boost::python;
using namespace std;
using namespace swganh::messages;

struct UpdateTransformWithParentMessageWrapper : UpdateTransformWithParentMessage, wrapper<UpdateTransformWithParentMessage>
{

};
void exportUpdateTransformWithParentMessage()
{
    class_<UpdateTransformWithParentMessageWrapper, boost::noncopyable>("UpdateTransformWithParentMessage");
}

