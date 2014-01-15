// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp> 
#include <boost/thread/lock_guard.hpp> 

#include "anh/service/galaxy.h"
#include "anh/service/service_description.h"
#include "anh/service/service_directory_interface.h"

// Forward Declare
namespace swganh { class EventDispatcher; }

namespace swganh {
namespace event_dispatcher {
	class EventDispatcher;
}
namespace service {

class DatastoreInterface;

class InvalidGalaxyError : public std::runtime_error {
public:
    InvalidGalaxyError(const std::string& message = "") 
        : std::runtime_error(message) {}
};

class InvalidServiceError : public std::runtime_error {
public:
    InvalidServiceError(const std::string& message = "") 
        : std::runtime_error(message) {}
};

/*! \brief ServiceDirectory is a utility class intended to assist servicees in
* registering themselves and participating in a galaxyed environment.
*/
class ServiceDirectory : public ServiceDirectoryInterface{
public:
    ServiceDirectory(std::shared_ptr<DatastoreInterface> datastore, 
        swganh::event_dispatcher::EventDispatcher* event_dispatcher);
    ServiceDirectory(std::shared_ptr<DatastoreInterface> datastore, 
        swganh::event_dispatcher::EventDispatcher* event_dispatcher, const std::string& galaxy_name, 
        const std::string& version = "", bool create_galaxy = false);

    Galaxy galaxy() const;
    ServiceDescription service() const;
        
    void joinGalaxy(const std::string& galaxy_name, const std::string& version = "", bool create_galaxy = false);
    
    void updateGalaxyStatus();

    bool registerService(ServiceDescription& service);
    bool removeService(const ServiceDescription& service);
    void updateService(const ServiceDescription& service);
    void updateServiceStatus(int32_t new_status);
    
    bool makePrimaryService(const ServiceDescription& service);

    void pulse();
    
    GalaxyList getGalaxySnapshot();
    ServiceList getServiceSnapshot(const Galaxy& galaxy);

private:
    std::string getGalaxyTimestamp_();

    std::shared_ptr<DatastoreInterface> datastore_;
    Galaxy active_galaxy_;
    ServiceDescription active_service_;

    boost::mutex mutex_;
};

}  // namespace service
}  // namespace swganh
