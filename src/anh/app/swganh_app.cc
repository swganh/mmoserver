// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#ifndef WIN32
#include <Python.h>
#endif

#include "anh/app/swganh_app.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>

#ifdef WIN32
#include <regex>
#else
#include <boost/regex.hpp>
#endif

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>

#include "anh/logger.h"

#include "DatabaseManager/databasemanager.h"
#include "anh/event_dispatcher/event_dispatcher.h"
#include "anh/plugin/plugin_manager.h"
#include "anh/service/datastore.h"
#include "anh/service/service_manager.h"

#include "anh/app/swganh_kernel.h"

#include "anh/scripting/utilities.h"

#include "version.h"

using namespace boost::asio;
//using namespace boost::thread;
using namespace boost::program_options;
using namespace std;
using namespace swganh;
using namespace swganh::app;

using swganh::plugin::RegistrationMap;

#ifdef WIN32
using std::regex;
using std::smatch;
using std::regex_match;
#else
using boost::regex;
using boost::smatch;
using boost::regex_match;
#endif

options_description AppConfig::BuildConfigDescription()
{
    options_description desc;

    desc.add_options()
    ("help,h", "Display help message and config options")

    //("server_mode", boost::program_options::value<std::string>(&server_mode)->default_value("all"),
     //"Specifies the service configuration mode to run the server in.")

    //("plugin,p", boost::program_options::value<std::vector<std::string>>(&plugins),
     //"Only used when single_server_mode is disabled, loads a module of the specified name")
    
	 //("plugin_directory", value<string>(&plugin_directory)->default_value("plugins/"),
     //"Directory containing the application plugins")

    ("script_directory", value<string>(&script_directory)->default_value("scripts"),
     "Directory containing the application scripts")

    ("tre_config", boost::program_options::value<std::string>(&tre_config),
     "File containing the tre configuration (live.cfg)")

    ("galaxy_name", boost::program_options::value<std::string>(&galaxy_name),
     "Name of the galaxy (cluster) to this process should run")

    ("resource_cache_size", boost::program_options::value<uint32_t>(&resource_cache_size),
     "Available cache size for the resource manager (in Megabytes)")

    ("io_threads", value<uint32_t>(&io_threads)->default_value(0), //boost::thread::hardware_concurrency()),
     "Total number of threads to allocate for io management.")

    ("db_threads", value<uint32_t>(&db_threads)->default_value(0), //2),
     "Total number of threads to allocate for database management")

    ("cpu_threads", value<uint32_t>(&cpu_threads)->default_value(1), //boost::thread::hardware_concurrency()),
     "Total number of threads to allocate for processing.")
	 /*
    ("db.galaxy_manager.host", boost::program_options::value<std::string>(&galaxy_manager_db.host),
     "Host address for the galaxy_manager datastore")
    ("db.galaxy_manager.schema", boost::program_options::value<std::string>(&galaxy_manager_db.schema),
     "Schema name for the galaxy_manager datastore")
    ("db.galaxy_manager.username", boost::program_options::value<std::string>(&galaxy_manager_db.username),
     "Username for authentication with the galaxy_manager datastore")
    ("db.galaxy_manager.password", boost::program_options::value<std::string>(&galaxy_manager_db.password),
     "Password for authentication with the galaxy_manager datastore")

    ("db.galaxy.host", boost::program_options::value<std::string>(&galaxy_db.host),
     "Host address for the galaxy datastore")
    ("db.galaxy.schema", boost::program_options::value<std::string>(&galaxy_db.schema),
     "Schema name for the galaxy datastore")
    ("db.galaxy.username", boost::program_options::value<std::string>(&galaxy_db.username),
     "Username for authentication with the galaxy datastore")
    ("db.galaxy.password", boost::program_options::value<std::string>(&galaxy_db.password),
     "Password for authentication with the galaxy datastore")

    ("db.swganh_static.host", boost::program_options::value<std::string>(&swganh_static_db.host),
     "Host address for the swganh_static datastore")
    ("db.swganh_static.schema", boost::program_options::value<std::string>(&swganh_static_db.schema),
     "Schema name for the swganh_static datastore")
    ("db.swganh_static.username", boost::program_options::value<std::string>(&swganh_static_db.username),
     "Username for authentication with the swganh_static datastore")
    ("db.swganh_static.password", boost::program_options::value<std::string>(&swganh_static_db.password),
     "Password for authentication with the swganh_static datastore")

    /*("service.login.udp_port",
     boost::program_options::value<uint16_t>(&login_config.listen_port),
     "The port the login service will listen for incoming client connections on")
    ("service.login.address",
     boost::program_options::value<string>(&login_config.listen_address),
     "The public address the login service will listen for incoming client connections on")
    ("service.login.status_check_duration_secs",
     boost::program_options::value<int>(&login_config.galaxy_status_check_duration_secs),
     "The amount of time between checks for updated galaxy status")
    ("service.login.login_error_timeout_secs",
     boost::program_options::value<int>(&login_config.login_error_timeout_secs)->default_value(5),
     "The number of seconds to wait before disconnecting a client after failed login attempt")
    ("service.login.auto_registration",
     boost::program_options::value<bool>(&login_config.login_auto_registration)->default_value(false),
     "Auto Registration flag")
	 */

    /*("service.connection.ping_port", boost::program_options::value<uint16_t>(&connection_config.ping_port),
     "The port the connection service will listen for incoming client ping requests on")
    ("service.connection.udp_port", boost::program_options::value<uint16_t>(&connection_config.listen_port),
     "The port the connection service will listen for incoming client connections on")
    ("service.connection.address", boost::program_options::value<string>(&connection_config.listen_address),
     "The public address the connection service will listen for incoming client connections on")
	 
	 //not yet implemented in our old core
    ("service.simulation.scene", boost::program_options::value<std::vector<std::string>>(&scenes),
     "Loads the specified scene, can have multiple scenes")
	 */

	 //the old cores config Options!!!
	//("script_directory", value<string>(&script_directory)->default_value("scripts"),
  

	("ZoneName", boost::program_options::value<std::string>(&zone_name), "The Name of the scene to load - = the name of the server")
	("GalaxyId", boost::program_options::value<uint16_t>(&galaxy_id)->default_value(2), "The Id of the Galaxy to connect to - = ")
	("BindAddress", boost::program_options::value<std::string>(&bind_address)->default_value("127.0.0.1"), "Network listen address. Use 127.0.0.1 only when your Galaxy consists of one machine. Otherwise use your local IP (ie 192.168.1.100)")
    ("BindPort", boost::program_options::value<uint16_t>(&bind_port), "Port the server listens for messages on. Every server has its dedicated port for communication with the connectionserver")
    ("ServiceMessageHeap", boost::program_options::value<uint32_t>(&service_message_heap)->default_value(8192), "The heap size that gets reserved for messages in the netlayers MessageService")
    ("GlobalMessageHeap", boost::program_options::value<uint32_t>(&global_message_heap)->default_value(8192), "The heap size that gets reserved for messages in the mainthreads MessageService")

    ("DBServer", boost::program_options::value<std::string>(&swganh_db.server)->default_value("localhost"), "Address of the MySQL Server.")
	("DBPort", boost::program_options::value<uint16_t>(&swganh_db.db_port)->default_value(3306), "Port of the MySQL Server.")
    ("DBName", boost::program_options::value<std::string>(&swganh_db.db_schema)->default_value("swganh"), "Name of the MySQL database schema.")
    ("DBUser", boost::program_options::value<std::string>(&swganh_db.username)->default_value("root"), "Username of the database user account.")
    ("DBPass", boost::program_options::value<std::string>(&swganh_db.password)->default_value(""), "Password of the database user account.")
    ("DBMinThreads", boost::program_options::value<uint16_t>(&swganh_db.min_thread)->default_value(4), "Minimum number of threads used for database work.")
    ("DBMaxThreads", boost::program_options::value<uint16_t>(&swganh_db.max_thread)->default_value(16), "Maximum number of threads used for database work.")
    
	("DBGlobalSchema", boost::program_options::value<std::string>(&swganh_db.global_schema)->default_value("swganh_static"), "")
    ("DBGalaxySchema", boost::program_options::value<std::string>(&swganh_db.galaxy_schema)->default_value("swganh"), "")
    ("DBConfigSchema", boost::program_options::value<std::string>(&swganh_db.config_schema)->default_value("swganh_config"), "")

	("ReliablePacketSizeServerToServer", boost::program_options::value<uint16_t>(&swganh_netlayer.reliable_server_server)->default_value(1400), "size of Packets for server server communication")
	("UnreliablePacketSizeServerToServer", boost::program_options::value<uint16_t>(&swganh_netlayer.unreliable_server_server)->default_value(1400), "size of Packets for server server communication")
	("ReliablePacketSizeServerToClient", boost::program_options::value<uint16_t>(&swganh_netlayer.reliable_server_server)->default_value(496), "size of Packets for server client communication 496 is SOE standard. Use lower for bad connection")
	("UnreliablePacketSizeServerToClient", boost::program_options::value<uint16_t>(&swganh_netlayer.unreliable_server_client)->default_value(496), "size of Packets for server client communication")
	("ServerPacketWindowSize", boost::program_options::value<uint32_t>(&swganh_netlayer.server_packet_window)->default_value(800), "amount of packets we send before receiving an ack")
	("ClientPacketWindowSize", boost::program_options::value<uint32_t>(&swganh_netlayer.client_packet_window)->default_value(80), "amount of packets we send before receiving an ack. Please note that the clients UDP Buffer is limited")
	("UdpBufferSize", boost::program_options::value<uint32_t>(&swganh_netlayer.udp_buffer)->default_value(4096), "Kernel UDP Buffer size in kb. This needs to be massive so the servers can keep communicating")
    
    ;

    return desc;
}

