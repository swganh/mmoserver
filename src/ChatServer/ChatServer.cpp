/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ChatServer.h"

// External references
#include "Common/MessageDispatch.h"
#include "Common/DispatchClient.h"

#include "NetworkManager/NetworkManager.h"
#include "NetworkManager/Service.h"
#include "DatabaseManager/DatabaseManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "LogManager/LogManager.h"
#include "ConfigManager/ConfigManager.h"
#include "ChatManager.h"
#include "TradeManagerChat.h"
#include "GroupManager.h"
#include "CSRManager.h"
#include "Common/MessageFactory.h"

#include <conio.h>
#include <stdio.h>
#include <stddef.h>

//======================================================================================================================

ChatServer* gChatServer;

//======================================================================================================================

ChatServer::ChatServer() : mNetworkManager(0),mDatabaseManager(0),mRouterService(0),mDatabase(0)
{
}

//======================================================================================================================

ChatServer::~ChatServer()
{
}

//======================================================================================================================

void ChatServer::Startup()
{
	gLogger->printSmallLogo();
	gLogger->logMsg("ChatServer Startup\n");
	// gLogger->logMsg(GetBuildString());
	gLogger->logMsg(ConfigManager::getBuildString());

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


	mDatabase->ExecuteSqlAsync(0,0,"UPDATE config_process_list SET serverstartID = serverstartID+1 WHERE name like 'chat'");

	gLogger->connecttoDB(mDatabaseManager);
	gLogger->createErrorLog("chatserver.log",(LogLevel)(gConfig->read<int>("LogLevel",2)),
										(bool)(gConfig->read<bool>("LogToFile", true)),
										(bool)(gConfig->read<bool>("ConsoleOut",true)),
										(bool)(gConfig->read<bool>("LogAppend",true)));

	mRouterService = mNetworkManager->CreateService((char*)gConfig->read<std::string>("BindAddress").c_str(), gConfig->read<uint16>("BindPort"),gConfig->read<uint32>("ServiceMessageHeap")*1024,true);

	// We need to register our IP and port in the DB so the connection server can connect to us.
	// Status:  0=offline, 1=loading, 2=online
	_updateDBServerList(1);

	// Instant the messageFactory. It will also run the Startup ().
	(void)MessageFactory::getSingleton();		// Use this a marker of where the factory is instanced. 
												// The code itself here is not needed, since it will instance itself at first use.

	// Connect to the ConnectionServer;
	_connectToConnectionServer();

	// Place all startup code here.
	mMessageDispatch = new MessageDispatch();
	mMessageDispatch->Startup(mRouterService);

	// load up our ChatManager
	mChatManager = ChatManager::Init(mDatabase,mMessageDispatch);
	mTradeManagerChatHandler = TradeManagerChatHandler::Init(mDatabase,mMessageDispatch,mChatManager);
	mCSRManager = CSRManager::Init(mDatabase, mMessageDispatch, mChatManager);

	// load up GroupManager
	mGroupManager = GroupManager::Init(mMessageDispatch);

	ChatMessageLib::Init(mClient);

	// We're done initializing.
	_updateDBServerList(2);

	gLogger->logMsg("ChatServer::Startup Complete\n");
	gLogger->printLogo();
	// std::string BuildString(GetBuildString());
	std::string BuildString(ConfigManager::getBuildString());
	gLogger->logMsgF("ChatServer %s",MSG_NORMAL,BuildString.substr(11,BuildString.size()).c_str());
	gLogger->logMsg("Welcome to your SWGANH Experience!\n");
}

//======================================================================================================================

void ChatServer::Shutdown()
{
	gLogger->logMsg("ChatServer shutting down...\n");

	// We're shutting down, so update the DB again.
	_updateDBServerList(0);

	// Shutdown the various handlers
	delete (mChatManager);
	delete (mCSRManager);
	mTradeManagerChatHandler->Shutdown();
	delete (mTradeManagerChatHandler);

	// Shutdown and delete the game modules.
	mMessageDispatch->Shutdown();

	delete mMessageDispatch;

	// Shutdown and delete our core services.
	mNetworkManager->DestroyService(mRouterService);
	mNetworkManager->Shutdown();
	mDatabaseManager->Shutdown();

	MessageFactory::getSingleton()->destroySingleton();	// Delete message factory and call shutdown();

	delete mNetworkManager;
	delete mDatabaseManager;

	gLogger->logMsg("ChatServer Shutdown Complete\n");
}

//======================================================================================================================

void ChatServer::Process()
{
	// Process our game modules
	mMessageDispatch->Process();

	//  Process our core services
	mDatabaseManager->Process();
	mNetworkManager->Process();
	mTradeManagerChatHandler->Process();
}


//======================================================================================================================

void ChatServer::_updateDBServerList(uint32 status)
{
  // Update the DB with our status.  This must be synchronous as the connection server relies on this data.
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE config_process_list SET address='%s', port=%u, status=%u WHERE name='chat';", mRouterService->getLocalAddress(), mRouterService->getLocalPort(), status));
}

//======================================================================================================================

void ChatServer::_connectToConnectionServer()
{
	ProcessAddress processAddress;
	memset(&processAddress, 0, sizeof(ProcessAddress));

	// Query the DB to find out who this is.
	// setup our databinding parameters.
	DataBinding* binding = mDatabase->CreateDataBinding(5);
	binding->addField(DFT_uint32, offsetof(ProcessAddress, mType), 4);
	binding->addField(DFT_string, offsetof(ProcessAddress, mAddress), 1);
	binding->addField(DFT_uint16, offsetof(ProcessAddress, mPort), 2);
	binding->addField(DFT_uint32, offsetof(ProcessAddress, mStatus), 4);
	binding->addField(DFT_uint32, offsetof(ProcessAddress, mActive), 4);

	// Setup our statement
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
	mClient = new DispatchClient();
	mRouterService->Connect(mClient, processAddress.mAddress, processAddress.mPort);
}

//======================================================================================================================

void handleExit()
{
	gChatServer->Shutdown();
	delete gChatServer;
}

//======================================================================================================================

int main(int argc, char* argv)
{
	bool exit = false;

	LogManager::Init(G_LEVEL_NORMAL, "ChatServer.log", LEVEL_NORMAL, true, true);

	ConfigManager::Init("ChatServer.cfg");

	gChatServer = new ChatServer();

	// Startup things
	gChatServer->Startup();

  // Since startup completed successfully, now set the atexit().  Otherwise we try to gracefully shutdown a failed startup, which usually fails anyway.
  //atexit(handleExit);

	// Main loop
	while (!exit)
	{
		gChatServer->Process();
		msleep(1);

		if(kbhit())
			break;
	}

	// Shutdown things
	gChatServer->Shutdown();
	delete gChatServer;

	// Delete LogManager
	delete LogManager::getSingletonPtr();

	delete ConfigManager::getSingletonPtr();

	return 0;
}


