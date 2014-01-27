/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "ZoneServer.h"

#include <iostream>
#include <fstream>
#include "anh/event_dispatcher/event_dispatcher.h"

#include "anh/service/datastore.h"
#include "anh/service/service_manager.h"

#include "ZoneServer/GameSystemManagers/CharacterLoginHandler.h"
#include "CharSheetManager.h"
//	Managers
#include "ZoneServer/GameSystemManagers/Crafting Manager/CraftingManager.h"
#include "Zoneserver/GameSystemManagers/AdminManager.h"
#include "ZoneServer/ProfessionManagers/Artisan Manager/ArtisanManager.h"
#include "Zoneserver/GameSystemManagers/Buff Manager/BuffManager.h"
#include "Zoneserver/GameSystemManagers/Combat Manager/CombatManager.h"
#include "ZoneServer/ProfessionManagers/Entertainer Manager/EntertainerManager.h"
#include "ZoneServer/GameSystemManagers/Forage Manager/ForageManager.h"
#include "ZoneServer/GameSystemManagers/Group Manager/GroupManager.h"
#include "Zoneserver/ProfessionManagers/Medic Manager/MedicManager.h"
#include "Zoneserver/GameSystemManagers/NPC Manager/NpcManager.h"
#include "Zoneserver/ProfessionManagers/Scout Manager/ScoutManager.h"
#include "ZoneServer/GameSystemManagers/Skill Manager/SkillManager.h"
#include "ZoneServer/GameSystemManagers/State Manager/StateManager.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/StructureManager.h"
#include "ZoneServer/GameSystemManagers/Trade Manager/TradeManager.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
#include "ZoneServer/WorldManager.h"

#include <ZoneServer\Services\terrain\terrain_init.h>

#include "Zoneserver/Objects/Food.h"
#include "Zoneserver/Objects/NonPersistentItemFactory.h"
#include "Zoneserver/GameSystemManagers/NPC Manager/NonPersistentNpcFactory.h"
#include "ZoneServer/Objects/nonPersistantObjectFactory.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/ObjectController/ObjectControllerDispatch.h"
#include "ZoneServer/Objects/ObjectFactory.h"
#include "ZoneServer/GameSystemManagers/Travel Manager/TravelMapHandler.h"
#include "ZoneServer/WorldConfig.h"

// External references
//#include <ZoneServer\Services\scene_events.h>

//#include "ScriptEngine/ScriptEngine.h"
//#include "ScriptEngine/ScriptSupport.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseManager.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "NetworkManager/NetworkManager.h"
#include "NetworkManager/Service.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"
#include "MessageLib/MessageLib.h"

#include "Common/EventDispatcher.h"
#include "Utils/utils.h"
#include "anh/Utils/clock.h"
#include "Utils/Singleton.h"

#include <cppconn/resultset.h>

#include "anh/logger.h"

#include <memory>

#include "ZoneServer/GameSystemManagers/Ham Manager/HamService.h"

//#include <boost/thread/thread.hpp>
#include <boost/thread.hpp>
#include <boost/python.hpp>

#include "anh/service/service_interface.h"
#include "anh/plugin/plugin_manager.h"
#include "anh/app/swganh_app.h"

#ifdef WIN32
#include <regex>
#else
#include <boost/regex.hpp>
#endif

/*
// instantiate leak-finder:
#define INIT_LEAK_FINDER
// additional, use the XML-Output-File to analyse with "MemLeakAnalyse.exe"
#define XML_LEAK_FINDER
#include "Zoneserver/LeakFinder.h"
*/


#ifdef WIN32
using std::regex;
using std::smatch;
using std::regex_match;
#else
using boost::regex;
using boost::smatch;
using boost::regex_match;
#endif


using swganh::event_dispatcher::EventDispatcher;
using std::make_shared;
using std::shared_ptr;

using utils::Singleton;

//======================================================================================================================

ZoneServer* gZoneServer = NULL;

//======================================================================================================================

