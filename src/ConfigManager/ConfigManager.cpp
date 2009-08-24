/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ConfigManager.h"
#include "LogManager/LogManager.h"
#include <string>

//======================================================================================================================

bool			ConfigManager::mInsFlag    = false;
ConfigManager*  ConfigManager::mSingleton  = NULL;

//======================================================================================================================

ConfigManager::ConfigManager(const std::string& name)
{
	gLogger->logMsgF("Loading ConfgFile [ %s ]",MSG_NORMAL,name.c_str());
	try
	{
		mConfigFile = new ConfigFile(CONFIG_DIR + name);
	}
	catch(file_not_found)
	{
		gLogger->logMsgFailed(15);
		gLogger->logMsgF("ConfigFile not found [ %s ], exiting...\n",MSG_HIGH,name.c_str());
		exit(-1);
	}

	if(name.length() <= 1)
	{
		gLogger->logMsgFailed(38-name.length());
	}
	else
	{
		gLogger->logMsgOk(42-name.length());	
	}
	//gLogger->logMsg("Successfully Loaded [" + name + "]");
}

//======================================================================================================================

ConfigManager*	ConfigManager::Init(const std::string& name)
{
	if(!mInsFlag)
	{
		mSingleton = new ConfigManager(name);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//======================================================================================================================

ConfigManager::~ConfigManager()
{
	mInsFlag = false;
	SAFE_DELETE(mConfigFile);
	//SAFE_DELETE(mSingleton);
}

//======================================================================================================================