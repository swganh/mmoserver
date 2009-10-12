/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ConfigManager.h"
#include "LogManager/LogManager.h"
#include "Common/BuildInfo.h"

#include <string>


//======================================================================================================================


bool			ConfigManager::mInsFlag    = false;
ConfigManager*  ConfigManager::mSingleton  = NULL;

//======================================================================================================================

ConfigManager::ConfigManager(const std::string& name)
{
	try
	{
		mConfigFile = new ConfigFile(CONFIG_DIR + name);
	}
	catch(file_not_found)
	{
		gLogger->logMsgLoadFailure("ConfigManager::ConfigFile not found [ %s ], exiting...",MSG_HIGH,name.c_str());
		exit(-1);
	}

	gLogger->logMsgLoadSuccess("ConfigManager::loaded Configuration...",MSG_NORMAL);

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

std::string ConfigManager::getBuildString(void)
{
	return GetBuildString();
}


//======================================================================================================================

std::string ConfigManager::getBuildNumber(void)
{
	return GetBuildNumber();
}

//======================================================================================================================

std::string ConfigManager::getBuildTime(void)
{
	return GetBuildTime();
}

//======================================================================================================================
