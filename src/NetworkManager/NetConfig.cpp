/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "NetConfig.h"
#include "LogManager/LogManager.h"
#include "ConfigManager/ConfigManager.h"

//======================================================================================================================

bool			NetConfig::mInsFlag = false;
NetConfig*		NetConfig::mSingleton = NULL;

//======================================================================================================================

NetConfig::NetConfig() 
{

	 //packet sizes
	 mReliableSizeServerServer		= gConfig->read<uint16>("ReliablePacketSizeServerServer",1400);
	 mUnReliableSizeServerServer	= gConfig->read<int>("UnReliablePacketSizeServerServer",1400);

	 mReliableSizeServerClient		= gConfig->read<int>("ReliablePacketSizeServerClient",496);
	 mUnReliableSizeServerClient	= gConfig->read<int>("UnReliablePacketSizeServerClient",496);

	 mServerPacketWindow			= gConfig->read<int>("ServerPacketWindowSize",800);
	 mClientPacketWindow			= gConfig->read<int>("ClientPacketWindowSize",80);
	 //mMaxBazaarListing = gConfig->read<int>("BazaarMaxListing",35);

}


//======================================================================================================================

NetConfig* NetConfig::Init()
{
	if(mInsFlag == false)
	{
		mSingleton = new NetConfig();
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//======================================================================================================================

NetConfig::~NetConfig()
{
	mInsFlag = false;
	//delete(mSingleton);
}

//======================================================================================================================