SwganhApp::SwganhApp(int argc, char* argv[])
    : io_pool_()
    , cpu_pool_()
    , io_work_(new boost::asio::io_service::work(io_pool_))
    , cpu_work_(new boost::asio::io_service::work(cpu_pool_))
{
    kernel_ = make_shared<SwganhKernel>(io_pool_, cpu_pool_);
    running_ = false;
    initialized_ = false;

    Initialize(argc, argv);
    Start();
}

SwganhApp::~SwganhApp()
{
    Stop();
    // Shutdown Event Dispatcher
    kernel_->Shutdown();

    io_work_.reset();
    cpu_work_.reset();

    // join the threadpool threads until each one has exited.
    for_each(io_threads_.begin(), io_threads_.end(), boost::mem_fn(&boost::thread::join));
    for_each(cpu_threads_.begin(), cpu_threads_.end(), boost::mem_fn(&boost::thread::join));

    kernel_.reset();
}

void SwganhApp::Initialize(int argc, char* argv[])
{
    

    std::cout << "Ben Kenobi : That boy was our last hope." << std::endl;
    std::cout << "      Yoda : No! There is another..." << std::endl << std::endl;
    std::cout << " .d8888b.  888       888  .d8888b.         d8888 888b    888 888    888 " << std::endl;
    std::cout << "d88P  Y88b 888   o   888 d88P  Y88b       d88888 8888b   888 888    888 " << std::endl;
    std::cout << "Y88b.      888  d8b  888 888    888      d88P888 88888b  888 888    888 " << std::endl;
    std::cout << " \"Y888b.   888 d888b 888 888            d88P 888 888Y88b 888 8888888888 " << std::endl;
    std::cout << "    \"Y88b. 888d88888b888 888  88888    d88P  888 888 Y88b888 888    888 " << std::endl;
    std::cout << "      \"888 88888P Y88888 888    888   d88P   888 888  Y88888 888    888 " << std::endl;
    std::cout << "Y88b  d88P 8888P   Y8888 Y88b  d88P  d8888888888 888   Y8888 888    888 " << std::endl;
    std::cout << " \"Y8888P\"  888P     Y888  \"Y8888P88 d88P     888 888    Y888 888    888 " << std::endl << std::endl;
    std::cout << "                             Release " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << " "<< std::endl << std::endl << std::endl;




    // Load the configuration
    LoadAppConfig_(argc, argv);

	// Init Logging - we need to have the config (zone name) for the log file
    SetupLogging_();

    auto app_config = kernel_->GetAppConfig();

    CleanupServices_();

    // append command dir
    std::string py_path = "import sys; sys.path.append('.'); sys.path.append('" + app_config.script_directory + "');";
    {
        swganh::scripting::ScopedGilLock lock;
        PyRun_SimpleString(py_path.c_str());
    }

    // Load the plugin configuration.
    LoadPlugins_(app_config.plugins);

    // Load core services
    LoadCoreServices_();


    initialized_ = true;
}

