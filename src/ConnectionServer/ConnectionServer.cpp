/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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
mServerService(0)
{
	// log msg to default log
	//gLogger->printSmallLogo();
	gLogger->logMsg("ConnectionServer Startup", FOREGROUND_GREEN | FOREGROUND_RED);
	
	// Startup our core modules
	mNetworkManager = new NetworkManager();

	// Create our status service
	//clientservice
	mClientService = mNetworkManager->GenerateService((char*)gConfig->read<std::string>("BindAddress").c_str(), gConfig->read<uint16>("BindPort"),gConfig->read<uint32>("ClientServiceMessageHeap")*1024, false);//,5);
	//serverservice
	mServerService = mNetworkManager->GenerateService((char*)gConfig->read<std::string>("ClusterBindAddress").c_str(), gConfig->read<uint16>("ClusterBindPort"),gConfig->read<uint32>("ServerServiceMessageHeap")*1024, true);//,15);

	mDatabaseManager = new DatabaseManager();

	mDatabase = mDatabaseManager->Connect(DBTYPE_MYSQL,
									   (char*)(gConfig->read<std::string>("DBServer")).c_str(),
									   gConfig->read<int>("DBPort"),
									   (char*)(gConfig->read<std::string>("DBUser")).c_str(),
									   (char*)(gConfig->read<std::string>("DBPass")).c_str(),
									   (char*)(gConfig->read<std::string>("DBName")).c_str());

	mClusterId = gConfig->read<uint32>("ClusterId");
	
	mDatabase->ExecuteSqlAsync(0, 0, "UPDATE galaxy SET status=1, last_update=NOW() WHERE galaxy_id=%u;", mClusterId);

	gLogger->connecttoDB(mDatabaseManager);
	gLogger->createErrorLog("connection.log",(LogLevel)(gConfig->read<int>("LogLevel",2)),
										(bool)(gConfig->read<bool>("LogToFile", true)),
										(bool)(gConfig->read<bool>("ConsoleOut",true)),
										(bool)(gConfig->read<bool>("LogAppend",true)));

	mDatabase->ExecuteSqlAsync(0,0,"UPDATE config_process_list SET serverstartID = serverstartID+1 WHERE name like 'connection'");
	// In case of a crash, we need to cleanup the DB a little.
	DatabaseResult* result = mDatabase->ExecuteSynchSql("UPDATE account SET loggedin=0 WHERE loggedin=%u;", mClusterId);
	mDatabase->DestroyResult(result);

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
	gLogger->logMsg("ConnectionServer::Server Boot Complete", FOREGROUND_GREEN);
	//gLogger->printLogo();
	// std::string BuildString(GetBuildString());	

	gLogger->logMsgF("ConnectionServer - Build %s",MSG_NORMAL,ConfigManager::getBuildString().c_str());
	gLogger->logMsg("Welcome to your SWGANH Experience!");
}

//======================================================================================================================

ConnectionServer::~ConnectionServer(void)
{
	gLogger->logMsg("ConnectionServer Shutting down...");

	// Update our status for the LoginServer
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE galaxy SET status=0 WHERE galaxy_id=%u;",mClusterId));

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

	gLogger->logMsg("ConnectionServer Shutdown Complete");
}

//======================================================================================================================

void ConnectionServer::Process(void)
{
	// Process our core services first.
	//mNetworkManager->Process();
	mDatabaseManager->Process();
	
	//we dont want this stalled by the clients!!!
	mServerService->Process();
	mClientService->Process();

	// Now process our sub modules
	gMessageFactory->Process();
	mClientManager->Process();
	mServerManager->Process();
	mMessageRouter->Process();
}

//======================================================================================================================

void ConnectionServer::_updateDBServerList(uint32 status)
{
	// Execute our query
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE config_process_list SET address='%s', port=%u, status=%u WHERE name='connection';",mServerService->getLocalAddress(), mServerService->getLocalPort(), status));
}

//======================================================================================================================

int main(int argc, char* argv[])
{
	// In release mode, catch any unhandled exceptions that may cause the program to crash and create a dump report.
#if !defined(_DEBUG) && defined(_WIN32)
	SetUnhandledExceptionFilter(CreateMiniDump);
#endif

	// init our logmanager singleton,set global level normal, create the default log with normal priority, output to file + console, also truncate
	LogManager::Init(G_LEVEL_NORMAL, "ConnectionServer.log", LEVEL_NORMAL, true, true);

	// init out configmanager singleton (access configvariables with gConfig Macro,like: gConfig->readInto(test,"test");)
	ConfigManager::Init("ConnectionServer.cfg");

	gConnectionServer = new ConnectionServer();

	// Main loop
	while(1)
	{
        gConnectionServer->Process();
     
				if(Anh_Utils::kbhit())
					if(std::cin.get() == 'q')
						break;

        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}

	// Shutdown things
	delete gConnectionServer;

	delete LogManager::getSingletonPtr();

	return 0;
}

//======================================================================================================================







