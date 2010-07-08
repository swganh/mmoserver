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

#include "LoginManager.h"

#include "NetworkManager/NetworkManager.h"
#include "NetworkManager/Service.h"

#include "LogManager/LogManager.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseManager.h"

#include "Common/MessageFactory.h"
#include "ConfigManager/ConfigManager.h"
#include "Utils/utils.h"

#if !defined(_DEBUG) && defined(_WIN32)
#include "Utils/mdump.h"
#endif

#include <boost/thread/thread.hpp>
#include "Utils/clock.h"

//======================================================================================================================
LoginServer* gLoginServer = 0;


//======================================================================================================================
LoginServer::LoginServer(void) :
mNetworkManager(0)
{
	// log msg to default log
  
  Anh_Utils::Clock::Init();
  gLogger->log(LogManager::INFORMATION, "Login Server Startup");

	// Initialize our modules.

	mNetworkManager = new NetworkManager();
	mService = mNetworkManager->GenerateService((char*)gConfig->read<std::string>("BindAddress").c_str(), gConfig->read<uint16>("BindPort"),gConfig->read<uint32>("ServiceMessageHeap")*1024,false);


	mDatabaseManager = new DatabaseManager();

	// Connect to our database and pass it off to our modules.
	mDatabase = mDatabaseManager->Connect(DBTYPE_MYSQL,
										 (char*)(gConfig->read<std::string>("DBServer")).c_str(),
										 gConfig->read<int>("DBPort"),
										 (char*)(gConfig->read<std::string>("DBUser")).c_str(),
										 (char*)(gConfig->read<std::string>("DBPass")).c_str(),
										 (char*)(gConfig->read<std::string>("DBName")).c_str());

  mDatabase->ExecuteSqlAsync(0,0,"UPDATE config_process_list SET serverstartID = serverstartID+1 WHERE name like 'login'");
  mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE config_process_list SET status=%u WHERE name='login';", 1));

  // In case of a crash, we need to cleanup the DB a little.
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE account SET authenticated=0 WHERE authenticated=1;"));

    //and session_key now as well
    mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE account SET session_key='';"));

	// Instant the messageFactory. It will also run the Startup ().
	(void)MessageFactory::getSingleton();		// Use this a marker of where the factory is instanced. 
												// The code itself here is not needed, since it will instance itself at first use.

	mLoginManager = new LoginManager(mDatabase);

	// Let our network Service know about our callbacks
	mService->AddNetworkCallback(mLoginManager);

	// We're done initializing.
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE config_process_list SET address='%s', port=%u, status=%u WHERE name='login';", mService->getLocalAddress(), mService->getLocalPort(), 2));

	gLogger->log(LogManager::CRITICAL, "Login Server startup complete");
	//gLogger->printLogo();
	// std::string BuildString(GetBuildString());	

	gLogger->log(LogManager::INFORMATION, "Login Server - Build %s", ConfigManager::getBuildString().c_str());
	gLogger->log(LogManager::CRITICAL,"Welcome to your SWGANH Experience!");
}


//======================================================================================================================
LoginServer::~LoginServer(void)
{
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE config_process_list SET status=%u WHERE name='login';", 0));
	gLogger->log(LogManager::CRITICAL, "LoginServer shutting down...");

	delete mLoginManager;

	mNetworkManager->DestroyService(mService);
	delete mNetworkManager;
	
	MessageFactory::getSingleton()->destroySingleton();	// Delete message factory and call shutdown();

	delete mDatabaseManager;

	gLogger->log(LogManager::CRITICAL, "LoginServer Shutdown complete");
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
    try {
	    ConfigManager::Init("LoginServer.cfg");
    } catch (file_not_found) {
        std::cout << "Unable to find configuration file: " << CONFIG_DIR << "LoginServer.cfg" << std::endl;
        exit(-1);
    }

    try {
	    LogManager::Init(
            static_cast<LogManager::LOG_PRIORITY>(gConfig->read<int>("ConsoleLog_MinPriority", 6)),
            static_cast<LogManager::LOG_PRIORITY>(gConfig->read<int>("FileLog_MinPriority", 6)),
            gConfig->read<std::string>("FileLog_Name", "login_server.log"));
    } catch (...) {
        std::cout << "Unable to open log file for writing" << std::endl;
        exit(-1);
    }

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