void SwganhApp::Start()
{
    if (!initialized_)
    {
        throw std::runtime_error("Called application Start before Initialize");
    }

    running_ = true;

    //Create a number of threads to pull packets off the wire.
    for (uint32_t i = 0; i < kernel_->GetAppConfig().io_threads; ++i)
    {
        io_threads_.emplace_back([this] ()
        {
            //Continue looping despite errors.
            //If we successfully leave the run method we return.
            while(true)
            {
                try
                {
                    io_pool_.run();
                    return;
                }
                catch(...)
                {
                    LOG(error) << "A near fatal exception has occurred.";
                }
            }
        });
    }

    for (uint32_t i = 0; i < kernel_->GetAppConfig().cpu_threads; ++i)
    {
        cpu_threads_.emplace_back([this] ()
        {
            //Continue looping despite errors.
            //If we successfully leave the run method we return.
            while(true)
            {
                try
                {
                    cpu_pool_.run();
                    return;
                }
                catch(...)
                {
                    LOG(error) << "A near fatal exception has occurred.";
                }
            }
        });
    }

    kernel_->GetServiceManager()->Start();

    //kernel_->GetEventDispatcher()->Dispatch(std::make_shared<BaseEvent>("Core::ApplicationInitComplete"));
    //Now that services are started, start the scenes.
    //auto simulation_service = kernel_->GetServiceManager()->GetService<SimulationServiceInterface>("SimulationService");

    //Ground Zones
    //simulation_service->StartScene("corellia");
    //simulation_service->StartScene("dantooine");
    //simulation_service->StartScene("dathomir");
    //simulation_service->StartScene("endor");
    //simulation_service->StartScene("lok");
    //simulation_service->StartScene("naboo");
    //simulation_service->StartScene("rori");
    //simulation_service->StartScene("talus");
    //simulation_service->StartScene("tatooine");
    //simulation_service->StartScene("yavin4");
    //simulation_service->StartScene("taanab");

    //Space Zones
    //simulation_service->StartScene("space_corellia");
    //simulation_service->StartScene("space_dantooine");
    //simulation_service->StartScene("space_dathomir");
    //simulation_service->StartScene("space_endor");
    //simulation_service->StartScene("space_lok");
    //simulation_service->StartScene("space_naboo");
    //simulation_service->StartScene("space_tatooine");
    //simulation_service->StartScene("space_yavin4");

    //Special
    //simulation_service->StartScene("tutorial");
}

