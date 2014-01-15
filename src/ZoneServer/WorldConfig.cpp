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

#include "ZoneServer/WorldConfig.h"

#include "anh/logger.h"

#include "DatabaseManager/Database.h"
//#include "DatabaseManager/"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include <cppconn/resultset.h>

#include "anh/plugin/bindings.h"
#include "anh/plugin/plugin_manager.h"

#include "anh/app/swganh_kernel.h"

//======================================================================================================================

WorldConfig*	WorldConfig::mSingleton = NULL;

//======================================================================================================================

WorldConfig::WorldConfig(uint32 zoneId,swganh::app::SwganhKernel* kernel, std::string zoneName) :
    mKernel(kernel),
    mZoneId(zoneId),
    mTutorialEnabled(true),
    mInstanceEnabled(false),
	mLoadComplete(false),
	mZoneName(zoneName)
{
	mKernel->GetDatabase()->executeSqlAsync(this,NULL,"SELECT csa.attribute,cs.value"
                               " FROM %s.config_server cs"
                               " INNER JOIN %s.config_server_attributes csa ON (csa.id = cs.config_attributes_id)"
                               " WHERE cs.server_name like 'all' ",
                               mKernel->GetDatabase()->galaxy(),mKernel->GetDatabase()->galaxy());

	LOG (error) << "started query";
}


void WorldConfig::setUp()
{
	//load globals
/*
	std::stringstream s;
	s	<< "SELECT csa.attribute,cs.value FROM " << mKernel->GetDatabase()->galaxy() << ".config_server cs INNER JOIN "
		<< mKernel->GetDatabase()->galaxy() << ".config_server_attributes csa ON (csa.id = cs.config_attributes_id)"
		<< " WHERE cs.server_name like 'all' ";
	
	mKernel->GetDatabase()->executeAsyncProcedure(s.str(), [=] (swganh::database::DatabaseResult* result)	{

		handleDatabaseResult(result);

		//now load the zones specifics
        std::stringstream s;
		s	<< "SELECT csa.attribute,cs.value FROM " << mKernel->GetDatabase()->galaxy() << ".config_server cs INNER JOIN " << mKernel->GetDatabase()->galaxy() 
			<< ".config_server_attributes csa ON (csa.id = cs.config_attributes_id) WHERE cs.server_name like '" << mZoneName << "'";
        
		mKernel->GetDatabase()->executeAsyncProcedure(s.str(), [=] (swganh::database::DatabaseResult* dresult)	{

			mSingleton->handleDatabaseResult(dresult);

		});
        
    

	});
	*/
}

//======================================================================================================================

WorldConfig* WorldConfig::Init(uint32 zoneId,swganh::app::SwganhKernel* kernel, std::string zoneName)
{
    if(!mSingleton)
    {
        mSingleton = new WorldConfig(zoneId, kernel, zoneName);
    }
    
	return mSingleton;
}

//======================================================================================================================

WorldConfig::~WorldConfig()
{
}

//======================================================================================================================

void	WorldConfig::handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result)
{
	LOG (error) << "build attribute map";
    buildAttributeMap(result);

    // verify loaded settings, unless we cant do table level checks with crap mysql

    // Container Depth
    mContainerDepth= getConfiguration<uint16>("Player_ContainerDepth",(uint16)5);
    if(mContainerDepth > 255)
        mContainerDepth= 255;
    else if(mContainerDepth< 3)
        mContainerDepth= 3;



    // Message of the day

    // Player viewing range
    mPlayerViewingRange = getConfiguration<uint16>("Zone_Player_ViewingRange",(uint16)128);
    if(mPlayerViewingRange > 256)
        mPlayerViewingRange = 256;
    else if(mPlayerViewingRange < 32)
        mPlayerViewingRange = 32;

    //save our initial value to reset after scaling down due to load
    mPlayerViewingRangeMax = mPlayerViewingRange;

    // Player chat range
    mPlayerChatRange = getConfiguration<uint16>("Zone_Player_ChatRange",(uint16)128);

    if(mPlayerChatRange > 256)
        mPlayerChatRange = 256;
    else if(mPlayerChatRange < 32)
        mPlayerChatRange = 32;


    // Server Time Update Frequency

    mServerTimeInterval = getConfiguration<uint32>("Server_Time_Interval",(uint32)30);

    if(mServerTimeInterval < 10)
        mServerTimeInterval = 10;
    else if(mServerTimeInterval > 300)
        mServerTimeInterval = 300;

    // Server Time Speed

    mServerTimeSpeed = getConfiguration<uint32>("Server_Time_Speed",(uint32)0);

    if(mServerTimeSpeed < 0)
        mServerTimeSpeed = 0;
    else if(mServerTimeSpeed > 5000)
        mServerTimeSpeed = 5000;

    // ham regen
    mHealthRegenDivider = static_cast<float>(getConfiguration<float>("Player_Health_RegenDivider",(float)100.0));

    if(mHealthRegenDivider < 1.0f || mHealthRegenDivider > 500.0f)
        mHealthRegenDivider = 100.0f;

    mActionRegenDivider = static_cast<float>(getConfiguration<float>("Player_Action_RegenDivider",100));
    if(mActionRegenDivider < 1.0f || mActionRegenDivider > 500.0f)
        mActionRegenDivider = 100.0f;

    mMindRegenDivider = static_cast<float>(getConfiguration<float>("Player_Mind_RegenDivider",100));
    if(mMindRegenDivider < 1.0f || mMindRegenDivider > 500.0f)
        mMindRegenDivider = 100.0f;

    // incapacitation
    mPlayerMaxIncaps = static_cast<uint8>(getConfiguration<uint32>("Player_Incapacitation",(uint32)3));

    if(mPlayerMaxIncaps < 1 || mPlayerMaxIncaps > 50)
    {
        mPlayerMaxIncaps = 3;
    }

    mPlayerBaseIncapTime = getConfiguration<uint32>("Player_Incap_Time",30);
    if(mPlayerBaseIncapTime < 1 || mPlayerBaseIncapTime > 300)
    {
        mPlayerBaseIncapTime = 300;
    }

    mIncapResetTime = getConfiguration<uint32>("Player_Incap_Reset",300);
    if(mIncapResetTime < 1 || mIncapResetTime > 3600)
    {
        mIncapResetTime = 300;
    }

    mGroupMissionUpdateTime = getConfiguration<uint32>("Group_MissionUpdate_Time",10000);
    if(mGroupMissionUpdateTime < 1000 || mGroupMissionUpdateTime > 60000)
    {
        mGroupMissionUpdateTime = 30000;
    }

	//now load the zones specifics
    if(!mLoadComplete)
    {
        mLoadComplete = true;
        int8 sql[255];
        sprintf(sql,"SELECT csa.attribute,cs.value FROM %s.config_server cs INNER JOIN %s.config_server_attributes csa ON (csa.id = cs.config_attributes_id) WHERE cs.server_name like '%s'",mKernel->GetDatabase()->galaxy(),mKernel->GetDatabase()->galaxy(),mZoneName);
		mKernel->GetDatabase()->executeSqlAsync(this,NULL,sql);
        
    }
}

