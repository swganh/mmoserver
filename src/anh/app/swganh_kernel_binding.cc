// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "swganh_kernel_binding.h"
#include "swganh_event_binding.h"

BOOST_PYTHON_MODULE(py_app)
{
    docstring_options local_docstring_options(true, true, false);

    exportSWGANHKernel();
    exportEventDispatcher();
    exportPythonEvent();
}
