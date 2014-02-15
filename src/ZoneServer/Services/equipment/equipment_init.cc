// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "equipment_init.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include "anh/logger.h"

#include "anh/plugin/bindings.h"
#include "anh/plugin/plugin_manager.h"

#include "anh/app/swganh_kernel.h"

#include "equipment_service.h"

#include "version.h"

namespace swganh
{
namespace equipment
{

void Initialize(swganh::app::SwganhKernel* kernel)
{
    swganh::plugin::ObjectRegistration registration;
    registration.version.major = VERSION_MAJOR;
    registration.version.minor = VERSION_MINOR;

    // Register
    registration.CreateObject = [kernel] (swganh::plugin::ObjectParams* params) -> void *
    {
        return new EquipmentService(kernel->GetResourceManager());
    };

    registration.DestroyObject = [] (void * object)
    {
        if (object)
        {
            delete static_cast<EquipmentService*>(object);
        }
    };

    kernel->GetPluginManager()->RegisterObject("Equipment::EquipmentService", &registration);

}

}
}  // namespace swganh::galaxy
