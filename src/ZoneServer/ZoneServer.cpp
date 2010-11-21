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

#include <glog/logging.h>

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
#include "Common/ConfigManager.h"
#include "Utils/utils.h"
#include "Utils/clock.h"
#include "Utils/Singleton.h"

#include "ZoneServer/HamService.h"

#include <boost/thread/thread.hpp>

using utils::Singleton;
using common::EventDispatcher;

#ifdef WIN32
#undef ERROR
#endif


//======================================================================================================================

ZoneServer* gZoneServer = NULL;

//======================================================================================================================

ZoneServer::ZoneServer(int8* zoneName)
    : mZoneName(zoneName)
    , mLastHeartbeat(0)
    , mNetworkManager(0)
    , mDatabaseManager(0)
    , mRouterService(0)
    , mDatabase(0)
    , ham_service_(nullptr)
{
    Anh_Utils::Clock::Init();

    LOG(INFO) << "ZoneServer startup sequence for [" << zoneName << "]";

    // Create and startup our core services.
    mDatabaseManager = new DatabaseManager();

    mNetworkManager = new NetworkManager();

    // Connect to the DB and start listening for the RouterServer.
    mDatabase = mDatabaseManager->connect(DBTYPE_MYSQL,
                                          (int8*)(gConfig->read<std::string>("DBServer")).c_str(),
                                          gConfig->read<int>("DBPort"),
                                          (int8*)(gConfig->read<std::string>("DBUser")).c_str(),
                                          (int8*)(gConfig->read<std::string>("DBPass")).c_str(),
                                          (int8*)(gConfig->read<std::string>("DBName")).c_str());

    // increase the server start that will help us to organize our logs to the corresponding serverstarts (mostly for errors)
    mDatabase->executeProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('%s', NULL, NULL, NULL);", zoneName);
    

    mRouterService = mNetworkManager->GenerateService((char*)gConfig->read<std::string>("BindAddress").c_str(), gConfig->read<uint16>("BindPort"),gConfig->read<uint32>("ServiceMessageHeap")*1024,true);

    // Grab our zoneId out of the DB for this zonename.
    uint32 zoneId = 0;
    DatabaseResult* result = mDatabase->executeSynchSql("SELECT planet_id FROM planet WHERE name=\'%s\';", zoneName);
    

    if (!result->getRowCount())
    {
        LOG(ERROR) << "Map not found for [" << zoneName << "]";
        
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

    WorldConfig::Init(zoneId,mDatabase,zoneName);
    ObjectControllerCommandMap::Init(mDatabase);
    MessageLib::Init();
    ObjectFactory::Init(mDatabase);

	//attribute commands for food buffs
    FoodCommandMapClass::Init();

    //structure manager callback functions
    StructureManagerCommandMapClass::Init();

    WorldManager::Init(zoneId,this,mDatabase);

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

    ham_service_ = std::unique_ptr<zone::HamService>(new zone::HamService(Singleton<EventDispatcher>::Instance(), gObjControllerCmdPropertyMap));

    ScriptEngine::Init();

    mCharacterLoginHandler = new CharacterLoginHandler(mDatabase,mMessageDispatch);

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
    mDatabase->executeProcedure("CALL sp_ServerStatusUpdate('%s', %u, '%s', %u)", mZoneName.getAnsi(), status, mRouterService->getLocalAddress(), mRouterService->getLocalPort());
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
    DatabaseResult* result = mDatabase->executeSynchSql("SELECT id, address, port, status, active FROM config_process_list WHERE name='connection';");
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
    // Initialize the google logging.
    google::InitGoogleLogging(argv[0]);

#ifndef _WIN32
    google::InstallFailureSignalHandler();
#endif
    
    FLAGS_log_dir = "./logs";
    FLAGS_stderrthreshold = 1;
    
    //set stdout buffers to 0 to force instant flush
    setvbuf( stdout, NULL, _IONBF, 0);

    // The second argument on the command line should be the zone name.
    //OnlyInstallUnhandeldExceptionFilter(); // Part of stackwalker
    char zone[50];
    if(argc < 2)
    {
        printf("A list of Zones can be found in ZoneList.txt\n");
        printf("Enter zone: ");
        int n = scanf("%s", zone);

        if (n != 1)
            std::exit(-1);

        // Remove the // if it cause problems, This enables you to just type
        // inn the zone insted of navigate using comand prompt.
        //std::exit(-1);
    }
    else
    {
        sprintf(zone,"%s",argv[1]);
    }

    int8 configfileName[64];
    sprintf(configfileName, "%s.cfg", zone);

    try {
        ConfigManager::Init(configfileName);
    } catch (file_not_found) {
        std::cout << "Unable to find configuration file: " << CONFIG_DIR << configfileName << std::endl;
        exit(-1);
    }

    // Start things up
    gZoneServer = new ZoneServer((int8*)(gConfig->read<std::string>("ZoneName")).c_str());

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

    return 0;
}

//======================================================================================================================







