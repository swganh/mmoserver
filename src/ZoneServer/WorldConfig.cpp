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

#include "WorldConfig.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "ConfigManager/ConfigManager.h"

//======================================================================================================================

WorldConfig*	WorldConfig::mSingleton = NULL;

//======================================================================================================================

WorldConfig::WorldConfig(uint32 zoneId,Database* database, string zoneName) :
mDatabase(database),
mZoneId(zoneId),
mTutorialEnabled(true),
mInstanceEnabled(false)
{
	mZoneName = zoneName;
	mLoadComplete = false;

	//load globals 
	mDatabase->ExecuteSqlAsync(this,NULL,"SELECT csa.attribute,cs.value"
										 " FROM config_server cs"
										 " INNER JOIN config_server_attributes csa ON (csa.id = cs.config_attributes_id)"
										 " WHERE cs.server_name like 'all' ");

}

	


//======================================================================================================================

WorldConfig* WorldConfig::Init(uint32 zoneId,Database* database, string zoneName)
{
	if(!mSingleton)
	{
		mSingleton = new WorldConfig(zoneId,database, zoneName);
	}
	
	return mSingleton;
}

//======================================================================================================================

WorldConfig::~WorldConfig()
{
}

//======================================================================================================================

void WorldConfig::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	
	buildAttributeMap(result);

	// verify loaded settings, unless we cant do table level checks with crap mysql
	
	// Container Depth
	mContainerDepth= gWorldConfig->getConfiguration<uint16>("Player_ContainerDepth",(uint16)5);
	if(mContainerDepth > 256)
		mContainerDepth= 256;
	else if(mContainerDepth< 3)
		mContainerDepth= 3;


	
	// Message of the day

	// Player viewing range
	mPlayerViewingRange = gWorldConfig->getConfiguration<uint16>("Zone_Player_ViewingRange",(uint16)128);
	if(mPlayerViewingRange > 256)
		mPlayerViewingRange = 256;
	else if(mPlayerViewingRange < 32)
		mPlayerViewingRange = 32;

	//save our initial value to reset after scaling down due to load
	mPlayerViewingRangeMax = mPlayerViewingRange;

	// Player chat range
	mPlayerChatRange = gWorldConfig->getConfiguration<uint16>("Zone_Player_ChatRange",(uint16)128);
	
	if(mPlayerChatRange > 256)
		mPlayerChatRange = 256;
	else if(mPlayerChatRange < 32)
		mPlayerChatRange = 32;


	// Server Time Update Frequency
	
	mServerTimeInterval = gWorldConfig->getConfiguration<uint32>("Server_Time_Interval",(uint32)30);
	
	if(mServerTimeInterval < 10)
		mServerTimeInterval = 10;
	else if(mServerTimeInterval > 300)
		mServerTimeInterval = 300;

	// Server Time Speed

	mServerTimeSpeed = gWorldConfig->getConfiguration<uint32>("Server_Time_Speed",(uint32)0);

	if(mServerTimeSpeed < 0)
		mServerTimeSpeed = 0;
	else if(mServerTimeSpeed > 5000)
		mServerTimeSpeed = 5000;

	// ham regen
	mHealthRegenDivider = static_cast<float>(gWorldConfig->getConfiguration<float>("Player_Health_RegenDivider",(float)100.0));
	
	if(mHealthRegenDivider < 1.0f || mHealthRegenDivider > 500.0f)
		mHealthRegenDivider = 100.0f;

	mActionRegenDivider = static_cast<float>(gWorldConfig->getConfiguration<float>("Player_Action_RegenDivider",100));
	if(mActionRegenDivider < 1.0f || mActionRegenDivider > 500.0f)
		mActionRegenDivider = 100.0f;

	mMindRegenDivider = static_cast<float>(gWorldConfig->getConfiguration<float>("Player_Mind_RegenDivider",100));
	if(mMindRegenDivider < 1.0f || mMindRegenDivider > 500.0f)
		mMindRegenDivider = 100.0f;

	// incapacitation
	mPlayerMaxIncaps = static_cast<uint8>(gWorldConfig->getConfiguration<uint32>("Player_Incapacitation",(uint32)3));
	
	if(mPlayerMaxIncaps < 1 || mPlayerMaxIncaps > 50)
	{
		mPlayerMaxIncaps = 3;
	}

	mPlayerBaseIncapTime = gWorldConfig->getConfiguration<uint32>("Player_Incap_Time",30);
	if(mPlayerBaseIncapTime < 1 || mPlayerBaseIncapTime > 300)
	{
		mPlayerBaseIncapTime = 300;
	}

	mIncapResetTime = gWorldConfig->getConfiguration<uint32>("Player_Incap_Reset",300);
	if(mIncapResetTime < 1 || mIncapResetTime > 3600)
	{
		mIncapResetTime = 300;
	}
	
	//now load the zones specifics
	if(!mLoadComplete)
	{
		mLoadComplete = true;
		int8 sql[255];
		sprintf(sql,"SELECT csa.attribute,cs.value FROM config_server cs INNER JOIN config_server_attributes csa ON (csa.id = cs.config_attributes_id) WHERE cs.server_name like '%s' ",mZoneName.getAnsi());
		mDatabase->ExecuteSqlAsync(this,NULL,sql);
	}

	mGroupMissionUpdateTime = gWorldConfig->getConfiguration<uint32>("Group_MissionUpdate_Time",10000);
	if(mGroupMissionUpdateTime < 1000 || mGroupMissionUpdateTime > 60000)
	{
		mGroupMissionUpdateTime = 30000;
	}

}

