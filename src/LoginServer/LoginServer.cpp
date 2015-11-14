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


#ifdef ERROR
#undef ERROR
#endif
#include "Utils/logger.h"

#include <iostream>
#include <fstream>

#include "LoginManager.h"
#include "Common/BuildInfo.h"

#include "NetworkManager/NetworkManager.h"
#include "NetworkManager/Service.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseManager.h"

#include "NetworkManager/MessageFactory.h"
#include "Utils/utils.h"

#include <boost/thread/thread.hpp>
#include "Utils/clock.h"

//======================================================================================================================
LoginServer* gLoginServer = 0;


//======================================================================================================================
LoginServer::LoginServer(int argc, char* argv[]) 
	: BaseServer()
    , mNetworkManager(0)
{
    Anh_Utils::Clock::Init();
    LOG(WARNING) << "Login Server Startup";

	// Load Configuration Options
	std::list<std::string> config_files;
	config_files.push_back("config/general.cfg");
	config_files.push_back("config/loginserver.cfg");
	LoadOptions_(argc, argv, config_files);

    // Initialize our modules.

	MessageFactory::getSingleton(configuration_variables_map_["GlobalMessageHeap"].as<uint32_t>());

	mNetworkManager = new NetworkManager( NetworkConfig(configuration_variables_map_["ReliablePacketSizeServerToServer"].as<uint16_t>(), 
		configuration_variables_map_["UnreliablePacketSizeServerToServer"].as<uint16_t>(), 
		configuration_variables_map_["ReliablePacketSizeServerToClient"].as<uint16_t>(), 
		configuration_variables_map_["UnreliablePacketSizeServerToClient"].as<uint16_t>(), 
		configuration_variables_map_["ServerPacketWindowSize"].as<uint32_t>(), 
		configuration_variables_map_["ClientPacketWindowSize"].as<uint32_t>(),
		configuration_variables_map_["UdpBufferSize"].as<uint32_t>()));

    LOG(WARNING) << "Config port set to " << configuration_variables_map_["BindPort"].as<uint16>();
    mService = mNetworkManager->GenerateService((char*)configuration_variables_map_["BindAddress"].as<std::string>().c_str(), configuration_variables_map_["BindPort"].as<uint16_t>(),configuration_variables_map_["ServiceMessageHeap"].as<uint32_t>()*1024,false);

	mDatabaseManager = new DatabaseManager(DatabaseConfig(configuration_variables_map_["DBMinThreads"].as<uint32_t>(), configuration_variables_map_["DBMaxThreads"].as<uint32_t>(), configuration_variables_map_["DBGlobalSchema"].as<std::string>(), configuration_variables_map_["DBGalaxySchema"].as<std::string>(), configuration_variables_map_["DBConfigSchema"].as<std::string>()));

    // Connect to our database and pass it off to our modules.
    mDatabase = mDatabaseManager->connect(DBTYPE_MYSQL,
                                          (char*)(configuration_variables_map_["DBServer"].as<std::string>()).c_str(),
                                          configuration_variables_map_["DBPort"].as<uint16_t>(),
                                          (char*)(configuration_variables_map_["DBUser"].as<std::string>()).c_str(),
                                          (char*)(configuration_variables_map_["DBPass"].as<std::string>()).c_str(),
                                          (char*)(configuration_variables_map_["DBName"].as<std::string>()).c_str());

    mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_ServerStatusUpdate('login', NULL, NULL, NULL);",mDatabase->galaxy()); // SQL - Update Server Start ID
    mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_ServerStatusUpdate('login', %u, NULL, NULL);",mDatabase->galaxy(), 1); // SQL - Update Server Status
    
    // In case of a crash, we need to cleanup the DB a little.
    mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.account SET account_authenticated = 0 WHERE account_authenticated = 1;",mDatabase->galaxy()));
    
    //and session_key now as well
    mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.account SET account_session_key = '';",mDatabase->galaxy()));
  
    // Instant the messageFactory. It will also run the Startup ().
    (void)MessageFactory::getSingleton();		// Use this a marker of where the factory is instanced.
    // The code itself here is not needed, since it will instance itself at first use.

    mLoginManager = new LoginManager(mDatabase);

    // Let our network Service know about our callbacks
    mService->AddNetworkCallback(mLoginManager);

    // We're done initializing.
    mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_ServerStatusUpdate('login', %u, '%s', %u);",mDatabase->galaxy(), 2, mService->getLocalAddress(), mService->getLocalPort()); // SQL - Update Server Details

    LOG(WARNING) << "Login Server startup complete";
    //gLogger->printLogo();
    // std::string BuildString(GetBuildString());

    LOG(WARNING) <<  "Login Server - Build " << GetBuildString().c_str();
    LOG(WARNING) << "Welcome to your SWGANH Experience!";
}


//======================================================================================================================
LoginServer::~LoginServer(void)
{
    mDatabase->executeProcedureAsync(0, 0, "CALL %s.sp_ServerStatusUpdate('login', %u, NULL, NULL);",mDatabase->galaxy(), 2); // SQL - Update server status
    
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
    mDatabaseManager->process();
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
    bool exit = false;

    try {
		gLoginServer = new LoginServer(argc, argv);

		// Since startup completed successfully, now set the atexit().  Otherwise we try to gracefully shutdown a failed startup, which usually fails anyway.
		//atexit(handleExit);

		// Main loop
		while (!exit)
		{
			gLoginServer->Process();

			if(Anh_Utils::kbhit())
				if(std::cin.get() == 'q')
					break;

			//boost::this_thread::sleep(boost::posix_time::milliseconds(10));
			usleep(2000);
		}

		// Shutdown things
		delete gLoginServer;
	} catch( std::exception& e ) {
		std::cout << e.what() << std::endl;
		std::cin.get();
		return 0;
	}

	return 0;
}




