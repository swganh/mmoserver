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

#include <iostream>
#include <fstream>

#include "LoginManager.h"
#include "Common\BuildInfo.h"

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
	: options_description_("Login Server Configuration")
    , mNetworkManager(0)
{
    Anh_Utils::Clock::Init();
    LOG(WARNING) << "Login Server Startup";

	options_description_.add_options()
		("help", "Displays this help dialog.")
		("BindAddress", boost::program_options::value<std::string>()->default_value("127.0.0.1"), "")
		("BindPort", boost::program_options::value<uint16_t>()->default_value(44990), "")
		("ServiceMessageHeap", boost::program_options::value<uint32_t>()->default_value(8192), "")
		("GlobalMessageHeap", boost::program_options::value<uint32_t>()->default_value(8192), "")
		("DBServer", boost::program_options::value<std::string>()->default_value("localhost"), "")
		("DBPort", boost::program_options::value<uint16_t>()->default_value(3306), "")
		("DBName", boost::program_options::value<std::string>()->default_value("swganh"), "")
		("DBUser", boost::program_options::value<std::string>()->default_value("root"), "")
		("DBPass", boost::program_options::value<std::string>()->default_value(""), "")
		("DBMinThreads", boost::program_options::value<uint32_t>()->default_value(2), "")
		("DBMaxThreads", boost::program_options::value<uint32_t>()->default_value(4), "")
		("ReliablePacketSizeServerToServer", boost::program_options::value<uint16_t>()->default_value(1400), "")
		("UnreliablePacketSizeServerToServer", boost::program_options::value<uint16_t>()->default_value(1400), "")
		("ReliablePacketSizeServerToClient", boost::program_options::value<uint16_t>()->default_value(496), "")
		("UnreliablePacketSizeServerToClient", boost::program_options::value<uint16_t>()->default_value(496), "")
		("ServerPacketWindowSize", boost::program_options::value<uint32_t>()->default_value(800), "")
		("ClientPacketWindowSize", boost::program_options::value<uint32_t>()->default_value(80), "")
		("UdpBufferSize", boost::program_options::value<uint32_t>()->default_value(4096), "")
	;

	std::ifstream ifs("config/LoginServer.cfg");
	if(!ifs) { throw std::runtime_error("Could not open the configuration file 'config/LoginServer.cfg'"); }

	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options_description_), variables_map_);
	boost::program_options::store(boost::program_options::parse_config_file(ifs, options_description_), variables_map_);
	boost::program_options::notify(variables_map_);

	// The help argument has been flagged, display the
	// server options and throw a runtime_error exception
	// to stop server startup.
	if(variables_map_.count("help"))
	{
		std::cout << options_description_ << std::endl;
		throw std::runtime_error("Help option flagged.");
	}

    // Initialize our modules.

	MessageFactory::getSingleton(variables_map_["GlobalMessageHeap"].as<uint32_t>());

	mNetworkManager = new NetworkManager( NetworkConfig(variables_map_["ReliablePacketSizeServerToServer"].as<uint16_t>(), 
		variables_map_["UnreliablePacketSizeServerToServer"].as<uint16_t>(), 
		variables_map_["ReliablePacketSizeServerToClient"].as<uint16_t>(), 
		variables_map_["UnreliablePacketSizeServerToClient"].as<uint16_t>(), 
		variables_map_["ServerPacketWindowSize"].as<uint32_t>(), 
		variables_map_["ClientPacketWindowSize"].as<uint32_t>(),
		variables_map_["UdpBufferSize"].as<uint32_t>()));

    LOG(WARNING) << "Config port set to " << variables_map_["BindPort"].as<uint16>();
    mService = mNetworkManager->GenerateService((char*)variables_map_["BindAddress"].as<std::string>().c_str(), variables_map_["BindPort"].as<uint16_t>(),variables_map_["ServiceMessageHeap"].as<uint32_t>()*1024,false);

	mDatabaseManager = new DatabaseManager(DatabaseConfig(variables_map_["DBMinThreads"].as<uint32_t>(), variables_map_["DBMaxThreads"].as<uint32_t>()));

    // Connect to our database and pass it off to our modules.
    mDatabase = mDatabaseManager->connect(DBTYPE_MYSQL,
                                          (char*)(variables_map_["DBServer"].as<std::string>()).c_str(),
                                          variables_map_["DBPort"].as<uint16_t>(),
                                          (char*)(variables_map_["DBUser"].as<std::string>()).c_str(),
                                          (char*)(variables_map_["DBPass"].as<std::string>()).c_str(),
                                          (char*)(variables_map_["DBName"].as<std::string>()).c_str());

    mDatabase->executeProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('login', NULL, NULL, NULL);"); // SQL - Update Server Start ID
    mDatabase->executeProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('login', %u, NULL, NULL);", 1); // SQL - Update Server Status
    
    // In case of a crash, we need to cleanup the DB a little.
    mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE account SET account_authenticated = 0 WHERE account_authenticated = 1;"));
    
    //and session_key now as well
    mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE account SET account_session_key = '';"));
  
    // Instant the messageFactory. It will also run the Startup ().
    (void)MessageFactory::getSingleton();		// Use this a marker of where the factory is instanced.
    // The code itself here is not needed, since it will instance itself at first use.

    mLoginManager = new LoginManager(mDatabase);

    // Let our network Service know about our callbacks
    mService->AddNetworkCallback(mLoginManager);

    // We're done initializing.
    mDatabase->executeProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('login', %u, '%s', %u);", 2, mService->getLocalAddress(), mService->getLocalPort()); // SQL - Update Server Details

    LOG(WARNING) << "Login Server startup complete";
    //gLogger->printLogo();
    // std::string BuildString(GetBuildString());

    LOG(WARNING) <<  "Login Server - Build " << GetBuildString().c_str();
    LOG(WARNING) << "Welcome to your SWGANH Experience!";
}


//======================================================================================================================
LoginServer::~LoginServer(void)
{
    mDatabase->executeProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('login', %u, NULL, NULL);", 2); // SQL - Update server status
    
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
    // Initialize the google logging.
    google::InitGoogleLogging(argv[0]);

#ifndef _WIN32
    google::InstallFailureSignalHandler();
#endif

    FLAGS_log_dir = "./logs";
    FLAGS_stderrthreshold = 1;
    
    //set stdout buffers to 0 to force instant flush
    setvbuf( stdout, NULL, _IONBF, 0);

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

			boost::this_thread::sleep(boost::posix_time::milliseconds(10));
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