void SwganhApp::Stop()
{
    running_ = false;
}

bool SwganhApp::IsRunning()
{
    return running_;
}

SwganhKernel* SwganhApp::GetAppKernel() const
{
    return kernel_.get();
}

void SwganhApp::StartInteractiveConsole()
{
    swganh::scripting::ScopedGilLock lock;
//    swganh::Logger::getInstance().DisableConsoleLogging();

#ifdef WIN32
    std::system("cls");
#else
    if (std::system("clear") != 0)
    {
        LOG(warning) << "Error clearing screen, ignoring console mode";
        return;
    }
#endif

    std::cout << "swgpy console " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << std::endl;

    boost::python::object main = boost::python::object (boost::python::handle<>(boost::python::borrowed(
                                     PyImport_AddModule("__main__")
                                 )));
    auto global_dict = main.attr("__dict__");
    global_dict["kernel"] = boost::python::ptr(GetAppKernel());

    PyRun_InteractiveLoop(stdin, "<stdin>");

//    swganh::Logger::getInstance().EnableConsoleLogging();
}

void SwganhApp::LoadAppConfig_(int argc, char* argv[])
{
    auto config_description = kernel_->GetAppConfig().BuildConfigDescription();

    variables_map vm;
    store(parse_command_line(argc, argv, config_description), vm);

	std::string zone_name;
	 if(vm.count("ZoneName") == 0) {
        std::cout << "Enter a zone: ";
        std::cin >> zone_name;
    } else {
        zone_name	= vm["ZoneName"].as<std::string>();
    }

	std::stringstream config_file_name;
    config_file_name << "config/" << zone_name << ".cfg";
	std::stringstream config_file_general_name;
    config_file_general_name << "config/general.cfg";

	ifstream config_file(config_file_name.str());
	ifstream config_file_general(config_file_general_name.str());

    if (!config_file.is_open())
    {
        throw runtime_error("Unable to open the configuration file at: config/swganh.cfg");
    }

    try
    {
		if(config_file_general.is_open())
		{
			store(parse_config_file(config_file_general, config_description, true), vm);
		}
        store(parse_config_file(config_file, config_description, true), vm);
    }
    catch(...)
    {
        throw runtime_error("Unable to parse the configuration file at: config/swganh.cfg");
    }

    notify(vm);
    config_file.close();

    if (vm.count("help"))
    {
        std::cout << config_description << "\n\n";
        exit(0);
    }
}

