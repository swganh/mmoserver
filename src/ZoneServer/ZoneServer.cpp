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

#include "Utils/logger.h"

#include <iostream>
#include <fstream>
#include "anh/event_dispatcher/event_dispatcher.h"


#include "CharacterLoginHandler.h"
#include "CharSheetManager.h"
//	Managers
#include "CraftingManager.h"
#include "AdminManager.h"
#include "ArtisanManager.h"
#include "BuffManager.h"
#include "CombatManager.h"
#include "EntertainerManager.h"
#include "ForageManager.h"
#include "GroupManager.h"
#include "MedicManager.h"
#include "NpcManager.h"
#include "ScoutManager.h"
#include "SkillManager.h"
#include "StateManager.h"
#include "StructureManager.h"
#include "TradeManager.h"
#include "UIManager.h"
#include "WorldManager.h"

#include "Food.h"
#include "NonPersistentItemFactory.h"
#include "NonPersistentNpcFactory.h"
#include "nonPersistantObjectFactory.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerDispatch.h"
#include "ObjectFactory.h"
#include "TravelMapHandler.h"
#include "WorldConfig.h"

// External references
#include "MessageLib/MessageLib.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"
#include "NetworkManager/NetworkManager.h"
#include "NetworkManager/Service.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseManager.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"
#include "Common/EventDispatcher.h"
#include "Utils/utils.h"
#include "Utils/clock.h"
#include "Utils/Singleton.h"

#include "ZoneServer/HamService.h"

#include <boost/thread/thread.hpp>

using anh::event_dispatcher::EventDispatcher;
using std::make_shared;
using std::shared_ptr;

using utils::Singleton;

#ifdef WIN32
#undef ERROR
#endif


//======================================================================================================================

ZoneServer* gZoneServer = NULL;

//======================================================================================================================

