// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include <anh/service/service_directory.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <anh/event_dispatcher/event_dispatcher.h>
#include <anh/service/datastore.h>

using namespace swganh::service;
using namespace std;

ServiceDirectory::ServiceDirectory(
    shared_ptr<DatastoreInterface> datastore,
    swganh::event_dispatcher::EventDispatcher* event_dispatcher)
    : datastore_(datastore)
{}

ServiceDirectory::ServiceDirectory(
    shared_ptr<DatastoreInterface> datastore,
    swganh::event_dispatcher::EventDispatcher* event_dispatcher,
    const string& galaxy_name,
    const string& version ,
    bool create_galaxy)
    : datastore_(datastore)
{
    //joinGalaxy(galaxy_name, version, create_galaxy);
}

Galaxy ServiceDirectory::galaxy() const
{
    return active_galaxy_;
}

ServiceDescription ServiceDirectory::service() const
{
    return active_service_;
}

void ServiceDirectory::joinGalaxy(const std::string& galaxy_name, const std::string& version, bool create_galaxy)
{
    auto galaxy = datastore_->findGalaxyByName(galaxy_name);

    if (!galaxy)
    {
        // if no galaxy was found and no request to create it was made, fail now
        if (! create_galaxy)
        {
            throw InvalidGalaxyError(std::string("Attempted to join an invalid galaxy: ").append(galaxy_name));
        }

        if (!(galaxy = datastore_->createGalaxy(galaxy_name, version)))
        {
            throw InvalidGalaxyError(std::string("Attempt to create galaxy failed: ").append(galaxy_name));
        }
    }

    boost::lock_guard<boost::mutex> lk(mutex_);
    active_galaxy_ = *galaxy;
}
void ServiceDirectory::updateGalaxyStatus()
{
    auto services = getServiceSnapshot(active_galaxy_);

    if (services.empty())
    {
        return;
    }
    uint32_t offline_count = 0;
    uint32_t loading_count = 0;
    uint32_t online_count = 0;
    service::Galaxy::StatusType galaxy_status;
    for_each(services.begin(), services.end(), [&] (swganh::service::ServiceDescription& service)
    {
        if (service.status() == service::Galaxy::OFFLINE)
        {
            offline_count++;
        }
        else if(service.status() == service::Galaxy::LOADING)
        {
            loading_count++;
        }
        else if(service.status() == service::Galaxy::ONLINE)
        {
            online_count++;
        }
    });
    if (online_count == services.size())
    {
        galaxy_status = service::Galaxy::ONLINE;
    }
    else if (offline_count == services.size())
    {
        galaxy_status = service::Galaxy::OFFLINE;
    }
    else
    {
        galaxy_status = service::Galaxy::LOADING;
    }

    uint32_t galaxy_id;

    {
        boost::lock_guard<boost::mutex> lk(mutex_);
        active_galaxy_.status((Galaxy::StatusType)galaxy_status);
        galaxy_id = active_galaxy_.id();
    }

    datastore_->saveGalaxyStatus(galaxy_id, galaxy_status);
}

bool ServiceDirectory::registerService(ServiceDescription& service)
{
    if (datastore_->createService(active_galaxy_, service))
    {
        {
            boost::lock_guard<boost::mutex> lk(mutex_);
            active_service_ = service;
        }
        return true;
    }

    return false;
}

bool ServiceDirectory::removeService(const ServiceDescription& service)
{
    if (datastore_->deleteServiceById(service.id()))
    {
        return true;
    }

    return false;
}

void ServiceDirectory::updateService(const ServiceDescription& service)
{
    boost::lock_guard<boost::mutex> lk(mutex_);
    datastore_->saveService(service);
}

void ServiceDirectory::updateServiceStatus(int32_t new_status)
{
    boost::lock_guard<boost::mutex> lk(mutex_);

    active_service_.status(new_status);
    datastore_->saveService(active_service_);
}

bool ServiceDirectory::makePrimaryService(const ServiceDescription& service)
{
    boost::lock_guard<boost::mutex> lk(mutex_);
    active_galaxy_.primary_id(service.id());
    return true;
}

void ServiceDirectory::pulse()
{
    boost::lock_guard<boost::mutex> lk(mutex_);

    if (active_service_.id())
    {
        std::string last_pulse = "";
        if (active_galaxy_.id() && active_galaxy_.primary_id() != active_service_.id())
        {
            last_pulse = getGalaxyTimestamp_();
        }
        else
        {
            last_pulse = boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time());
        }

        active_service_.last_pulse(last_pulse);
        datastore_->saveService(active_service_);
    }

    if (active_galaxy_.id())
    {
        auto galaxy = datastore_->findGalaxyById(active_galaxy_.id());
        active_galaxy_ = *galaxy;
    }
}

GalaxyList ServiceDirectory::getGalaxySnapshot()
{
    boost::lock_guard<boost::mutex> lk(mutex_);

    return datastore_->getGalaxyList();
}

ServiceList ServiceDirectory::getServiceSnapshot(const Galaxy& galaxy)
{
    boost::lock_guard<boost::mutex> lk(mutex_);

    return datastore_->getServiceList(galaxy.id());
}

std::string ServiceDirectory::getGalaxyTimestamp_()
{
    auto service = datastore_->findServiceById(active_galaxy_.primary_id());

    if (!service)
    {
        return boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time());
    }

    return service->last_pulse();
}
