// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "ham_init.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include "anh/logger.h"

#include "anh/plugin/bindings.h"
#include "anh/plugin/plugin_manager.h"

#include "anh/app/swganh_kernel.h"

#include "ham_service.h"

#include "version.h"

using swganh::plugin::ObjectRegistration;

namespace swganh
{
namespace ham
{

void Initialize(swganh::app::SwganhKernel* kernel)
{
    swganh::plugin::ObjectRegistration registration;
    registration.version.major = VERSION_MAJOR;
    registration.version.minor = VERSION_MINOR;

    // Register
    registration.CreateObject = [kernel] (swganh::plugin::ObjectParams* params) -> void *
    {
        return new HamService(kernel);
    };

    registration.DestroyObject = [] (void* object)
    {
        if (object)
        {
            delete static_cast<HamService*>(object);
        }
    };

    kernel->GetPluginManager()->RegisterObject("Ham::HamService", &registration);

}

}
}  // namespace swganh::galaxy
