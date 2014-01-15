// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "service_manager.h"

#include "anh/logger.h"

#include <algorithm>
#include <string>
#include <stdexcept>

#include "service_directory_interface.h"
#include "service_description.h"

#ifndef WIN32
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_search;
#else
#include <regex>
using std::regex;
using std::regex_search;
#endif

using namespace swganh::service;
using namespace std;

ServiceManager::ServiceManager(ServiceDirectoryInterface* service_directory)
    : service_directory_(service_directory)
{}

ServiceManager::~ServiceManager()
{
	auto it = services_.begin();
    for(it = services_.begin();it != services_.end();it++)
    {
		
		service_directory_->removeService((it)->second->GetServiceDescription());
    }
}

ServiceInterface* ServiceManager::GetService(string name)
{
    auto it = services_.find(name);

    if (it == services_.end())
    {
        return nullptr;
    }

    return it->second.get();
}

void ServiceManager::AddService(string name, shared_ptr<ServiceInterface> service)
{
    auto current_service = GetService(name);

    if (current_service)
    {
        // throw exception here, service already exists
        throw std::runtime_error("service already exists: " + name);
        return;
    }

    auto& service_description = service->GetServiceDescription();
    if (!service_directory_->registerService(service_description))
    {
        throw std::runtime_error("Unable to register service " + service_description.name());
    }

    // update the status of the service
    service_description.status(swganh::service::Galaxy::LOADING);
    service_directory_->updateService(service_description);

    services_[name] = service;
}

void ServiceManager::Start()
{
    // First pass initialization
//for (auto& service_entry : services_)

	auto it = services_.begin();
    for(it = services_.begin();it != services_.end();it++)
    {
        auto& service = it->second;
        service->Initialize();
    }

    // After all resources are acquired services can be started
	//for (auto& service_entry : services_)
    //{
	it = services_.begin();
    for(it = services_.begin();it != services_.end();it++)
    {

        auto& service = it->second;

        service->Startup();
        service->SetStatus(Galaxy::ONLINE);

        auto service_description = service->GetServiceDescription();
        service_directory_->updateService(service_description);
        LOG(info) << "Started " << service_description.name();
    }
}

void ServiceManager::Stop()
{
//for (auto& service_entry : services_)
	auto it = services_.begin();
    for(it = services_.begin();it != services_.end();it++)
    {
        auto& service = it->second;

        service->Shutdown();
        service->SetStatus(Galaxy::OFFLINE);

        auto service_description = service->GetServiceDescription();
        service_directory_->updateService(service_description);
        LOG(info) << "Stopped " << service_description.name();
    }
}