ZoneServer::ZoneServer(int argc, char* argv[], swganh::app::SwganhKernel*	kernel)
    : mLastHeartbeat(0)
    , mNetworkManager(0)
    , mDatabaseManager(0)
    , mRouterService(0)
	, kernel_(kernel)
    , ham_service_(nullptr)
{
    Anh_Utils::Clock::Init();
	

	LOG(error) << "ZoneServer startup sequence for [" << kernel_->GetAppConfig().zone_name << "]";

    // Create and startup our core services.
    //mDatabaseManager = new swganh::database::DatabaseManager(swganh::database::DatabaseConfig(configuration_variables_map_["DBMinThreads"].as<uint32_t>(), configuration_variables_map_["DBMaxThreads"].as<uint32_t>(), configuration_variables_map_["DBGlobalSchema"].as<std::string>(), configuration_variables_map_["DBGalaxySchema"].as<std::string>(), configuration_variables_map_["DBConfigSchema"].as<std::string>()));
	mDatabaseManager = new swganh::database::DatabaseManager(swganh::database::DatabaseConfig(kernel_->GetAppConfig().swganh_db.min_thread, kernel_->GetAppConfig().swganh_db.max_thread, kernel_->GetAppConfig().swganh_db.global_schema, kernel_->GetAppConfig().swganh_db.galaxy_schema, kernel_->GetAppConfig().swganh_db.config_schema));

    // Startup our core modules
	MessageFactory::getSingleton(kernel_->GetAppConfig().global_message_heap);

	mNetworkManager = new NetworkManager( NetworkConfig(kernel_->GetAppConfig().swganh_netlayer.reliable_server_server,
										kernel_->GetAppConfig().swganh_netlayer.unreliable_server_server,
										kernel_->GetAppConfig().swganh_netlayer.reliable_server_client,
										kernel_->GetAppConfig().swganh_netlayer.unreliable_server_client,
										kernel_->GetAppConfig().swganh_netlayer.server_packet_window,
										kernel_->GetAppConfig().swganh_netlayer.client_packet_window,
										kernel_->GetAppConfig().swganh_netlayer.udp_buffer));


    // Connect to the DB and start listening for the RouterServer.
	kernel_->SetDatabase( mDatabaseManager->connect(swganh::database::DBTYPE_MYSQL,
											kernel_->GetAppConfig().swganh_db.server,
                                            //(char*)(configuration_variables_map_["DBServer"].as<std::string>()).c_str(),
											kernel_->GetAppConfig().swganh_db.db_port,
											kernel_->GetAppConfig().swganh_db.username,
											kernel_->GetAppConfig().swganh_db.password,
											kernel_->GetAppConfig().swganh_db.db_schema));
                                          //(char*)(configuration_variables_map_["DBUser"].as<std::string>()).c_str(),
                                          //(char*)(configuration_variables_map_["DBPass"].as<std::string>()).c_str(),
                                          //(char*)(configuration_variables_map_["DBName"].as<std::string>()).c_str()) );


    //mRouterService = mNetworkManager->GenerateService((char*)configuration_variables_map_["BindAddress"].as<std::string>().c_str(), configuration_variables_map_["BindPort"].as<uint16_t>(),configuration_variables_map_["ServiceMessageHeap"].as<uint32_t>()*1024, true);
	mRouterService = mNetworkManager->GenerateService((char*)kernel_->GetAppConfig().bind_address.c_str(), kernel_->GetAppConfig().bind_port,kernel_->GetAppConfig().service_message_heap *1024, true);

	swganh::terrain::Initialize(kernel_);

	// Load core services
    LoadCoreServices_();

    // Grab our zoneId out of the DB for this zonename.
    uint32 zoneId = 0;
	std::stringstream sql;
	sql << "SELECT planet_id, terrain_file  FROM  " << kernel_->GetDatabase()->galaxy() << ".planet WHERE name= '" << kernel_->GetAppConfig().zone_name << "';";
	

	swganh::database::DatabaseResult* result = kernel_->GetDatabase()->executeSql(sql.str());
	if (!result->getRowCount())
    {
		LOG(error) << "Map not found for [" << kernel_->GetAppConfig().zone_name << "]";

        abort();
    }

	std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
	//be aware however that the classic bindings are *much* faster
	//for these small queries this is absolutely acceptable, though
	std::string trn;
	try	{
		if (!result_set->next()) {
            LOG(warning) << "ZoneServer::ZoneServer : Unable to load Zone Id";
            exit(1);
        }
	
		zoneId = result_set->getInt("planet_id");
		trn = result_set->getString("terrain_file");
	}
	catch(std::exception& e) {
    	std::cout << e.what() << std::endl;
    	std::cin.get();
		 exit(1);
    //	return 0;
    }
	
    kernel_->GetDatabase()->destroyResult(result);

	// increase the server start that will help us to organize our logs to the corresponding serverstarts (mostly for errors)
	kernel_->GetDatabase()->executeProcedureAsync(0, 0, "CALL %s.sp_ServerStatusUpdate('%s', NULL, NULL, NULL);", kernel_->GetDatabase()->galaxy(), (char*) kernel_->GetAppConfig().zone_name.c_str());

    // Place all startup code here.
	kernel_->SetDispatch(new MessageDispatch(mRouterService));

	// We need to register our IP and port in the DB so the connection server can connect to us.
    // Status:  0=offline, 1=loading, 2=online
    _updateDBServerList(1);

	Anh_Utils::Clock::Init();

	WorldConfig::Init(zoneId,kernel_,kernel_->GetAppConfig().zone_name);
    ObjectControllerCommandMap::Init(kernel_->GetDatabase());
	MessageLib::Init(kernel_->GetEventDispatcher());
    ObjectFactory::Init(kernel_);

    //attribute commands for food buffs
    FoodCommandMapClass::Init();

    //structure manager callback functions
    StructureManagerCommandMapClass::Init();

	WorldManager::Init(zoneId,this,kernel_, trn, false);


    // NonPersistentContainerFactory::Init(mDatabase);
    (void)NonPersistentItemFactory::Instance();	// This call is just for clarity, when matching the deletion of classes.
    // The object will create itself upon first usage,
    (void)NonPersistentNpcFactory::Instance(kernel_);

    (void)ForageManager::Instance();
    (void)ScoutManager::Instance();
    (void)NonPersistantObjectFactory::Instance();

    //ArtisanManager callback
    CraftingManager::Init(kernel_->GetDatabase());
    gStateManager.loadStateMaps();
    UIManager::Init(kernel_->GetDispatch());
    CombatManager::Init(kernel_->GetDatabase());
	TravelMapHandler::Init(kernel_->GetDatabase(),kernel_->GetDispatch(),zoneId);
    CharSheetManager::Init(kernel_->GetDatabase(),kernel_->GetDispatch());
    TradeManager::Init(kernel_->GetDatabase(),kernel_->GetDispatch());
    BuffManager::Init(kernel_->GetDatabase());
    MedicManager::Init(kernel_->GetDispatch());
    AdminManager::Init(kernel_->GetDispatch());
    EntertainerManager::Init(kernel_->GetDatabase(),kernel_->GetDispatch());
    GroupManager::Init(kernel_->GetDatabase(),kernel_->GetDispatch());

    if(zoneId != 41)
        StructureManager::Init(kernel_->GetDatabase(),kernel_->GetDispatch());

    // Invoked when all creature regions for spawning of lairs are loaded
    // (void)NpcManager::Instance();

    ham_service_ = std::unique_ptr<zone::HamService>(new zone::HamService(Singleton<common::EventDispatcher>::Instance(), gObjControllerCmdPropertyMap));

    //ScriptEngine::Init();

    mCharacterLoginHandler = new CharacterLoginHandler(kernel_->GetDatabase(), kernel_->GetDispatch());

    mObjectControllerDispatch = new ObjectControllerDispatch(kernel_->GetDispatch());
}

