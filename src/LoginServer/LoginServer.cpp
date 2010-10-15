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

#include "LoginServer.h"

// Fix for issues with glog redefining this constant
#ifdef ERROR
#undef ERROR
#endif
#include <glog/logging.h>

#include "LoginManager.h"

#include "NetworkManager/NetworkManager.h"
#include "NetworkManager/Service.h"



#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseManager.h"

#include "NetworkManager/MessageFactory.h"
#include "Common/ConfigManager.h"
#include "Utils/utils.h"

#include <boost/thread/thread.hpp>
#include "Utils/clock.h"

//======================================================================================================================
LoginServer* gLoginServer = 0;


//======================================================================================================================
LoginServer::LoginServer(void) :
    mNetworkManager(0)
{
    Anh_Utils::Clock::Init();
    LOG(WARNING) << "Login Server Startup";

    // Initialize our modules.

    mNetworkManager = new NetworkManager();
    LOG(WARNING) << "Config port set to " << gConfig->read<uint16>("BindPort");
    mService = mNetworkManager->GenerateService((char*)gConfig->read<std::string>("BindAddress").c_str(), gConfig->read<uint16>("BindPort"),gConfig->read<uint32>("ServiceMessageHeap")*1024,false);

    mDatabaseManager = new DatabaseManager();

    // Connect to our database and pass it off to our modules.
    mDatabase = mDatabaseManager->Connect(DBTYPE_MYSQL,
                                          (char*)(gConfig->read<std::string>("DBServer")).c_str(),
                                          gConfig->read<int>("DBPort"),
                                          (char*)(gConfig->read<std::string>("DBUser")).c_str(),
                                          (char*)(gConfig->read<std::string>("DBPass")).c_str(),
                                          (char*)(gConfig->read<std::string>("DBName")).c_str());

    mDatabase->ExecuteProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('login', NULL, NULL, NULL);"); // SQL - Update Server Start ID
    mDatabase->ExecuteProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('login', %u, NULL, NULL);", 1); // SQL - Update Server Status
    
    

    // In case of a crash, we need to cleanup the DB a little.
    mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE account SET account_authenticated = 0 WHERE account_authenticated = 1;"));
    

    //and session_key now as well
    mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE account SET account_session_key = '';"));
  

    // Instant the messageFactory. It will also run the Startup ().
    (void)MessageFactory::getSingleton();		// Use this a marker of where the factory is instanced.
    // The code itself here is not needed, since it will instance itself at first use.

    mLoginManager = new LoginManager(mDatabase);

    // Let our network Service know about our callbacks
    mService->AddNetworkCallback(mLoginManager);

    // We're done initializing.
    mDatabase->ExecuteProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('login', %u, '%s', %u);", 2, mService->getLocalAddress(), mService->getLocalPort()); // SQL - Update Server Details

    LOG(WARNING) << "Login Server startup complete";
    //gLogger->printLogo();
    // std::string BuildString(GetBuildString());

    LOG(WARNING) <<  "Login Server - Build " << ConfigManager::getBuildString().c_str();
    LOG(WARNING) << "Welcome to your SWGANH Experience!";
}


//======================================================================================================================
LoginServer::~LoginServer(void)
{
    mDatabase->ExecuteProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('login', %u, NULL, NULL);", 2); // SQL - Update server status
    
    LOG(WARNING) << "LoginServer shutting down...";

    delete mLoginManager;

    mNetworkManager->DestroyService(mService);
    delete mNetworkManager;

    MessageFactory::getSingleton()->destroySingleton();	// Delete message factory and call shutdown();

    delete mDatabaseManager;

    LOG(WARNING) << "LoginServer Shutdown complete";
}

//======================================================================================================================
void LoginServer::Process(void)
{
    mNetworkManager->Process();
    mDatabaseManager->Process();
    mLoginManager->Process();
    gMessageFactory->Process();
}


//======================================================================================================================
void handleExit(void)
{
    delete gLoginServer;
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
  
    try {
        ConfigManager::Init("LoginServer.cfg");
    } catch (file_not_found) {
        std::cout << "Unable to find configuration file: " << CONFIG_DIR << "LoginServer.cfg" << std::endl;
        exit(-1);
    }

    /*try {
        LogManager::Init(
            static_cast<LogManager::LOG_PRIORITY>(gConfig->read<int>("ConsoleLog_MinPriority", 6)),
            static_cast<LogManager::LOG_PRIORITY>(gConfig->read<int>("FileLog_MinPriority", 6)),
            gConfig->read<std::string>("FileLog_Name", "login_server.log"));
    } catch (...) {
        std::cout << "Unable to open log file for writing" << std::endl;
        exit(-1);
    }*/
    
    //set stdout buffers to 0 to force instant flush
    setvbuf( stdout, NULL, _IONBF, 0);

    bool exit = false;

    //We cannot startup Database Logging until we startup the Database.

    gLoginServer = new LoginServer();

    // Since startup completed successfully, now set the atexit().  Otherwise we try to gracefully shutdown a failed startup, which usually fails anyway.
    //atexit(handleExit);

    // Main loop
    while (!exit)
    {
        gLoginServer->Process();

        if(Anh_Utils::kbhit())
            if(std::cin.get() == 'q')
                break;

        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }

    // Shutdown things
    delete gLoginServer;

    return 0;
}




