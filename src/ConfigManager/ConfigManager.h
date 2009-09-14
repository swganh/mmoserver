/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CONFIGMANAGER_H
#define ANH_CONFIGMANAGER_H

#include "Utils/typedefs.h"
#include "ConfigFile.h"


#define CONFIG_DIR "config//"
#define	gConfig	ConfigManager::getSingletonPtr()->getConfigFile()

//======================================================================================================================

class ConfigManager
{
	public:

		static ConfigManager*	getSingletonPtr() { return mSingleton; }
		static ConfigManager*	Init(const std::string& name);
		static const char*		getBuildString(void);
		static const char*		getBuildNumber(void);
		static const char*		getBuildTime(void);

		ConfigFile*				getConfigFile() { return mConfigFile; }
		~ConfigManager();

	private:

		ConfigManager(const std::string& name);

		static ConfigManager*	mSingleton;
		static bool				mInsFlag;

		ConfigFile*				mConfigFile;
};

//======================================================================================================================

#endif
