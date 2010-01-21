/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "AdminServer.h"
#include "CharacterAdminHandler.h"
#include "PlanetMapHandler.h"

// External references

#include "NetworkManager/NetworkManager.h"
#include "NetworkManager/Service.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseManager.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/DispatchClient.h"
#include "Common/MessageDispatch.h"
#include "ConfigManager/ConfigManager.h"
#include "Utils/utils.h"

#include <boost/thread/thread.hpp>

#include <cstring>

//======================================================================================================================
AdminServer* gAdminServer;


//======================================================================================================================
AdminServer::AdminServer(void) :
mNetworkManager(0),
mDatabaseManager(0),
mRouterService(0),
mDatabase(0)
{

}


//======================================================================================================================
AdminServer::~AdminServer(void)
{

}


//======================================================================================================================
void AdminServer::Startup(void)
{
  //gLogger->printSmallLogo();
  // gLogger->logMsgF("AdminServer Startup : %s",MSG_HIGH,GetBuildString());
  gLogger->logMsgF("AdminServer Startup : %s",MSG_HIGH,ConfigManager::getBuildString().c_str());
  
  //gLogger->logMsg(GetBuildString());

  // Create and startup our core services.
  mDatabaseManager = new DatabaseManager();
  mDatabaseManager->Startup();

  
  mNetworkManager = new NetworkManager();
  mNetworkManager->Startup();


  // Connect to the DB and start listening for the RouterServer.
  mDatabase = mDatabaseManager->Connect(DBTYPE_MYSQL,
									   (char*)(gConfig->read<std::string>("DBServer")).c_str(),
									   gConfig->read<int>("DBPort"),
									   (char*)(gConfig->read<std::string>("DBUser")).c_str(),
									   (char*)(gConfig->read<std::string>("DBPass")).c_str(),
									   (char*)(gConfig->read<std::string>("DBName")).c_str());

  gLogger->connecttoDB(mDatabaseManager);
  gLogger->createErrorLog("AdminServer.log",(LogLevel)(gConfig->read<int>("LogLevel",2)),
										(bool)(gConfig->read<bool>("LogToFile", true)),
										(bool)(gConfig->read<bool>("ConsoleOut",true)),
										(bool)(gConfig->read<bool>("LogAppend",true)));


  mDatabase->ExecuteSqlAsync(0,0,"UPDATE config_process_list SET serverstartID = serverstartID+1 WHERE name like 'admin'");
  mRouterService = mNetworkManager->GenerateService((char*)gConfig->read<std::string>("BindAddress").c_str(), gConfig->read<uint16>("BindPort"),gConfig->read<uint32>("ServiceMessageHeap")*1024, true);

  // We need to register our IP and port in the DB so the connection server can connect to us.
  // Status:  0=offline, 1=loading, 2=online
  _updateDBServerList(1);

  // Connect to the ConnectionServer;
  _connectToConnectionServer();

  // Place all startup code here.
  mMessageDispatch = new MessageDispatch();
  mMessageDispatch->Startup(mRouterService);

  mCharacterAdminHandler = new CharacterAdminHandler();
  mCharacterAdminHandler->Startup(mDatabase, mMessageDispatch);
  
  mPlanetMapHandler = new PlanetMapHandler();
  mPlanetMapHandler->Startup(mDatabase,mMessageDispatch);

  // We're done initiailizing.
  _updateDBServerList(2);

  gLogger->logMsg("AdminServer::Startup Complete");
  //gLogger->printLogo();
  // std::string BuildString(GetBuildString());	
  
  gLogger->logMsgF("AdminServer %s", MSG_NORMAL, ConfigManager::getBuildString().c_str());
  gLogger->logMsg("Welcome to your SWGANH Experience!");
}


