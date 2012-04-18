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

#include "ConnectionServer.h"

#include "ConnectionServerOpcodes.h"
#include "ClientManager.h"
#include "ConnectionDispatch.h"
#include "ServerManager.h"
#include "MessageRouter.h"

#include "NetworkManager/NetworkManager.h"
#include "NetworkManager/Service.h"

// Fix for issues with glog redefining this constant
#ifdef _WIN32
#undef ERROR
#endif

#include <glog/logging.h>

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseManager.h"

#include "NetworkManager/MessageFactory.h"
#include "Common/ConfigManager.h"
#include "Utils/utils.h"
#include "Utils/clock.h"

//#include "stackwalker.h"
#include <boost/thread/thread.hpp>

//======================================================================================================================

ConnectionServer* gConnectionServer = 0;

//======================================================================================================================

ConnectionServer::ConnectionServer(void) :
    mDatabaseManager(0),
    mDatabase(0),
    mNetworkManager(0),
    mMessageRouter(0),
    mClientManager(0),
    mServerManager(0),
    mConnectionDispatch(0),
    mClusterId(0),
    mClientService(0),
    mServerService(0),
    mLocked(false),
    mLastHeartbeat(0)
{
    Anh_Utils::Clock::Init();
    // log msg to default log
    //gLogger->printSmallLogo();
    LOG(WARNING) << "ConnectionServer Startup";

    // Startup our core modules
    mNetworkManager = new NetworkManager();

    // Create our status service
    //clientservice
    mClientService = mNetworkManager->GenerateService((char*)gConfig->read<std::string>("BindAddress").c_str(), gConfig->read<uint16>("BindPort"),gConfig->read<uint32>("ClientServiceMessageHeap")*1024, false);//,5);
    //serverservice
    mServerService = mNetworkManager->GenerateService((char*)gConfig->read<std::string>("ClusterBindAddress").c_str(), gConfig->read<uint16>("ClusterBindPort"),gConfig->read<uint32>("ServerServiceMessageHeap")*1024, true);//,15);

    mDatabaseManager = new DatabaseManager();

    mDatabase = mDatabaseManager->connect(DBTYPE_MYSQL,
                                          (char*)(gConfig->read<std::string>("DBServer")).c_str(),
                                          gConfig->read<int>("DBPort"),
                                          (char*)(gConfig->read<std::string>("DBUser")).c_str(),
                                          (char*)(gConfig->read<std::string>("DBPass")).c_str(),
                                          (char*)(gConfig->read<std::string>("DBName")).c_str());

    mClusterId = gConfig->read<uint32>("ClusterId");

    mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_GalaxyStatusUpdate(%u, %u);",mDatabase->galaxy(), 1, mClusterId); // Set status to online
    

    mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_ServerStatusUpdate('connection', NULL, NULL, NULL);",mDatabase->galaxy());
    

    // In case of a crash, we need to cleanup the DB a little.
    mDatabase->executeSynchSql("UPDATE %s.account SET account_loggedin=0 WHERE account_loggedin=%u;",mDatabase->galaxy(), mClusterId);
    
    // Status:  0=offline, 1=loading, 2=online
    _updateDBServerList(1);

    // Instant the messageFactory. It will also run the Startup ().
    (void)MessageFactory::getSingleton();		// Use this a marker of where the factory is instanced.
    // The code itself here is not needed, since it will instance itself at first use.

    // Startup our router modules.
    mConnectionDispatch = new ConnectionDispatch();
    mMessageRouter = new MessageRouter(mDatabase, mConnectionDispatch);
    mClientManager = new ClientManager(mClientService, mDatabase, mMessageRouter, mConnectionDispatch);
    mServerManager = new ServerManager(mServerService, mDatabase, mMessageRouter, mConnectionDispatch,mClientManager);

    // We're done initiailizing.
    _updateDBServerList(2);

    LOG(WARNING) << "Connection server startup complete";
}

