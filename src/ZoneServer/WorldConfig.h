/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_WORLDCONFIG_H
#define ANH_ZONESERVER_WORLDCONFIG_H

#include "Utils/typedefs.h"
#include "DatabaseManager/DatabaseCallback.h"
#include <map>
#include "LogManager/LogManager.h"
#include <boost/lexical_cast.hpp>

#define	gWorldConfig	WorldConfig::getSingletonPtr()


//======================================================================================================================

class Database;
class DatabaseCallback;
class DatabaseResult;

//======================================================================================================================

typedef std::map<uint32,std::string>	 ConfigurationMap;

class Configuration_QueryContainer
{
	public:

		Configuration_QueryContainer(){}

		string	mKey;
		string	mValue;
};

class WorldConfig : public DatabaseCallback
{
	public:

		~WorldConfig();

		static inline void deleteManager(void)    
		{ 
			if (mSingleton)
			{
				delete mSingleton;
				mSingleton = 0;
			}
		}

		static WorldConfig*	Init(uint32 zoneId,Database* database, string zoneName);
		static WorldConfig*	getSingletonPtr() { return mSingleton; }

		virtual void		handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void				buildAttributeMap(DatabaseResult* result);

		// configuration attributes
		ConfigurationMap*			getConfigurationMap(){ return &mConfigurationMap; }
		template<typename T> T		getConfiguration(string key, T fallback) const;
		template<typename T> T		getConfiguration(string key) const;
		template<typename T> T		getConfiguration(uint32 keyCrc) const;
		void						setConfiguration(string key,std::string value);
		void						addConfiguration(string key,std::string value);
		bool						hasConfiguration(string key) const;
		void						removeConfiguration(string key);


		uint32				getGroupMissionUpdateTime(){return mGroupMissionUpdateTime;}

		uint16				getPlayerViewingRange(){ return mPlayerViewingRange; }
		uint16				getPlayerChatRange(){ return mPlayerChatRange; }
		
		uint32				getServerTimeInterval(){ return mServerTimeInterval; }
		uint32				getServerTimeSpeed(){ return mServerTimeSpeed; }

		uint8				getPlayerMaxIncaps(){ return mPlayerMaxIncaps; }
		uint32				getBaseIncapTime(){ return mPlayerBaseIncapTime; }
		uint32				getIncapResetTime(){ return mIncapResetTime; }
			
		float				mHealthRegenDivider,mActionRegenDivider,mMindRegenDivider;
		bool				isTutorial() { return (mTutorialEnabled && (mZoneId == 41)); }
		void				enableTutorial() { mTutorialEnabled = true; }
		void				disableTutorial() { mTutorialEnabled = false; }
		void				enableInstance() { mInstanceEnabled = true; }


		// For now, the Tutorial is the only instance we have, but we need to be able to expand on that consept.
		bool				isInstance();

	private:

		WorldConfig(uint32 zoneId,Database* database, string zoneName);

		ConfigurationMap		mConfigurationMap;
		static WorldConfig*		mSingleton;
		bool					mLoadComplete;

		Database*				mDatabase;
		uint32					mZoneId;
		string					mZoneName;

		//
		// configuration variables
		//

		// Player Viewing Range, the default range used for spatial queries
		uint16				mPlayerViewingRange;

		// Player chat range
		uint16				mPlayerChatRange;

		// Logged Timeout, time until a disconnected player gets removed from the world
		uint32				mLoggedTime;

		// Server Time Update Frequency, how often time updates are send to players
		uint32				mServerTimeInterval;

		// Server Time Speed, add to the timecounter, adjusts how fast time goes by
		uint32				mServerTimeSpeed;

		// Server Weather Update Frequency, how often weather updates happen
		uint32				mWeatherUpdateInterval;

		// Maximum Weather type, max id for the randomizer to set a weathertype, differs for each planet
		uint8				mMaxWeatherId;

		// incapacitation
		uint8				mPlayerMaxIncaps;
		uint32				mPlayerBaseIncapTime;
		uint32				mIncapResetTime;

		// Test
		bool				mTutorialEnabled;
		bool				mInstanceEnabled;

		//Bazaar
		uint32				mMaxBazaarPrice;
		uint32				mMaxBazaarListing;

		// GroupMissionUpdateTime determines how often we update the waypoints for our group
		uint32				mGroupMissionUpdateTime;
};

//=============================================================================

template<typename T>
T	WorldConfig::getConfiguration(string key, T fallback) const
{
	ConfigurationMap::const_iterator it = mConfigurationMap.find(key.getCrc());

	if(it != mConfigurationMap.end())
	{
		try
		{
			return(boost::lexical_cast<T>((*it).second));
		}
		catch(boost::bad_lexical_cast &)
		{
			gLogger->logErrorF("configuration","WorldConfig::getConfiguration: cast failed (%s)",MSG_HIGH,key.getAnsi());
		}
	}
	else
		gLogger->logErrorF("configuration","WorldConfig::getConfiguration: could not find %s - returning fallback",MSG_HIGH,key.getAnsi());

	return(T(fallback));
}

template<typename T>
T	WorldConfig::getConfiguration(string key) const
{
	ConfigurationMap::const_iterator it = mConfigurationMap.find(key.getCrc());

	if(it != mConfigurationMap.end())
	{
		try
		{
			return(boost::lexical_cast<T>((*it).second));
		}
		catch(boost::bad_lexical_cast &)
		{
			gLogger->logErrorF("configuration","WorldConfig::getConfiguration: cast failed ('%s')",MSG_HIGH,key.getAnsi());
		}
	}
	else
		gLogger->logErrorF("configuration","WorldConfig::getConfiguration: couldn't find '%s'",MSG_HIGH,key.getAnsi());

	return(T());
}

//=============================================================================

template<typename T>
T	WorldConfig::getConfiguration(uint32 keyCrc) const
{
	ConfigurationMap::iterator it = mConfigurationMap.find(keyCrc);

	if(it != mConfigurationMap.end())
	{
		try
		{
			return(boost::lexical_cast<T>((*it).second));
		}
		catch(boost::bad_lexical_cast &)
		{
			gLogger->logErrorF("configuration","Object::getAttribute: cast failed ('%s')",MSG_HIGH,keyCrc);
		}
	}
	else
		gLogger->logErrorF("configuration","Object::getAttribute: couldn't find '%s'",MSG_HIGH,keyCrc);

	return(T());
}


#endif

