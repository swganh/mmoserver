/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_WORLDCONFIG_H
#define ANH_ZONESERVER_WORLDCONFIG_H

#include <map>
#include <boost/lexical_cast.hpp>

#include "DatabaseManager/DatabaseCallback.h"

#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include "anh/crc.h"

#define	gWorldConfig	WorldConfig::getSingletonPtr()


//======================================================================================================================
namespace swganh	{
namespace app
{
class SwganhKernel;
}
namespace database	{
class Database;
class DatabaseResult;
class DatabaseCallback;
}
}

//======================================================================================================================

typedef std::map<uint32,std::string>	 ConfigurationMap;

class Configuration_QueryContainer
{
public:

    Configuration_QueryContainer() {}

    int8		Key[128];
	int8		Value[128];
    //std::string	Value;
};

class WorldConfig : public swganh::database::DatabaseCallback
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

    static WorldConfig*	Init(uint32 zoneId,swganh::app::SwganhKernel* kernel, std::string zoneName);
    static WorldConfig*	getSingletonPtr() {
        return mSingleton;
    }

	void			setUp();

	virtual void	handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result);
    void			buildAttributeMap(swganh::database::DatabaseResult* result);

    // configuration attributes
    ConfigurationMap*			getConfigurationMap() {
        return &mConfigurationMap;
    }
    template<typename T> T		getConfiguration(std::string key, T fallback) const;
    template<typename T> T		getConfiguration(std::string key) const;
    template<typename T> T		getConfiguration(uint32 keyCrc) const;
    void						setConfiguration(std::string key,std::string value);
    void						addConfiguration(std::string key,std::string value);
    bool						hasConfiguration(std::string key) const;
    void						removeConfiguration(std::string key);


    uint32				getGroupMissionUpdateTime() {
        return mGroupMissionUpdateTime;
    }

    uint16				getPlayerContainerDepth() {
        return mContainerDepth;
    }

    uint16				getPlayerViewingRange() {
        return mPlayerViewingRange;
    }
    uint16				getPlayerChatRange() {
        return mPlayerChatRange;
    }

    uint32				getServerTimeInterval() {
        return mServerTimeInterval;
    }
    uint32				getServerTimeSpeed() {
        return mServerTimeSpeed;
    }

    uint8				getPlayerMaxIncaps() {
        return mPlayerMaxIncaps;
    }
    uint32				getBaseIncapTime() {
        return mPlayerBaseIncapTime;
    }
    uint32				getIncapResetTime() {
        return mIncapResetTime;
    }

    float				mHealthRegenDivider,mActionRegenDivider,mMindRegenDivider;
    bool				isTutorial() {
        return (mTutorialEnabled && (mZoneId == 41));
    }
    void				enableTutorial() {
        mTutorialEnabled = true;
    }
    void				disableTutorial() {
        mTutorialEnabled = false;
    }
    void				enableInstance() {
        mInstanceEnabled = true;
    }

    void				ResetViewingRange() {
        mPlayerViewingRange = mPlayerViewingRangeMax;
    }
    void				setViewingRange(uint16 range) {
        mPlayerViewingRange = range;
    }

    // For now, the Tutorial is the only instance we have, but we need to be able to expand on that concept.
    bool				isInstance();

private:

    WorldConfig(uint32 zoneId,swganh::app::SwganhKernel* kernel, std::string zoneName);

    ConfigurationMap		mConfigurationMap;
    static WorldConfig*		mSingleton;
    bool					mLoadComplete;

	swganh::app::SwganhKernel* 	mKernel;
    //swganh::database::Database*				mDatabase;
    uint32					mZoneId;
    std::string				mZoneName;

    //
    // configuration variables
    //
    // Player Viewing Range, the default range used for spatial queries
    uint16				mContainerDepth;

    // Player Viewing Range, the default range used for spatial queries
    uint16				mPlayerViewingRange;
    uint16				mPlayerViewingRangeMax;

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
T	WorldConfig::getConfiguration(std::string key, T fallback) const
{
    ConfigurationMap::const_iterator it = mConfigurationMap.find(swganh::memcrc(key));

    if(it != mConfigurationMap.end())
    {
        try
        {
            return(boost::lexical_cast<T>((*it).second));
        }
        catch(boost::bad_lexical_cast &)
        {
            
        }
    }
    //else
    //	gLogger->logErrorF("configuration","WorldConfig::getConfiguration: could not find %s - returning fallback",key.getAnsi());

    return(T(fallback));
}

template<typename T>
T	WorldConfig::getConfiguration(std::string key) const
{
    ConfigurationMap::const_iterator it = mConfigurationMap.find(swganh::memcrc(key));

    if(it != mConfigurationMap.end())
    {
        try
        {
            return(boost::lexical_cast<T>((*it).second));
        }
        catch(boost::bad_lexical_cast &)
        {
            
        }
    }

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
            
        }
    }

    return(T());
}


#endif

