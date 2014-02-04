// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#ifndef WIN32
#include <Python.h>
#endif

#include "anh/logger.h"
#include "anh/scripting/python_shared_ptr.h"

#include <memory>
#include <boost/python.hpp>
#include <boost/python/overloads.hpp>

#include "ham_service_interface.h"

using namespace swganh::ham;
using namespace boost::python;



void exportHamService()
{
    class_<HamServiceInterface, std::shared_ptr<HamServiceInterface>, boost::noncopyable>("HamService", "The ham service allows the manipulation of creatures ham.", no_init)
    //.def("getHeight", &TerrainServiceInterface::GetHeight, GetHeightOverload(args("scene_id", "x", "z", "raw"), "Returns the height of a particular x, z coordinate"))
    //.def("getWaterHeight", &TerrainServiceInterface::GetWaterHeight, GetWaterHeightOverload(args("scene_id", "x", "z", "raw"), "Returns the water height at a particular x, z coordinate. This is probably not necessary for external code."))
    //.def("isWater", &TerrainServiceInterface::IsWater, IsWaterOverload(args("scene_id", "x", "z", "raw"), "Returns true iff the x, z is water."))
	;
}