//======================================================================================================================

ZoneServer::~ZoneServer(void)
{
    LOG(info) << "ZoneServer shutting down...";

    // We're shutting down, so update the DB again.
    _updateDBServerList(0);

    // Shutdown and delete the game modules.
    delete mCharacterLoginHandler;
    gTravelMapHandler->Shutdown();
    gTradeManager->Shutdown();
    delete mObjectControllerDispatch;
    AdminManager::deleteManager();

    gWorldManager->Shutdown();	// Should be closed before script engine and script support, due to halting of scripts.
    
	//gScriptEngine->shutdown();
    //ScriptSupport::Instance()->destroyInstance();

    // gMessageFactory->Shutdown(); // Nothing to do there yet, since deleting of the heap is done in the destructor.

    // Delete the non persistent factories, that are possible to delete.
    // NonPersistentContainerFactory::getSingletonPtr()->destroySingleton();
    NonPersistentItemFactory::deleteFactory();
    NonPersistentNpcFactory::deleteFactory();
    ScoutManager::deleteManager();
    WorldConfig::deleteManager();

    NonPersistantObjectFactory::deleteFactory();

    // Shutdown and delete our core services.
    mNetworkManager->DestroyService(mRouterService);
    delete mNetworkManager;
    delete mDatabaseManager;
    delete gSkillManager->getSingletonPtr();
    delete gMedicManager->getSingletonPtr();
    delete gBuffManager->getSingletonPtr();

    // NOW, I can feel that it should be safe to delete the data holding messages.
    gMessageFactory->destroySingleton();

	gClock->destroySingleton();

    LOG(info) << "...ZoneServer shutdown complete";
}

