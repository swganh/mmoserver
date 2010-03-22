/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ZoneServer.h"
#include "AdminManager.h"
#include "BuffManager.h"
#include "CharacterLoginHandler.h"
#include "CharSheetManager.h"
#include "CombatManager.h"
#include "EntertainerManager.h"
#include "Food.h"
#include "ForageManager.h"
#include "GroupManager.h"
#include "MedicManager.h"
#include "NonPersistentItemFactory.h"
#include "NonPersistentNpcFactory.h"
#include "nonPersistantObjectFactory.h"
#include "NpcManager.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerDispatch.h"
#include "ObjectFactory.h"
#include "ScoutManager.h"
#include "SkillManager.h"
#include "StructureManager.h"
#include "TradeManager.h"
#include "TravelMapHandler.h"
#include "UIManager.h"
#include "WorldManager.h"
#include "WorldConfig.h"

// External references
#include "MessageLib/MessageLib.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"
#include "LogManager/LogManager.h"
#include "NetworkManager/NetworkManager.h"
#include "NetworkManager/Service.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseManager.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "ConfigManager/ConfigManager.h"
#include "Utils/utils.h"

#include <boost/thread/thread.hpp>

//======================================================================================================================

ZoneServer* gZoneServer = NULL;

//======================================================================================================================

