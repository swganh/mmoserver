// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "MessageLib/messages/update_pvp_status_message.h"

#include <boost/python.hpp>

using namespace boost::python;
using namespace std;
using namespace swganh::messages;

struct UpdatePvpStatusMessageWrapper : UpdatePvpStatusMessage, wrapper<UpdatePvpStatusMessage>
{

};

void exportUpdatePvpStatusMessage()
{
    class_<UpdatePvpStatusMessageWrapper, boost::noncopyable>("UpdatePvpStatusMessage");
}
