// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "service_interface.h"

namespace swganh {
namespace database {
    class DatabaseManagerInterface;
}}  // namespace swganh::database

namespace swganh {
namespace service {

class ServiceDirectoryInterface;

class ServiceManager {
public:
    explicit ServiceManager(ServiceDirectoryInterface* service_directory);
    
    ~ServiceManager();

    void AddService(std::string name, std::shared_ptr<ServiceInterface> service);

    ServiceInterface* GetService(std::string name);

    template<typename T>
    T* GetService(std::string name)
    {
        T* service;

        auto tmp = GetService(name);

#ifdef _DEBUG
        service = dynamic_cast<T*>(tmp);
#else
        service = static_cast<T*>(tmp);
#endif

        return service;
    }

    template<typename T>
    std::vector<T*> GetServicesByType(std::string type_name)
    {
        std::vector<T*> services;
        T* tmp;

        for(auto& entry : services_)
        {
            swganh::service::ServiceDescription description = entry.second->GetServiceDescription();

            if (description.type().compare(type_name) == 0) 

            {
#ifdef _DEBUG
                tmp = dynamic_cast<T>(entry.second.get());
#else
                tmp = static_cast<T>(entry.second.get());
#endif
                services.push_back(tmp);
            }
        }

        return services;
    }

    // add start/stop services, all and individually
    void Start();
    void Stop();

private:
    typedef std::map<std::string, std::shared_ptr<ServiceInterface>> ServiceMap;
    ServiceMap services_;

    ServiceDirectoryInterface* service_directory_;
};
    
}}  // namespace swganh::service
