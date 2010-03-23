/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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
	mContainerDepth= gWorldConfig->getConfiguration("Player_ContainerDepth",(uint16)5);
	if(mContainerDepth > 256)
		mContainerDepth= 256;
	else if(mContainerDepth< 3)
		mContainerDepth= 3;


	
	// Message of the day

	// Player viewing range
	mPlayerViewingRange = gWorldConfig->getConfiguration("Zone_Player_ViewingRange",(uint16)128);
	if(mPlayerViewingRange > 256)
		mPlayerViewingRange = 256;
	else if(mPlayerViewingRange < 32)
		mPlayerViewingRange = 32;

	// Player chat range
	mPlayerChatRange = gWorldConfig->getConfiguration("Zone_Player_ChatRange",(uint16)128);
	
	if(mPlayerChatRange > 256)
		mPlayerChatRange = 256;
	else if(mPlayerChatRange < 32)
		mPlayerChatRange = 32;


	// Server Time Update Frequency
	
	mServerTimeInterval = gWorldConfig->getConfiguration("Server_Time_Interval",30);
	
	if(mServerTimeInterval < 10)
		mServerTimeInterval = 10;
	else if(mServerTimeInterval > 300)
		mServerTimeInterval = 300;

	// Server Time Speed

	mServerTimeSpeed = gWorldConfig->getConfiguration("Server_Time_Speed",0);

	if(mServerTimeSpeed < 0)
		mServerTimeSpeed = 0;
	else if(mServerTimeSpeed > 5000)
		mServerTimeSpeed = 5000;

	// ham regen
	mHealthRegenDivider = static_cast<float>(gWorldConfig->getConfiguration("Player_Health_RegenDivider",100));
	
	if(mHealthRegenDivider < 1.0f || mHealthRegenDivider > 500.0f)
		mHealthRegenDivider = 100.0f;

	mActionRegenDivider = static_cast<float>(gWorldConfig->getConfiguration("Player_Action_RegenDivider",100));
	if(mActionRegenDivider < 1.0f || mActionRegenDivider > 500.0f)
		mActionRegenDivider = 100.0f;

	mMindRegenDivider = static_cast<float>(gWorldConfig->getConfiguration("Player_Mind_RegenDivider",100));
	if(mMindRegenDivider < 1.0f || mMindRegenDivider > 500.0f)
		mMindRegenDivider = 100.0f;

	// incapacitation
	mPlayerMaxIncaps = static_cast<uint8>(gWorldConfig->getConfiguration("Player_Incapacitation",3));
	
	if(mPlayerMaxIncaps < 1 || mPlayerMaxIncaps > 50)
	{
		mPlayerMaxIncaps = 3;
	}

	mPlayerBaseIncapTime = gWorldConfig->getConfiguration("Player_Incap_Time",30);
	if(mPlayerBaseIncapTime < 1 || mPlayerBaseIncapTime > 300)
	{
		mPlayerBaseIncapTime = 300;
	}

	mIncapResetTime = gWorldConfig->getConfiguration("Player_Incap_Reset",300);
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

	mGroupMissionUpdateTime = gWorldConfig->getConfiguration("Group_MissionUpdate_Time",10000);
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

	gLogger->logMsg("WorldConfig::adding Configuration: ");
	for(uint64 i = 0;i < count;i++)
	{
		result->GetNextRow(mConfigurationBinding,(void*)&attribute);
		gLogger->logMsgF("WorldConfig::adding Attribute %s :: %s ",MSG_NORMAL,attribute.mKey.getAnsi(),attribute.mValue.getAnsi());

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
		gLogger->logMsgLoadSuccess("WorldConfig:: %u attributes mapped...",MSG_NORMAL,count);
	}
	else
	{
		gLogger->logMsgLoadFailure("WorldConfig::mapping attributes...",MSG_NORMAL);					
	}

}

//=========================================================================

void WorldConfig::setConfiguration(string key,std::string value)
{
	ConfigurationMap::iterator it = mConfigurationMap.find(key.getCrc());

	if(it == mConfigurationMap.end())
	{
		gLogger->logMsgF("WorldConfig::setConfiguration: could not find %s",MSG_HIGH,key.getAnsi());
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
		gLogger->logMsgF("WorldConfig::removeConfiguration: could not find %s",MSG_HIGH,key.getAnsi());
}

//=========================================================================