//======================================================================================================================

void ZoneServer::handleWMReady()
{
    _updateDBServerList(2);
    LOG(warning) << "ZoneServer startup complete";

    // Connect to the ConnectionServer;
    _connectToConnectionServer();
}

//======================================================================================================================

void ZoneServer::Process(void)
{
    uint64_t current_timestep = Anh_Utils::Clock::getSingleton()->getLocalTime();
    // Process our game modules
    mObjectControllerDispatch->Process();
    gWorldManager->Process();
    //gScriptEngine->process();
	
	//thats the message Dispatch
	kernel_->GetDispatch()->Process();

	//thats the old pre NewCore dispatcher
    gEventDispatcher.Tick(current_timestep);

    //event_dispatcher_->tick(0);

    //is there stalling ?
    mRouterService->Process();

    //  Process our core services

    mDatabaseManager->process();
    mNetworkManager->Process();

    // Heartbeat once in awhile
	uint64 time = Anh_Utils::Clock::getSingleton()->getLocalTime();
    if (time - mLastHeartbeat > 1800000)
    {
        mLastHeartbeat = time;
		LOG(info) << "Zone : " << kernel_->GetAppConfig().zone_name << " currently serves " << gWorldManager->getPlayerAccMap()->size() << "Players";
		
		//tick the db so the connection wont die when we are idle to long
		_updateDBServerList(2);
    }
}

//======================================================================================================================

void ZoneServer::_updateDBServerList(uint32 status)
{
    // Update the DB with our status.  This must be synchronous as the connection server relies on this data.
	std::stringstream sql;
	sql << "CALL " << kernel_->GetDatabase()->galaxy() << ".sp_ServerStatusUpdate('" << kernel_->GetAppConfig().zone_name << "', "
		<< status << ",'" << mRouterService->getLocalAddress() << "'," << mRouterService->getLocalPort() << ");";
	
	swganh::database::DatabaseResult* result = kernel_->GetDatabase()->executeProcedure(sql.str().c_str());
	kernel_->GetDatabase()->destroyResult(result);
}

//======================================================================================================================

