// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "swganh_core/simulation/simulation_service_binding.h"

BOOST_PYTHON_MODULE(py_simulation)
{
    docstring_options local_docstring_options(true, true, false);

    exportSimulationService();
}