ZoneServer::ZoneServer(int argc, char* argv[])
    : BaseServer()
    , mLastHeartbeat(0)
    , event_dispatcher_(make_shared<EventDispatcher>())
    , mNetworkManager(0)
    , mDatabaseManager(0)
    , mRouterService(0)
    , mDatabase(0)
    , ham_service_(nullptr)
{
    Anh_Utils::Clock::Init();

    configuration_options_description_.add_options()
    ("ZoneName", boost::program_options::value<std::string>())
    ("writeResourceMaps", boost::program_options::value<bool>())
    ("heightMapResolution", boost::program_options::value<uint16>()->default_value(3))
    ;

    // This is to retrieve the ZoneName
    LoadOptions_(argc, argv);

    if(configuration_variables_map_.count("ZoneName") == 0) {
        std::cout << "Enter a zone: ";
        std::cin >> mZoneName;
    } else {
        mZoneName = configuration_variables_map_["ZoneName"].as<std::string>();
    }

    std::stringstream config_file_name;
    config_file_name << "config/" << mZoneName << ".cfg";

    // Load Configuration Options
    std::list<std::string> config_files;
    config_files.push_back("config/general.cfg");
    config_files.push_back(config_file_name.str());
    LoadOptions_(argc, argv, config_files);


    LOG(ERR) << "ZoneServer startup sequence for [" << mZoneName << "]";

    // Create and startup our core services.
    mDatabaseManager = new DatabaseManager(DatabaseConfig(configuration_variables_map_["DBMinThreads"].as<uint32_t>(), configuration_variables_map_["DBMaxThreads"].as<uint32_t>(), configuration_variables_map_["DBGlobalSchema"].as<std::string>(), configuration_variables_map_["DBGalaxySchema"].as<std::string>(), configuration_variables_map_["DBConfigSchema"].as<std::string>()));

    // Startup our core modules
    MessageFactory::getSingleton(configuration_variables_map_["GlobalMessageHeap"].as<uint32_t>());

    mNetworkManager = new NetworkManager( NetworkConfig(configuration_variables_map_["ReliablePacketSizeServerToServer"].as<uint16_t>(),
                                          configuration_variables_map_["UnreliablePacketSizeServerToServer"].as<uint16_t>(),
                                          configuration_variables_map_["ReliablePacketSizeServerToClient"].as<uint16_t>(),
                                          configuration_variables_map_["UnreliablePacketSizeServerToClient"].as<uint16_t>(),
                                          configuration_variables_map_["ServerPacketWindowSize"].as<uint32_t>(),
                                          configuration_variables_map_["ClientPacketWindowSize"].as<uint32_t>(),
                                          configuration_variables_map_["UdpBufferSize"].as<uint32_t>()));

    // Connect to the DB and start listening for the RouterServer.
    mDatabase = mDatabaseManager->connect(DBTYPE_MYSQL,
                                          (char*)(configuration_variables_map_["DBServer"].as<std::string>()).c_str(),
                                          configuration_variables_map_["DBPort"].as<uint16_t>(),
                                          (char*)(configuration_variables_map_["DBUser"].as<std::string>()).c_str(),
                                          (char*)(configuration_variables_map_["DBPass"].as<std::string>()).c_str(),
                                          (char*)(configuration_variables_map_["DBName"].as<std::string>()).c_str());

    // increase the server start that will help us to organize our logs to the corresponding serverstarts (mostly for errors)
    mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_ServerStatusUpdate('%s', NULL, NULL, NULL);", mDatabase->galaxy(), mZoneName.c_str());

    mRouterService = mNetworkManager->GenerateService((char*)configuration_variables_map_["BindAddress"].as<std::string>().c_str(), configuration_variables_map_["BindPort"].as<uint16_t>(),configuration_variables_map_["ServiceMessageHeap"].as<uint32_t>()*1024, true);

    // Grab our zoneId out of the DB for this zonename.
    uint32 zoneId = 0;
    DatabaseResult* result = mDatabase->executeSynchSql("SELECT planet_id FROM %s.planet WHERE name=\'%s\';", mDatabase->galaxy(), mZoneName.c_str());


    if (!result->getRowCount())
    {
        LOG(ERR) << "Map not found for [" << mZoneName << "]";

        abort();
    }

    //  Yea, I'm getting annoyed with the DataBinding for such simple tasks.  Will implement a simple interface soon.

    DataBinding* binding = mDatabase->createDataBinding(1);
    binding->addField(DFT_uint32, 0, 4);

    result->getNextRow(binding, &zoneId);

    mDatabase->destroyDataBinding(binding);
    mDatabase->destroyResult(result);

    // We need to register our IP and port in the DB so the connection server can connect to us.
    // Status:  0=offline, 1=loading, 2=online
    _updateDBServerList(1);

    // Place all startup code here.
    mMessageDispatch = new MessageDispatch(mRouterService);

    WorldConfig::Init(zoneId,mDatabase,BString(mZoneName.c_str()));
    ObjectControllerCommandMap::Init(mDatabase);
    MessageLib::Init();
    ObjectFactory::Init(mDatabase);

    //attribute commands for food buffs
    FoodCommandMapClass::Init();

    //structure manager callback functions
    StructureManagerCommandMapClass::Init();

    WorldManager::Init(zoneId,this,mDatabase, configuration_variables_map_["heightMapResolution"].as<uint16>(), configuration_variables_map_["writeResourceMaps"].as<bool>(), mZoneName);

    // Init the non persistent factories. For now we take them one-by-one here, until we have a collection of them.
    // We can NOT create these factories among the already existing ones, if we want to have any kind of "ownership structure",
    // since the existing factories are impossible to delete without crashing the server.
    // NonPersistentContainerFactory::Init(mDatabase);
    (void)NonPersistentItemFactory::Instance();	// This call is just for clarity, when matching the deletion of classes.
    // The object will create itself upon first usage,
    (void)NonPersistentNpcFactory::Instance();

    (void)ForageManager::Instance();
    (void)ScoutManager::Instance();
    (void)NonPersistantObjectFactory::Instance();

    //ArtisanManager callback
    CraftingManager::Init(mDatabase);
    gStateManager.loadStateMaps();
    UIManager::Init(mDatabase,mMessageDispatch);
    CombatManager::Init(mDatabase);
    TravelMapHandler::Init(mDatabase,mMessageDispatch,zoneId);
    CharSheetManager::Init(mDatabase,mMessageDispatch);
    TradeManager::Init(mDatabase,mMessageDispatch);
    BuffManager::Init(mDatabase);
    MedicManager::Init(mMessageDispatch);
    AdminManager::Init(mMessageDispatch);
    EntertainerManager::Init(mDatabase,mMessageDispatch);
    GroupManager::Init(mDatabase,mMessageDispatch);

    if(zoneId != 41)
        StructureManager::Init(mDatabase,mMessageDispatch);

    // Invoked when all creature regions for spawning of lairs are loaded
    // (void)NpcManager::Instance();

    ham_service_ = std::unique_ptr<zone::HamService>(new zone::HamService(Singleton<common::EventDispatcher>::Instance(), gObjControllerCmdPropertyMap));

    ScriptEngine::Init();

    mCharacterLoginHandler = new CharacterLoginHandler(mDatabase, mMessageDispatch);

    mObjectControllerDispatch = new ObjectControllerDispatch(mDatabase,mMessageDispatch);
}

