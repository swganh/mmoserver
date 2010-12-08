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

#include <iostream>
#include <fstream>

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseManager.h"

#include "NetworkManager/MessageFactory.h"
#include "Utils/utils.h"
#include "Utils/clock.h"

//#include "stackwalker.h"
#include <boost/thread/thread.hpp>

//======================================================================================================================

ConnectionServer* gConnectionServer = 0;

//======================================================================================================================

ConnectionServer::ConnectionServer(int argc, char* argv[]) :
	options_description_("Connection Server Configuration"),
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
    LOG(WARNING) << "ConnectionServer Startup";

	options_description_.add_options()
		("help", "Displays this help dialog.")
		("BindAddress", boost::program_options::value<std::string>()->default_value("127.0.0.1"), "")
		("BindPort", boost::program_options::value<uint16_t>()->default_value(44991), "")
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
		("ClientServiceMessageHeap", boost::program_options::value<uint32_t>()->default_value(50000), "")
		("ServerServiceMessageHeap", boost::program_options::value<uint32_t>()->default_value(50000), "")
		("ClusterBindAddress", boost::program_options::value<std::string>()->default_value("127.0.0.1"), "")
		("ClusterBindPort", boost::program_options::value<uint16_t>()->default_value(5000))
		("ClusterId", boost::program_options::value<uint32_t>()->default_value(2))
	;

	std::ifstream ifs("config/ConnectionServer.cfg");
	if(!ifs) { throw std::runtime_error("Could not open the configuration file 'config/ConnectionServer.cfg'"); }

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

    // Startup our core modules
	MessageFactory::getSingleton(variables_map_["GlobalMessageHeap"].as<uint32_t>());

	mNetworkManager = new NetworkManager( NetworkConfig(variables_map_["ReliablePacketSizeServerToServer"].as<uint16_t>(), 
		variables_map_["UnreliablePacketSizeServerToServer"].as<uint16_t>(), 
		variables_map_["ReliablePacketSizeServerToClient"].as<uint16_t>(), 
		variables_map_["UnreliablePacketSizeServerToClient"].as<uint16_t>(), 
		variables_map_["ServerPacketWindowSize"].as<uint32_t>(), 
		variables_map_["ClientPacketWindowSize"].as<uint32_t>(),
		variables_map_["UdpBufferSize"].as<uint32_t>()));

    // Create our status service
    //clientservice
    mClientService = mNetworkManager->GenerateService((char*)variables_map_["BindAddress"].as<std::string>().c_str(), variables_map_["BindPort"].as<uint16_t>(),variables_map_["ClientServiceMessageHeap"].as<uint32_t>()*1024, false);//,5);
    //serverservice
    mServerService = mNetworkManager->GenerateService((char*)variables_map_["ClusterBindAddress"].as<std::string>().c_str(), variables_map_["ClusterBindPort"].as<uint16_t>(),variables_map_["ServerServiceMessageHeap"].as<uint32_t>()*1024, true);//,15);

	mDatabaseManager = new DatabaseManager(DatabaseConfig(variables_map_["DBMinThreads"].as<uint32_t>(), variables_map_["DBMaxThreads"].as<uint32_t>()));

    mDatabase = mDatabaseManager->connect(DBTYPE_MYSQL,
                                          (char*)(variables_map_["DBServer"].as<std::string>()).c_str(),
                                          variables_map_["DBPort"].as<uint16_t>(),
                                          (char*)(variables_map_["DBUser"].as<std::string>()).c_str(),
                                          (char*)(variables_map_["DBPass"].as<std::string>()).c_str(),
                                          (char*)(variables_map_["DBName"].as<std::string>()).c_str());

    mClusterId = variables_map_["ClusterId"].as<uint32_t>();

    mDatabase->executeProcedureAsync(0, 0, "CALL sp_GalaxyStatusUpdate(%u, %u);", 1, mClusterId); // Set status to online
    

    mDatabase->executeProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('connection', NULL, NULL, NULL);");
    

    // In case of a crash, we need to cleanup the DB a little.
    mDatabase->executeSynchSql("UPDATE account SET account_loggedin=0 WHERE account_loggedin=%u;", mClusterId);
    
    // Status:  0=offline, 1=loading, 2=online
    _updateDBServerList(1);

    // Startup our router modules.
    mConnectionDispatch = new ConnectionDispatch();
    mMessageRouter = new MessageRouter(mDatabase, mConnectionDispatch);
    mClientManager = new ClientManager(mClientService, mDatabase, mMessageRouter, mConnectionDispatch, mClusterId);
    mServerManager = new ServerManager(mServerService, mDatabase, mMessageRouter, mConnectionDispatch,mClientManager, mClusterId);

    // We're done initiailizing.
    _updateDBServerList(2);

    LOG(WARNING) << "Connection server startup complete";
}

//======================================================================================================================

ConnectionServer::~ConnectionServer(void)
{
    LOG(WARNING) << "ConnectionServer Shutting down...";

    // Update our status for the LoginServer
    mDatabase->executeProcedureAsync(0, 0, "CALL sp_GalaxyStatusUpdate(%u, %u);", 0, mClusterId); // Status set to offline
    

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
    mDatabase->executeProcedureAsync(0, 0, "CALL sp_ServerStatusUpdate('connection', %u, '%s', %u);", status, mServerService->getLocalAddress(), mServerService->getLocalPort());
    
}

//======================================================================================================================
void ConnectionServer::ToggleLock()
{
    mLocked = !mLocked;

    if(mLocked)
    {
        // Update our status for the LoginServer
        mDatabase->executeProcedureAsync(0, 0, "CALL sp_GalaxyStatusUpdate(%u, %u);", 3, mClusterId); // Status set to online (DEV / CSR Only)
        
        LOG(WARNING) << "Locking server to normal users";
    } else {
        // Update our status for the LoginServer
        mDatabase->executeProcedureAsync(0, 0, "CALL sp_GalaxyStatusUpdate(%u, %u);", 2, mClusterId); // Status set to online
        
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
		gConnectionServer = new ConnectionServer(argc, argv);

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
	} catch(std::exception& e) {
		std::cout << e.what() << std::endl;
		std::cin.get();
		return 0;
	}
    return 0;
}

//======================================================================================================================