ZoneServer::ZoneServer(int8* zoneName) :
mNetworkManager(0),
mDatabaseManager(0),
mRouterService(0),
mDatabase(0)
{
	//gLogger->printSmallLogo();
	// gLogger->logMsgF("ZoneServer - %s Startup %s",MSG_NORMAL,zoneName,GetBuildString());
	gLogger->logMsgF("ZoneServer - %s Startup %s",MSG_NORMAL,zoneName,ConfigManager::getBuildString().c_str());
	//gLogger->logMsg(GetBuildString());
	mZoneName = zoneName;

	// Create and startup our core services.
	mDatabaseManager = new DatabaseManager();

	mNetworkManager = new NetworkManager();

	// Connect to the DB and start listening for the RouterServer.
	mDatabase = mDatabaseManager->Connect(DBTYPE_MYSQL,
										   (int8*)(gConfig->read<std::string>("DBServer")).c_str(),
										   gConfig->read<int>("DBPort"),
										   (int8*)(gConfig->read<std::string>("DBUser")).c_str(),
										   (int8*)(gConfig->read<std::string>("DBPass")).c_str(),
										   (int8*)(gConfig->read<std::string>("DBName")).c_str());

	//make sure our logger has db access
	gLogger->connecttoDB(mDatabaseManager);

	//create an error log
	int8 log[128];
	sprintf(log,"%s.log",zoneName);
	gLogger->createErrorLog(log,(LogLevel)(gConfig->read<int>("LogLevel",2)),
										(bool)(gConfig->read<bool>("LogToFile", true)),
										(bool)(gConfig->read<bool>("ConsoleOut",true)),
										(bool)(gConfig->read<bool>("LogAppend",true)));

	//increase the server start that will help us to organize our logs to the corresponding serverstarts (mostly for errors)
	mDatabase->ExecuteSqlAsync(0,0,"UPDATE config_process_list SET serverstartID = serverstartID+1 WHERE name like \'%s\'",zoneName);

	mRouterService = mNetworkManager->GenerateService((char*)gConfig->read<std::string>("BindAddress").c_str(), gConfig->read<uint16>("BindPort"),gConfig->read<uint32>("ServiceMessageHeap")*1024,true);

	// Grab our zoneId out of the DB for this zonename.
	uint32 zoneId = 0;
	DatabaseResult* result = mDatabase->ExecuteSynchSql("SELECT planet_id FROM planet WHERE name=\'%s\';", zoneName);

	if (!result->getRowCount())
	{
		gLogger->logMsgF("FATAL: Map \'%s\' not found.  Aborting startup.", MSG_HIGH, zoneName);
		abort();
	}

	//  Yea, I'm getting annoyed with the DataBinding for such simple tasks.  Will implement a simple interface soon.
	gLogger->logMsgF("ZoneServer initializing for zone %s", MSG_NORMAL, zoneName);


	DataBinding* binding = mDatabase->CreateDataBinding(1);
	binding->addField(DFT_uint32, 0, 4);

	result->GetNextRow(binding, &zoneId);

	mDatabase->DestroyDataBinding(binding);
	mDatabase->DestroyResult(result);

	// We need to register our IP and port in the DB so the connection server can connect to us.
	// Status:  0=offline, 1=loading, 2=online
	_updateDBServerList(1);

	// Place all startup code here.
	mMessageDispatch = new MessageDispatch(mRouterService);

	WorldConfig::Init(zoneId,mDatabase,zoneName);
	ObjectControllerCommandMap::Init(mDatabase);
	MessageLib::Init();
	ObjectFactory::Init(mDatabase);
	
	//attribute commands for foodbuffs
	FoodCommandMapClass::Init();
	
	//structuremanager callback functions 
	StructureManagerCommandMapClass::Init();

	WorldManager::Init(zoneId,this,mDatabase);

	// Init the non persistent factories. For now we take them one-by-one here, until we have a collection of them.
	// We can NOT create these factories among the already existing ones, if we want to have any kind of "ownership structure",
	// since the existing factories are impossible to delete without crashing the server.
	// NonPersistentContainerFactory::Init(mDatabase);
	(void)NonPersistentItemFactory::Instance();	// This call is just for clarity, when matching the deletion of classes.
												// The object will create itself upon first usage,
	(void)NonPersistentNpcFactory::Instance();

	(void)ForageManager::Instance();
	(void)ScoutManager::Instance();
	(void)NonPersistantObjectFactory::Instance();

	UIManager::Init(mDatabase,mMessageDispatch);
	CombatManager::Init(mDatabase);
	TravelMapHandler::Init(mDatabase,mMessageDispatch,zoneId);
	CharSheetManager::Init(mDatabase,mMessageDispatch);
	TradeManager::Init(mDatabase,mMessageDispatch);
	BuffManager::Init(mDatabase);
	MedicManager::Init(mMessageDispatch);
	AdminManager::Init(mMessageDispatch);
	EntertainerManager::Init(mDatabase,mMessageDispatch);
	GroupManager::Init(mDatabase,mMessageDispatch);
	StructureManager::Init(mDatabase,mMessageDispatch);
	// Invoked when all creature regions for spawning of lairs are loaded
	// (void)NpcManager::Instance();


	ScriptEngine::Init();

	mCharacterLoginHandler = new CharacterLoginHandler(mDatabase,mMessageDispatch);

	mObjectControllerDispatch = new ObjectControllerDispatch(mDatabase,mMessageDispatch);
}

//======================================================================================================================

ZoneServer::~ZoneServer(void)
{
	gLogger->logMsg("ZoneServer shutting down...");

	// We're shutting down, so update the DB again.
	_updateDBServerList(0);

	// Shutdown and delete the game modules.
	delete mCharacterLoginHandler;
	gTravelMapHandler->Shutdown();
	gTradeManager->Shutdown();
	delete mObjectControllerDispatch;
	AdminManager::deleteManager();

	gWorldManager->Shutdown();	// Should be closed before script engine and script support, due to halting of scripts.
	gScriptEngine->shutdown();
	ScriptSupport::Instance()->destroyInstance();

	delete mMessageDispatch;

	// gMessageFactory->Shutdown(); // Nothing to do there yet, since deleting of the heap is done in the destructor.

	// Delete the non persistent factories, that are possible to delete.
	// NonPersistentContainerFactory::getSingletonPtr()->destroySingleton();
	NonPersistentItemFactory::deleteFactory();
	NonPersistentNpcFactory::deleteFactory();
	ScoutManager::deleteManager();
	WorldConfig::deleteManager();

	NonPersistantObjectFactory::deleteFactory();

	// Shutdown and delete our core services.
	mNetworkManager->DestroyService(mRouterService);
	delete mNetworkManager;

	delete mDatabaseManager;

	delete gSkillManager->getSingletonPtr();
	delete gMedicManager->getSingletonPtr();
	delete gBuffManager->getSingletonPtr();

	// NOW, I can feel that it should be safe to delete the data holding messages.
	gMessageFactory->destroySingleton();

	gLogger->logMsg("ZoneServer::Shutdown Complete\n");
}

