/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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
  gLogger->logMsg("LoginServer Startup");

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

	gLogger->connecttoDB(mDatabaseManager);
	gLogger->createErrorLog("LoginServer",(LogLevel)(gConfig->read<int>("LogLevel",2)),
										(bool)(gConfig->read<bool>("LogToFile", true)),
										(bool)(gConfig->read<bool>("ConsoleOut",true)),
										(bool)(gConfig->read<bool>("LogAppend",true)));


  mDatabase->ExecuteSqlAsync(0,0,"UPDATE config_process_list SET serverstartID = serverstartID+1 WHERE name like 'login'");
  mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE config_process_list SET status=%u WHERE name='login';", 1));

  // In case of a crash, we need to cleanup the DB a little.
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE account SET authenticated=0 WHERE authenticated=1;"));

	// Instant the messageFactory. It will also run the Startup ().
	(void)MessageFactory::getSingleton();		// Use this a marker of where the factory is instanced. 
												// The code itself here is not needed, since it will instance itself at first use.

	mLoginManager = new LoginManager(mDatabase);

	// Let our network Service know about our callbacks
	mService->AddNetworkCallback(mLoginManager);

	// We're done initializing.
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE config_process_list SET address='%s', port=%u, status=%u WHERE name='login';", mService->getLocalAddress(), mService->getLocalPort(), 2));

	gLogger->logMsg("LoginServer Startup complete");
	//gLogger->printLogo();
	// std::string BuildString(GetBuildString());	

	gLogger->logMsgF("LoginServer - Build %s",MSG_NORMAL,ConfigManager::getBuildString().c_str());
	gLogger->logMsg("Welcome to your SWGANH Experience!");
}


//======================================================================================================================
LoginServer::~LoginServer(void)
{
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE config_process_list SET status=%u WHERE name='login';", 0));
	gLogger->logMsg("LoginServer shutting down...");

	delete mLoginManager;

	mNetworkManager->DestroyService(mService);
	delete mNetworkManager;
	
	MessageFactory::getSingleton()->destroySingleton();	// Delete message factory and call shutdown();

	delete mDatabaseManager;

	gLogger->logMsg("LoginServer Shutdown complete");
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
	// In release mode, catch any unhandled exceptions that may cause the program to crash and create a dump report.
#if !defined(_DEBUG) && defined(_WIN32)
	SetUnhandledExceptionFilter(CreateMiniDump);
#endif

  bool exit = false;

  // init our logmanager singleton,set global level normal, create the default log with normal priority, output to file + console, also truncate
  LogManager::Init(G_LEVEL_NORMAL,"LoginServer.log", LEVEL_NORMAL, true, true);

  // init out configmanager singleton (access configvariables with gConfig Macro,like: gConfig->readInto(test,"test");)
  ConfigManager::Init("LoginServer.cfg");

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

	delete LogManager::getSingletonPtr();

  return 0;
}