//======================================================================================================================

bool WorldConfig::isInstance()
{
    return ( ((mZoneId == 0) && mInstanceEnabled) || isTutorial() );	// Make Corellia instanced
}

void WorldConfig::buildAttributeMap(swganh::database::DatabaseResult* result)
{
    Configuration_QueryContainer	attribute;
    uint64							count = result->getRowCount();
	std::stringstream				dataElements;

    swganh::database::DataBinding*	ConfigurationBinding;

	ConfigurationBinding = mKernel->GetDatabase()->createDataBinding(2);
	ConfigurationBinding->addField(swganh::database::DFT_string,offsetof(Configuration_QueryContainer,Key),128,0);
    ConfigurationBinding->addField(swganh::database::DFT_string,offsetof(Configuration_QueryContainer,Value),128,1);

	//std::string key;
	//std::string value;
    //gLogger->log(LogManager::DEBUG,"Adding Attribute Configuration");
	
	//std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

    for(uint64 i = 0; i < count; i++)
    {
		
        result->getNextRow(ConfigurationBinding,(void*)&attribute);
        LOG(error) << "Adding Attribute " << attribute.Key << " : " << attribute.Value;

		//key = result_set->getString("attribute");
		//value = result_set->getString("value");

        if(hasConfiguration(attribute.Key))
		//if(hasConfiguration(key))
        {
            setConfiguration(attribute.Key,attribute.Value);
			//setConfiguration(key, value);
        }
        else
        {
            addConfiguration(attribute.Key,attribute.Value);
			//addConfiguration(key,value);
        }
		
		/*if (!result_set->next())		{
			mKernel->GetDatabase()->destroyResult(result);
			return;
		}*/
    }

    LOG(info)  << "Mapped attributes mapped: [" << count << "]";

	mKernel->GetDatabase()->destroyDataBinding(ConfigurationBinding);

}

//=========================================================================

void WorldConfig::setConfiguration(std::string key,std::string value)
{
	ConfigurationMap::iterator it = mConfigurationMap.find(swganh::memcrc(key));

    if(it == mConfigurationMap.end())
    {
    	LOG(warning) << "Could not find configuration setting with key [" << key << "]";
        return;
    }

    (*it).second = value;
}

//=============================================================================

void WorldConfig::addConfiguration(std::string key,std::string value)
{
	mConfigurationMap.insert(std::make_pair(swganh::memcrc(key),value));
}

//=============================================================================

bool WorldConfig::hasConfiguration(std::string key) const
{
	if(mConfigurationMap.find(swganh::memcrc(key)) != mConfigurationMap.end())
        return(true);

    return(false);
}

//=============================================================================

void WorldConfig::removeConfiguration(std::string key)
{
	ConfigurationMap::iterator it = mConfigurationMap.find(swganh::memcrc(key));

    if(it != mConfigurationMap.end())
        mConfigurationMap.erase(it);
    else
    	LOG(warning) << "Could not find configuration setting with key [" << key << "]";
}

//=========================================================================