void ZoneServer::_connectToConnectionServer(void)
{
    ProcessAddress processAddress;
    memset(&processAddress, 0, sizeof(ProcessAddress));

    // Query the DB to find out who this is.
    // setup our databinding parameters.
    swganh::database::DataBinding* binding = kernel_->GetDatabase()->createDataBinding(5);
    binding->addField(swganh::database::DFT_uint32, offsetof(ProcessAddress, mType), 4);
    binding->addField(swganh::database::DFT_bstring, offsetof(ProcessAddress, mAddress), 16);
    binding->addField(swganh::database::DFT_uint16, offsetof(ProcessAddress, mPort), 2);
    binding->addField(swganh::database::DFT_uint32, offsetof(ProcessAddress, mStatus), 4);
    binding->addField(swganh::database::DFT_uint32, offsetof(ProcessAddress, mActive), 4);

    // Execute our statement
    swganh::database::DatabaseResult* result = kernel_->GetDatabase()->executeSynchSql("SELECT id, address, port, status, active FROM %s.config_process_list WHERE name='connection';",kernel_->GetDatabase()->galaxy());
    uint32 count = static_cast<uint32>(result->getRowCount());

    // If we found them
    if (count == 1)
    {
        // Retrieve our routes and add them to the map.
        result->getNextRow(binding, &processAddress);
    }

    // Delete our DB objects.
    kernel_->GetDatabase()->destroyDataBinding(binding);
    kernel_->GetDatabase()->destroyResult(result);

    // Now connect to the ConnectionServer
    DispatchClient* client = new DispatchClient();
    mRouterService->Connect(client, processAddress.mAddress.getAnsi(), processAddress.mPort);

    // Send our registration message
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClusterRegisterServer);
	gMessageFactory->addString(kernel_->GetAppConfig().zone_name);

    Message* message = gMessageFactory->EndMessage();
    client->SendChannelA(message, 0, CR_Connection, 1);
}

//======================================================================================================================

int main(int argc, char* argv[])
{
    //InitAllocCheck();

	Py_Initialize();
    PyEval_InitThreads();

	// Step 2: Release the GIL from the main thread so that other threads can use it
    PyEval_ReleaseThread(PyGILState_GetThisThreadState());

	//app will later hold all our plugins and service references
	//so far it holds the kernel
	swganh::app::SwganhApp app(argc, argv);
	//try {

    // Start things up
    gZoneServer = new ZoneServer(argc, argv, app.GetAppKernel());

	//gZoneServer->kernel_ = app.GetAppKernel();
    // Main loop
    while(1)
    {
        if(AdminManager::Instance()->shutdownZone())
        {
            break;
        }
        else if (Anh_Utils::kbhit())
        {
            char input = std::cin.get();
            if (input == '`' || input =='~')
            {
               app.StartInteractiveConsole();
            }else
            if(input == 'q')
            {
                break;
            } else if(input == 'm') {
                char message[256];
                std::cin.getline(message,256);
                gWorldManager->zoneSystemMessage(message);
            }
        }

        gZoneServer->Process();
        gMessageFactory->Process(); //Garbage Collection

        boost::this_thread::sleep(boost::posix_time::milliseconds(1));

    }

    // Shut things down
	// Step 4: Lock the GIL before calling finalize
    PyGILState_Ensure();
    Py_Finalize();

    delete gZoneServer;
    gZoneServer = NULL;
    //} catch (std::exception& e) {
    //	std::cout << e.what() << std::endl;
    //	std::cin.get();
    //	return 0;
    //}
	//DeInitAllocCheck();
    return 0;
}

//======================================================================================================================


void ZoneServer::CleanupServices_()
{


    auto service_directory = kernel_->GetServiceDirectory();

    auto services = service_directory->getServiceSnapshot(service_directory->galaxy());

    if (services.empty())
    {
        return;
    }

    LOG(warning) << "Services were not shutdown properly";

    std::for_each(services.begin(), services.end(), [this, &service_directory] (swganh::service::ServiceDescription& service)
    {
        service_directory->removeService(service);
    });
	
}

void ZoneServer::LoadCoreServices_()
{
	
    auto plugin_manager = kernel_->GetPluginManager();
    auto registration_map = plugin_manager->registration_map();

    regex rx("(?:.*\\:\\:)(.*Service)");
    smatch m;

	std::for_each(registration_map.begin(), registration_map.end(), [this, &rx, &m] (swganh::plugin::RegistrationMap::value_type& entry)
    {
        std::string name = entry.first;

        if (entry.first.length() > 7 && regex_match(name, m, rx))
        {
            auto service_name = m[1].str();
			std::tr1::shared_ptr<swganh::service::ServiceInterface> service;

			service = kernel_->GetPluginManager()->CreateObject<swganh::service::ServiceInterface>(name);

            kernel_->GetServiceManager()->AddService(service_name, service);
            LOG(info) << "Loaded Service " << name;
        }
    });
	
}