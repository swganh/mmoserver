/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
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

#include "Utils/bstring.h"
#include "Utils/typedefs.h"

#include "DatabaseManager/DatabaseCallback.h"

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

    Configuration_QueryContainer() {}

    BString	mKey;
    BString	mValue;
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

    static WorldConfig*	Init(uint32 zoneId,Database* database, BString zoneName);
    static WorldConfig*	getSingletonPtr() {
        return mSingleton;
    }

    virtual void		handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    void				buildAttributeMap(DatabaseResult* result);

    // configuration attributes
    ConfigurationMap*			getConfigurationMap() {
        return &mConfigurationMap;
    }
    template<typename T> T		getConfiguration(BString key, T fallback) const;
    template<typename T> T		getConfiguration(BString key) const;
    template<typename T> T		getConfiguration(uint32 keyCrc) const;
    void						setConfiguration(BString key,std::string value);
    void						addConfiguration(BString key,std::string value);
    bool						hasConfiguration(BString key) const;
    void						removeConfiguration(BString key);


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

    // For now, the Tutorial is the only instance we have, but we need to be able to expand on that consept.
    bool				isInstance();

private:

    WorldConfig(uint32 zoneId,Database* database, BString zoneName);

    ConfigurationMap		mConfigurationMap;
    static WorldConfig*		mSingleton;
    bool					mLoadComplete;

    Database*				mDatabase;
    uint32					mZoneId;
    BString					mZoneName;

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
T	WorldConfig::getConfiguration(BString key, T fallback) const
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
            
        }
    }
    //else
    //	gLogger->logErrorF("configuration","WorldConfig::getConfiguration: could not find %s - returning fallback",key.getAnsi());

    return(T(fallback));
}

template<typename T>
T	WorldConfig::getConfiguration(BString key) const
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