void SwganhApp::LoadPlugins_(vector<string> plugins)
{
    if (!plugins.empty())
    {
        auto plugin_manager = kernel_->GetPluginManager();
        auto plugin_directory = kernel_->GetAppConfig().plugin_directory;

        for_each(plugins.begin(), plugins.end(), [plugin_manager, plugin_directory] (const string& plugin)
        {
            LOG(info) << "Loading plugin " << plugin;
            plugin_manager->LoadPlugin(plugin, plugin_directory);
        });
    }
}

void SwganhApp::CleanupServices_()
{

    auto service_directory = kernel_->GetServiceDirectory();

    auto services = service_directory->getServiceSnapshot(service_directory->galaxy());

    if (services.empty())
    {
        return;
    }

    LOG(warning) << "Services were not shutdown properly";

    for_each(services.begin(), services.end(), [this, &service_directory] (swganh::service::ServiceDescription& service)
    {
        service_directory->removeService(service);
    });
}

void SwganhApp::LoadCoreServices_()
{

    auto plugin_manager = kernel_->GetPluginManager();
    auto registration_map = plugin_manager->registration_map();

    regex rx("(?:.*\\:\\:)(.*Service)");
    smatch m;

    for_each(registration_map.begin(), registration_map.end(), [this, &rx, &m] (RegistrationMap::value_type& entry)
    {
        std::string name = entry.first;

        if (entry.first.length() > 7 && regex_match(name, m, rx))
        {
            auto service_name = m[1].str();
            auto service = kernel_->GetPluginManager()->CreateObject<swganh::service::ServiceInterface>(name);
            kernel_->GetServiceManager()->AddService(service_name, service);
            LOG(info) << "Loaded Service " << name;
        }
    });
}

void SwganhApp::SetupLogging_()
{
	std::stringstream log_file_name;
	log_file_name << "logs/" << kernel_->GetAppConfig().zone_name << ".log";
	
	
	LOG(error) << " ";
	LOGINIT(log_file_name.str());
}
