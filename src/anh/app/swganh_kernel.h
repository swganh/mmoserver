// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include "anh/app/kernel_interface.h"

namespace swganh {
namespace event_dispatcher {
    class EventDispatcher;
}}  // namespace anh::event_dispatcher
//using namespace swganh;

namespace swganh {
namespace database	{
	class DatabaseConfig;
}

namespace tre {
    class TreArchive;
    class ResourceManager;
}}  // namespace swganh::tre

class MessageDispatch;

namespace swganh {
namespace app {

/*!
 * @Brief Contains information about the app config"
 */

struct AppConfig {
	
	std::string					plugin_directory;
	std::vector<std::string>	plugins;

	uint16_t					galaxy_id;
	std::string					bind_address;
    uint16_t					bind_port;

	uint32_t	service_message_heap;
	uint32_t	global_message_heap;
	
	std::vector<std::string> scenes;
    std::string script_directory;
    std::string galaxy_name;
	std::string zone_name;
    std::string tre_config;
    uint32_t resource_cache_size;
    
	uint32_t io_threads;
	uint32_t cpu_threads;
	uint32_t db_threads;

    /*!
    * @Brief Contains information about the database config"
    */

    struct DatabaseConfig {
        std::string server;
		uint16_t	db_port;
        std::string db_schema;
        std::string username;
        std::string password;
		uint16_t	min_thread;
		uint16_t	max_thread;

		std::string global_schema;
		std::string galaxy_schema;
		std::string config_schema;

    } swganh_db;

	struct NetLayerConfig {
		uint16_t	reliable_server_server;
		uint16_t	unreliable_server_server;
		uint16_t	reliable_server_client;
		uint16_t	unreliable_server_client;
		uint32_t	server_packet_window;
		uint32_t	client_packet_window;
		uint32_t	udp_buffer;
	}swganh_netlayer;

    /*!
    * @Brief Contains information about the Login config"
     */
/*
    struct LoginConfig {
        std::string listen_address;
        uint16_t listen_port;
        int galaxy_status_check_duration_secs;
        int login_error_timeout_secs;
        bool login_auto_registration;
    } login_config;
    /*!
    * @Brief Contains information about the app config"
    */

    struct ConnectionConfig {
        std::string listen_address;
        uint16_t listen_port;
        uint16_t ping_port;
    } connection_config;

    boost::program_options::options_description BuildConfigDescription();
};
    
class SwganhKernel : public swganh::app::KernelInterface {
public:
    explicit SwganhKernel(boost::asio::io_service& io_pool, boost::asio::io_service& cpu_pool);
    virtual ~SwganhKernel();

	void Shutdown();

    const swganh::app::Version& GetVersion();

    AppConfig& GetAppConfig();

    swganh::database::Database* GetDatabase();

	void	SetDatabase(swganh::database::Database* database);

	MessageDispatch* GetDispatch();

	void	SetDispatch(MessageDispatch* database);
    
    swganh::event_dispatcher::EventDispatcher* GetEventDispatcher();

    swganh::plugin::PluginManager* GetPluginManager();

    swganh::service::ServiceManager* GetServiceManager();
    
    swganh::service::ServiceDirectoryInterface* GetServiceDirectory();
    
    boost::asio::io_service& GetIoThreadPool();

	boost::asio::io_service& GetCpuThreadPool();

    swganh::tre::ResourceManager* GetResourceManager();

private:
    SwganhKernel();
    swganh::app::Version version_;
    swganh::app::AppConfig app_config_;
    
	std::unique_ptr<MessageDispatch> message_dispatch_;
    std::unique_ptr<swganh::database::Database> database_manager_;
	std::unique_ptr<swganh::event_dispatcher::EventDispatcher> event_dispatcher_;
    std::unique_ptr<swganh::plugin::PluginManager> plugin_manager_;
    std::unique_ptr<swganh::service::ServiceManager> service_manager_;
    std::unique_ptr<swganh::service::ServiceDirectoryInterface> service_directory_;
    std::unique_ptr<swganh::tre::ResourceManager> resource_manager_;

    boost::asio::io_service &io_pool_, &cpu_pool_;
};

}}  // namespace swganh::app
