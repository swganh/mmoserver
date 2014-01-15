// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#ifndef WIN32
#include <Python.h>
#endif

#include "swganh_kernel.h"

#include "anh/scripting/python_shared_ptr.h"

#include "anh/scripting/python_event.h"

#include <boost/python.hpp>

void exportPythonEvent()
{
    using boost::python::class_;
    using boost::python::bases;
    using boost::python::implicitly_convertible;

    class_<swganh::event_dispatcher::EventInterface, std::shared_ptr<swganh::event_dispatcher::EventInterface>, boost::noncopyable>("EventInterface", no_init)
        ;

    class_<swganh::event_dispatcher::BaseEvent, bases<swganh::event_dispatcher::EventInterface>, std::shared_ptr<swganh::event_dispatcher::BaseEvent>, boost::noncopyable>("BaseEvent", no_init)
        .def("type", &swganh::scripting::PythonEvent::Type, "Returns the type of the event")
        ;

    implicitly_convertible<std::shared_ptr<swganh::event_dispatcher::BaseEvent>, std::shared_ptr<swganh::event_dispatcher::EventInterface>>();

    class_<swganh::scripting::PythonEvent, bases<swganh::event_dispatcher::BaseEvent>, std::shared_ptr<swganh::scripting::PythonEvent>, boost::noncopyable>("PythonEvent", init<boost::python::object, float>())
        .def_readwrite("callback", &swganh::scripting::PythonEvent::callback, "callback to set that will finish the event")
        .def_readwrite("timer", &swganh::scripting::PythonEvent::timer, "timer that determines when the event will finish")
        ;

    implicitly_convertible<std::shared_ptr<swganh::scripting::PythonEvent>, std::shared_ptr<swganh::event_dispatcher::BaseEvent>>();
}
