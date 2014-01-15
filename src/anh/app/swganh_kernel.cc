// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "anh/app/swganh_kernel.h"
#include "anh/app/kernel_interface.h"

#include <mysql_driver.h>
#include <cppconn/connection.h>
#include <cppconn/driver.h>

#include "databasemanager/database.h"
#include "databasemanager/DatabaseManager.h"

#include "anh/event_dispatcher/event_dispatcher.h"
#include "anh/plugin/plugin_manager.h"
#include "anh/service/datastore.h"
#include "anh/service/service_directory.h"
#include "anh/service/service_manager.h"
#include "NetworkManager/MessageDispatch.h"

#include "anh/tre/resource_manager.h"

#include "version.h"

using namespace swganh::service;
using namespace swganh::app;

using swganh::app::Version;
using swganh::database::Database;
using swganh::database::DatabaseManager;
using swganh::database::DatabaseConfig;
using swganh::plugin::PluginManager;
using swganh::service::ServiceManager;

using std::make_shared;
using std::shared_ptr;

SwganhKernel::SwganhKernel(boost::asio::io_service& io_pool, boost::asio::io_service& cpu_pool)
    : io_pool_(io_pool)
    , cpu_pool_(cpu_pool)
{
    version_.major = VERSION_MAJOR;
    version_.minor = VERSION_MINOR;

    plugin_manager_ = nullptr;
    service_manager_ = nullptr;
}

SwganhKernel::~SwganhKernel()
{

}

void SwganhKernel::Shutdown()
{
    service_manager_->Stop();
    exit(0);

    //event_dispatcher_->Shutdown();

    resource_manager_.reset();
    event_dispatcher_.reset();
    service_manager_.reset();
    service_directory_.reset();
    plugin_manager_.reset();
}

const Version& SwganhKernel::GetVersion()
{
    return version_;
}

AppConfig& SwganhKernel::GetAppConfig()
{
    return app_config_;
}

Database* SwganhKernel::GetDatabase()
{
    if (!database_manager_)
    {
		return nullptr;
		//database_manager_.reset(new DatabaseManager(database_config_));
    }

    return database_manager_.get();
}

void SwganhKernel::SetDatabase(swganh::database::Database* database)
{
	database_manager_.reset(database);
}

MessageDispatch* SwganhKernel::GetDispatch()
{
    if (!message_dispatch_)
    {
		return nullptr;
    }

    return message_dispatch_.get();
}

void SwganhKernel::SetDispatch(MessageDispatch* dispatch)
{
	message_dispatch_.reset(dispatch);
}

swganh::event_dispatcher::EventDispatcher* SwganhKernel::GetEventDispatcher()
{
    if (!event_dispatcher_)
    {
		event_dispatcher_.reset(new swganh::event_dispatcher::EventDispatcher(io_pool_));
    }

    return event_dispatcher_.get();
}

PluginManager* SwganhKernel::GetPluginManager()
{
    if (!plugin_manager_)
    {
        plugin_manager_.reset(new PluginManager(this));
    }

    return plugin_manager_.get();
}

ServiceManager* SwganhKernel::GetServiceManager()
{
    if (!service_manager_)
    {
        service_manager_.reset(new ServiceManager(GetServiceDirectory()));
    }

    return service_manager_.get();
}

ServiceDirectoryInterface* SwganhKernel::GetServiceDirectory()
{
    if (!service_directory_)
    {

		auto data_store = make_shared<Datastore>(static_cast<SwganhKernel*>(this));
        service_directory_.reset(new ServiceDirectory(
                                     data_store,
                                     GetEventDispatcher(),
                                     app_config_.galaxy_name,
                                     GetVersion().ToString(),
                                     true));
									 
    }

    return service_directory_.get();
}


boost::asio::io_service& SwganhKernel::GetIoThreadPool()
{
    return io_pool_;
}

boost::asio::io_service& SwganhKernel::GetCpuThreadPool()
{
    return cpu_pool_;
}


swganh::tre::ResourceManager* SwganhKernel::GetResourceManager()
{
    if (!resource_manager_)
    {
        resource_manager_.reset(new swganh::tre::ResourceManager(std::make_shared<swganh::tre::TreArchive>(GetAppConfig().tre_config)));
    }

    return resource_manager_.get();
}
