/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "LoginServer.h"
#include "LoginManager.h"
#include "NetworkManager/NetworkManager.h"
#include "NetworkManager/Service.h"
#include "DatabaseManager/DatabaseManager.h"
#include "DatabaseManager/Database.h"
#include "LogManager/LogManager.h"
#include "ConfigManager/ConfigManager.h"

#include "Common/MessageFactory.h"

#include <conio.h>


//======================================================================================================================
LoginServer* gLoginServer = 0;


//======================================================================================================================
LoginServer::LoginServer(void) :
mNetworkManager(0)
{

}


//======================================================================================================================
LoginServer::~LoginServer(void)
{

}


//======================================================================================================================
void LoginServer::Startup(void)
{
	// log msg to default log
  //gLogger->printSmallLogo();
  gLogger->logMsg("LoginServer Startup");
  // gLogger->logMsg(GetBuildString());
  gLogger->logMsg(ConfigManager::getBuildString());

	// Initialize our modules.

	mNetworkManager = new NetworkManager();
	mNetworkManager->Startup();
	mService = mNetworkManager->CreateService((char*)gConfig->read<std::string>("BindAddress").c_str(), gConfig->read<uint16>("BindPort"),gConfig->read<uint32>("ServiceMessageHeap")*1024,false);


	mDatabaseManager = new DatabaseManager();
	mDatabaseManager->Startup();

	// Connect to our database and pass it off to our modules.
	mDatabase = mDatabaseManager->Connect(DBTYPE_MYSQL,
										 (char*)(gConfig->read<std::string>("DBServer")).c_str(),
										 gConfig->read<int>("DBPort"),
										 (char*)(gConfig->read<std::string>("DBUser")).c_str(),
										 (char*)(gConfig->read<std::string>("DBPass")).c_str(),
										 (char*)(gConfig->read<std::string>("DBName")).c_str());

	gLogger->connecttoDB(mDatabaseManager);
	gLogger->createErrorLog("LoginServer",(LogLevel)(gConfig->read<int>("LogLevel",2)),
										(bool)(gConfig->read<bool>("LogToFile", true)),
										(bool)(gConfig->read<bool>("ConsoleOut",true)),
										(bool)(gConfig->read<bool>("LogAppend",true)));


  mDatabase->ExecuteSqlAsync(0,0,"UPDATE config_process_list SET serverstartID = serverstartID+1 WHERE name like 'login'");

  // In case of a crash, we need to cleanup the DB a little.
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE account SET authenticated=0 WHERE authenticated=1;"));

	// Instant the messageFactory. It will also run the Startup ().
	(void)MessageFactory::getSingleton();		// Use this a marker of where the factory is instanced. 
												// The code itself here is not needed, since it will instance itself at first use.

	mLoginManager = new LoginManager();
	mLoginManager->Startup(mDatabase);

	// Let our network Service know about our callbacks
	mService->AddNetworkCallback(mLoginManager);
	gLogger->logMsg("LoginServer Startup complete");
	//gLogger->printLogo();
	// std::string BuildString(GetBuildString());	
	std::string BuildString(ConfigManager::getBuildString());	

	gLogger->logMsgF("LoginServer %s",MSG_NORMAL,BuildString.substr(11,BuildString.size()).c_str());
	gLogger->logMsg("Welcome to your SWGANH Experience!");
}


//======================================================================================================================
void LoginServer::Shutdown(void)
{
	gLogger->logMsg("LoginServer shutting down...");

	mLoginManager->Shutdown();
	delete(mLoginManager);

	mNetworkManager->DestroyService(mService);
	mNetworkManager->Shutdown();
	mDatabaseManager->Shutdown();
	
	MessageFactory::getSingleton()->destroySingleton();	// Delete message factory and call shutdown();

	delete mNetworkManager;
	delete mDatabaseManager;

	gLogger->logMsg("LoginServer Shutdown complete\n");
}


//======================================================================================================================
void LoginServer::Process(void)
{
	mNetworkManager->Process();
	mDatabaseManager->Process();
	mLoginManager->Process();
}


//======================================================================================================================
void handleExit(void)
{
  gLoginServer->Shutdown();
  delete gLoginServer;
}


//======================================================================================================================
int main(int argc, char* argv)
{
  bool exit = false;

  // init our logmanager singleton,set global level normal, create the default log with normal priority, output to file + console, also truncate
  LogManager::Init(G_LEVEL_NORMAL,"LoginServer.log", LEVEL_NORMAL, true, true);

  // init out configmanager singleton (access configvariables with gConfig Macro,like: gConfig->readInto(test,"test");)
  ConfigManager::Init("LoginServer.cfg");

  gLoginServer = new LoginServer();

  // Startup things
	gLoginServer->Startup();

  // Since startup completed successfully, now set the atexit().  Otherwise we try to gracefully shutdown a failed startup, which usually fails anyway.
  //atexit(handleExit);

	// Main loop
	while (!exit)
	{
		gLoginServer->Process();
		msleep(1);

		if(kbhit())
			break;
	}

	// Shutdown things
	gLoginServer->Shutdown();
	delete gLoginServer;

  return 0;
}




