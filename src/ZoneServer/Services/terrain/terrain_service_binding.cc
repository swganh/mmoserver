// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "terrain_service_binding.h"

BOOST_PYTHON_MODULE(py_terrain)
{
    boost::python::docstring_options local_docstring_options(true, true, false);

    exportTerrainService();
}