// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>

#include <list>
#include <memory>
#include <string>

namespace swganh {
namespace service {

class Galaxy;
class ServiceDescription;

class DatastoreInterface {
public:
    virtual ~DatastoreInterface() {}

    virtual std::shared_ptr<Galaxy> createGalaxy(const std::string& name, const std::string& version) const = 0;
    virtual std::shared_ptr<Galaxy> findGalaxyById(uint32_t id) const = 0;
    virtual std::shared_ptr<Galaxy> findGalaxyByName(const std::string& name) const = 0;
    virtual void saveGalaxyStatus(int32_t galaxy_id, int32_t status) const = 0;
    
    virtual bool createService(const Galaxy& galaxy, ServiceDescription& description) const = 0;
    virtual std::shared_ptr<ServiceDescription> findServiceById(uint32_t id) const = 0;
    virtual bool deleteServiceById(uint32_t id) const = 0;
    virtual void saveService(const ServiceDescription& service) const = 0;

    virtual std::list<Galaxy> getGalaxyList() const = 0;
    virtual std::list<ServiceDescription> getServiceList(uint32_t galaxy_id) const = 0;
    virtual std::string prepareTimestampForStorage(const std::string& timestamp) const = 0;
};

}  // namespace service
}  // namespace swganh