//======================================================================================================================

ConnectionServer::~ConnectionServer(void)
{
    LOG(WARNING) << "ConnectionServer Shutting down...";

    // Update our status for the LoginServer
    mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_GalaxyStatusUpdate(%u, %u);",mDatabase->galaxy(), 0, mClusterId); // Status set to offline
    

    // We're shuttind down, so update the DB again.
    _updateDBServerList(0);

    delete mClientManager;
    delete mServerManager;
    delete mMessageRouter;
    delete mConnectionDispatch;

    // Destroy our network services.
    mNetworkManager->DestroyService(mServerService);
    mNetworkManager->DestroyService(mClientService);

    // Shutdown our core modules
    delete mDatabaseManager;
    delete mNetworkManager;

    MessageFactory::getSingleton()->destroySingleton();	// Delete message factory and call shutdown();

    LOG(WARNING) << "ConnectionServer Shutdown Complete";
}

//======================================================================================================================

void ConnectionServer::Process(void)
{
    // Process our core services first.
    //mNetworkManager->Process();
    mDatabaseManager->process();

    //we dont want this stalled by the clients!!!
    mServerService->Process();
    mClientService->Process();

    // Now process our sub modules
    gMessageFactory->Process();
    mClientManager->Process();
    mServerManager->Process();
    mMessageRouter->Process();


    // Heartbeat once in awhile
    if (Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastHeartbeat > 180000)//main loop every 10ms
    {
        mLastHeartbeat = static_cast<uint32>(Anh_Utils::Clock::getSingleton()->getLocalTime());
        //gLogger->log(LogManager::NOTICE,"ConnectionServer Heartbeat. Connected Servers:%u Active Servers:%u", mServerManager->getConnectedServers(), mServerManager->getActiveServers());
    }

}

//======================================================================================================================

void ConnectionServer::_updateDBServerList(uint32 status)
{
    // Execute our query
    mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_ServerStatusUpdate('connection', %u, '%s', %u);",mDatabase->galaxy(), status, mServerService->getLocalAddress(), mServerService->getLocalPort());
    
}

//======================================================================================================================
void ConnectionServer::ToggleLock()
{
    mLocked = !mLocked;

    if(mLocked)
    {
        // Update our status for the LoginServer
        mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_GalaxyStatusUpdate(%u, %u);",mDatabase->galaxy(), 3, mClusterId); // Status set to online (DEV / CSR Only)
        
        LOG(WARNING) << "Locking server to normal users";
    } else {
        // Update our status for the LoginServer
        mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_GalaxyStatusUpdate(%u, %u);",mDatabase->galaxy(), 2, mClusterId); // Status set to online
        
        LOG(WARNING) << "unlocking server to normal users";
    }
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

    try {
        ConfigManager::Init("ConnectionServer.cfg");
    } catch (file_not_found) {
        std::cout << "Unable to find configuration file: " << CONFIG_DIR << "ConnectionServer.cfg" << std::endl;
        exit(-1);
    }

    /*try {
        LogManager::Init(
            static_cast<LogManager::LOG_PRIORITY>(gConfig->read<int>("ConsoleLog_MinPriority", 6)),
            static_cast<LogManager::LOG_PRIORITY>(gConfig->read<int>("FileLog_MinPriority", 6)),
            gConfig->read<std::string>("FileLog_Name", "connection_server.log"));
    } catch (...) {
        std::cout << "Unable to open log file for writing" << std::endl;
        exit(-1);
    }*/

    gConnectionServer = new ConnectionServer();

    // Main loop
    while(1)
    {
        gConnectionServer->Process();

        if(Anh_Utils::kbhit())
        {
            char input = std::cin.get();
            if(input == 'q')
                break;
            else if(input == 'l')
                gConnectionServer->ToggleLock();
        }


        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }

    // Shutdown things
    delete gConnectionServer;

    return 0;
}

//======================================================================================================================