//======================================================================================================================

void ZoneServer::handleWMReady()
{
	_updateDBServerList(2);
	gLogger->logMsg("ZoneServer::Startup Complete");
	//gLogger->printLogo();
	// std::string BuildString(GetBuildString());

	gLogger->logMsgF("ZoneServer:%s %s",MSG_NORMAL,getZoneName().getAnsi(),ConfigManager::getBuildString().c_str());
	gLogger->logMsg("Welcome to your SWGANH Experience!");

	// Connect to the ConnectionServer;
	_connectToConnectionServer();
}

//======================================================================================================================

void ZoneServer::Process(void)
{

	// Process our game modules
	mObjectControllerDispatch->Process();
	gWorldManager->Process();
	gScriptEngine->process();
	mMessageDispatch->Process();

	//is there stalling ?
	mRouterService->Process();

	//  Process our core services
	mDatabaseManager->Process();
	mNetworkManager->Process();
}

//======================================================================================================================

void ZoneServer::_updateDBServerList(uint32 status)
{
	// Update the DB with our status.  This must be synchronous as the connection server relies on this data.
	mDatabase->DestroyResult(mDatabase->ExecuteSynchSql("UPDATE config_process_list SET address='%s', port=%u, status=%u WHERE name='%s';", mRouterService->getLocalAddress(), mRouterService->getLocalPort(), status, mZoneName.getAnsi()));
}

//======================================================================================================================

void ZoneServer::_connectToConnectionServer(void)
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
	uint32 count = static_cast<uint32>(result->getRowCount());

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
	mRouterService->Connect(client, processAddress.mAddress, processAddress.mPort);

	// Send our registration message
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opClusterRegisterServer);
	gMessageFactory->addString(mZoneName);

	Message* message = gMessageFactory->EndMessage();
	client->SendChannelA(message, 0, CR_Connection, 1);
}

//======================================================================================================================

int main(int argc, char* argv[])
{
	// The second argument on the command line should be the zone name.
	//OnlyInstallUnhandeldExceptionFilter(); // Part of stackwalker
	char zone[50];
	if(argc < 2)
	{
		printf("A list of Zones can be found in ZoneList.txt\n");
		printf("Enter zone: ");
		int n = scanf("%s", zone);

		if (n != 1)
			std::exit(-1);

		// Remove the // if it cause problems, This enables you to just type
		// inn the zone insted of navigate using comand prompt.
		//std::exit(-1);
	}
	else
	{
		sprintf(zone,"%s",argv[1]);
	}

	int8 logfileName[64], configfileName[64];
	sprintf(logfileName, "%s.log", zone);
	sprintf(configfileName, "%s.cfg", zone);

	LogManager::Init(G_LEVEL_NORMAL,logfileName,LEVEL_NORMAL,true,true);
	ConfigManager::Init(configfileName);

	// Start things up
	gZoneServer = new ZoneServer((int8*)(gConfig->read<std::string>("ZoneName")).c_str());

	// Main loop
	while(1)
	{
		if (Anh_Utils::kbhit() || AdminManager::Instance()->shutdownZone())
		{
			break;
		}

		gZoneServer->Process();
		gMessageFactory->Process(); //Garbage Collection

        boost::this_thread::sleep(boost::posix_time::milliseconds(1));

	}

	// Shut things down

	delete gZoneServer;
	gZoneServer = NULL;

	return 0;
}

//======================================================================================================================