//======================================================================================================================

ZoneServer::~ZoneServer(void)
{
    LOG(INFO) << "ZoneServer shutting down";

    // We're shutting down, so update the DB again.
    _updateDBServerList(0);

    // Shutdown and delete the game modules.
    delete mCharacterLoginHandler;
    gTravelMapHandler->Shutdown();
    gTradeManager->Shutdown();
    delete mObjectControllerDispatch;
    AdminManager::deleteManager();

    gWorldManager->Shutdown();	// Should be closed before script engine and script support, due to halting of scripts.
    gScriptEngine->shutdown();
    ScriptSupport::Instance()->destroyInstance();

    delete mMessageDispatch;

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

    LOG(INFO) << "ZoneServer shutdown complete";
}

//======================================================================================================================

void ZoneServer::handleWMReady()
{
    _updateDBServerList(2);
    LOG(WARNING) << "ZoneServer startup complete";

    // Connect to the ConnectionServer;
    _connectToConnectionServer();
}

//======================================================================================================================

void ZoneServer::Process(void)
{
    uint64_t current_timestep = Anh_Utils::Clock::getSingleton()->getGlobalTime();
    // Process our game modules
    mObjectControllerDispatch->Process();
    gWorldManager->Process();
    gScriptEngine->process();
    mMessageDispatch->Process();
    gEventDispatcher.Tick(current_timestep);

    event_dispatcher_->tick(0);

    //is there stalling ?
    mRouterService->Process();

    //  Process our core services

    mDatabaseManager->process();
    mNetworkManager->Process();

    // Heartbeat once in awhile
    if (Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastHeartbeat > 180000)
    {
        mLastHeartbeat = static_cast<uint32>(Anh_Utils::Clock::getSingleton()->getLocalTime());
    }
}

//======================================================================================================================

void ZoneServer::_updateDBServerList(uint32 status)
{
    // Update the DB with our status.  This must be synchronous as the connection server relies on this data.
    mDatabase->executeProcedure("CALL %s.sp_ServerStatusUpdate('%s', %u, '%s', %u)", mDatabase->galaxy(), mZoneName.c_str(), status, mRouterService->getLocalAddress(), mRouterService->getLocalPort());
}

//======================================================================================================================

void ZoneServer::_connectToConnectionServer(void)
{
    ProcessAddress processAddress;
    memset(&processAddress, 0, sizeof(ProcessAddress));

    // Query the DB to find out who this is.
    // setup our databinding parameters.
    DataBinding* binding = mDatabase->createDataBinding(5);
    binding->addField(DFT_uint32, offsetof(ProcessAddress, mType), 4);
    binding->addField(DFT_bstring, offsetof(ProcessAddress, mAddress), 16);
    binding->addField(DFT_uint16, offsetof(ProcessAddress, mPort), 2);
    binding->addField(DFT_uint32, offsetof(ProcessAddress, mStatus), 4);
    binding->addField(DFT_uint32, offsetof(ProcessAddress, mActive), 4);

    // Execute our statement
    DatabaseResult* result = mDatabase->executeSynchSql("SELECT id, address, port, status, active FROM %s.config_process_list WHERE name='connection';",mDatabase->galaxy());
    uint32 count = static_cast<uint32>(result->getRowCount());

    // If we found them
    if (count == 1)
    {
        // Retrieve our routes and add them to the map.
        result->getNextRow(binding, &processAddress);
    }

    // Delete our DB objects.
    mDatabase->destroyDataBinding(binding);
    mDatabase->destroyResult(result);

    // Now connect to the ConnectionServer
    DispatchClient* client = new DispatchClient();
    mRouterService->Connect(client, processAddress.mAddress.getAnsi(), processAddress.mPort);

    // Send our registration message
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClusterRegisterServer);
    gMessageFactory->addString(mZoneName);

    Message* message = gMessageFactory->EndMessage();
    client->SendChannelA(message, 0, CR_Connection, 1);
}

//======================================================================================================================

int main(int argc, char* argv[])
{
    //try {

    // Start things up
    gZoneServer = new ZoneServer(argc, argv);

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

    delete gZoneServer;
    gZoneServer = NULL;
    //} catch (std::exception& e) {
    //	std::cout << e.what() << std::endl;
    //	std::cin.get();
    //	return 0;
    //}

    return 0;
}

//======================================================================================================================