//======================================================================================================================

bool WorldConfig::isInstance()
{
	return ( ((mZoneId == 0) && mInstanceEnabled) || isTutorial() );	// Make Corellia instanced
}	

void WorldConfig::buildAttributeMap(DatabaseResult* result)
{
	Configuration_QueryContainer	attribute;
	uint64							count = result->getRowCount();
	BStringVector					dataElements;

	DataBinding*					mConfigurationBinding;

	mConfigurationBinding = mDatabase->CreateDataBinding(2);
	mConfigurationBinding->addField(DFT_bstring,offsetof(Configuration_QueryContainer,mKey),64,0);
	mConfigurationBinding->addField(DFT_bstring,offsetof(Configuration_QueryContainer,mValue),128,1);
	
	//gLogger->log(LogManager::DEBUG,"Adding Attribute Configuration");
	
	for(uint64 i = 0;i < count;i++)
	{
		result->GetNextRow(mConfigurationBinding,(void*)&attribute);
		//gLogger->logCont(LogManager::DEBUG,"Adding Attribute %s: %s ",attribute.mKey.getAnsi(),attribute.mValue.getAnsi());
		

		if(hasConfiguration(attribute.mKey))
		{
			setConfiguration(attribute.mKey,std::string(attribute.mValue.getAnsi()));
		}
		else
		{
			addConfiguration(attribute.mKey,std::string(attribute.mValue.getAnsi()));
		}
	}

	if(count > 0)
	{
		gLogger->log(LogManager::NOTICE,"Mapped %u Server Attributes.",count);		
	}

}

//=========================================================================

void WorldConfig::setConfiguration(string key,std::string value)
{
	ConfigurationMap::iterator it = mConfigurationMap.find(key.getCrc());

	if(it == mConfigurationMap.end())
	{
		gLogger->log(LogManager::INFORMATION,"WorldConfig::setConfiguration: could not find %s",key.getAnsi());
		return;
	}

	(*it).second = value;
}

//=============================================================================

void WorldConfig::addConfiguration(string key,std::string value)
{
	mConfigurationMap.insert(std::make_pair(key.getCrc(),value));
}

//=============================================================================

bool WorldConfig::hasConfiguration(string key) const
{
	if(mConfigurationMap.find(key.getCrc()) != mConfigurationMap.end())
		return(true);

	return(false);
}

//=============================================================================

void WorldConfig::removeConfiguration(string key)
{
	ConfigurationMap::iterator it = mConfigurationMap.find(key.getCrc());

	if(it != mConfigurationMap.end())
		mConfigurationMap.erase(it);
	else
		gLogger->log(LogManager::INFORMATION,"WorldConfig::removeConfiguration: could not find %s",key.getAnsi());
}

//=========================================================================