//======================================================================================================================
void AdminServer::Shutdown(void)
{
  gLogger->logMsg("AdminServer shutting down...");

   // We're shutting down, so update the DB again.
  _updateDBServerList(0);

  // Shutdown and delete the game modules.
  mCharacterAdminHandler->Shutdown();
  mPlanetMapHandler->Shutdown();
  mMessageDispatch->Shutdown();

  delete mCharacterAdminHandler;
  delete mPlanetMapHandler;
  delete mMessageDispatch;

  // Shutdown and delete our core services.
  mNetworkManager->DestroyService(mRouterService);
  mDatabaseManager->Shutdown();
  mNetworkManager->Shutdown();
 
  delete mNetworkManager;
  delete mDatabaseManager;

  gLogger->logMsg("AdminServer Shutdown Complete\n");
}


//======================================================================================================================
void AdminServer::Process(void)
{
  // Process our game modules
  mCharacterAdminHandler->Process();
  mPlanetMapHandler->Process();
  mMessageDispatch->Process();

  //  Process our core services
  mDatabaseManager->Process();
  mNetworkManager->Process();
}


//======================================================================================================================
void AdminServer::_updateDBServerList(uint32 status)
{
  // Update the DB with our status.  This must be synchronous as the connection server relies on this data.
  mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE config_process_list SET address='%s', port=%u, status=%u WHERE name='admin';", mRouterService->getLocalAddress(), mRouterService->getLocalPort(), status));
}


//======================================================================================================================
void AdminServer::_connectToConnectionServer(void)
{
  ProcessAddress processAddress;
  memset(&processAddress, 0, sizeof(ProcessAddress));

  // Query the DB to find out who this is.
  // setup our databinding parameters.
  DataBinding* binding = mDatabase->CreateDataBinding(5);
  binding->addField(DFT_uint32, offsetof(ProcessAddress, mType), 4);
  binding->addField(DFT_string, offsetof(ProcessAddress, mAddress), 16);
  binding->addField(DFT_uint16, offsetof(ProcessAddress, mPort), 2);
  binding->addField(DFT_uint32, offsetof(ProcessAddress, mStatus), 4);
  binding->addField(DFT_uint32, offsetof(ProcessAddress, mActive), 4);

  // Execute our statement
  DatabaseResult* result = mDatabase->ExecuteSynchSql("SELECT id, address, port, status, active FROM config_process_list WHERE name='connection';");
  uint64 count = result->getRowCount();

  // If we found them
  if (count == 1)
  {
    // Retrieve our routes and add them to the map.
    result->GetNextRow(binding, &processAddress);
  }

  // Delete our DB objects.
  mDatabase->DestroyDataBinding(binding);
  mDatabase->DestroyResult(result);

  // Now connect to the ConnectionServer
  DispatchClient* client = new DispatchClient();
  std::cout << "We are trying to connect to a ConnectionServer at: " << processAddress.mAddress << " : " << processAddress.mPort << std::endl;
  mRouterService->Connect(client, processAddress.mAddress, processAddress.mPort);
}


//======================================================================================================================
void handleExit(void)
{
  gAdminServer->Shutdown();
  delete gAdminServer;
}


//======================================================================================================================
int main(int argc, char* argv[])
{
	bool exit = false;
	LogManager::Init(G_LEVEL_NORMAL, "AdminServer.log", LEVEL_NORMAL, true, true);

	// init out configmanager singleton (access configvariables with gConfig Macro,like: gConfig->readInto(test,"test");)
	ConfigManager::Init("AdminServer.cfg");

	gAdminServer = new AdminServer();


	// Startup things
	gAdminServer->Startup();

  // Since startup completed successfully, now set the atexit().  Otherwise we try to gracefully shutdown a failed startup, which usually fails anyway.
 // atexit(handleExit);

  // Main loop
  while (!exit)
  {
	  gAdminServer->Process();

		if(Anh_Utils::kbhit())
		  break;

      boost::this_thread::sleep(boost::posix_time::milliseconds(1));
  }

	// Shutdown things
	gAdminServer->Shutdown();
	delete gAdminServer;

	return 0;
}


