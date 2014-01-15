// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <boost/program_options/options_description.hpp>
#include "anh/service/service_description.h"

namespace swganh {
namespace service {

class ServiceInterface {
public:
    virtual ~ServiceInterface() {}

    virtual ServiceDescription& GetServiceDescription() = 0;

    virtual void SetStatus(Galaxy::StatusType status) = 0;

    /**
     * @brief Initializes all internal state. Only requests for other
     * services and internal plugin implentations should go here.
     */
    virtual void Initialize() {}

    /*
    * @brief Starts up the service, sets running_ to true
    */
    virtual void Startup() {}
    /*
    * @brief Stops the service, sets running_ to false
    */
    virtual void Shutdown() {}

};

class BaseService : public ServiceInterface
{
public:
    virtual ~BaseService() {}

    virtual ServiceDescription& GetServiceDescription() { return service_description_; }
    void SetServiceDescription(ServiceDescription service_description) { service_description_ = service_description; }
    void SetStatus(Galaxy::StatusType status) {
        service_description_.status(status);
    }

private:
    ServiceDescription service_description_;
};

}}  // namespace swganh::service