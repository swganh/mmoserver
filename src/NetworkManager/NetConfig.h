/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_NETCONFIG_H
#define ANH_ZONESERVER_NETCONFIG_H

#include "Utils/typedefs.h"

#define	gNetConfig	NetConfig::getSingletonPtr()


//======================================================================================================================


//======================================================================================================================

class NetConfig 
{
	public:

		~NetConfig();
		static NetConfig*	Init();
		static NetConfig*	getSingletonPtr() { return mSingleton; }

		uint16	getServerServerReliableSize(){ return mReliableSizeServerServer;}
		uint16	getServerClientReliableSize(){ return mReliableSizeServerClient;}
		uint16	getServerServerUnReliableSize(){ return mUnReliableSizeServerServer;}
		uint16	getServerClientUnReliableSize(){ return mUnReliableSizeServerClient;}

		uint32	getServerPacketWindow(){ return mServerPacketWindow;}
		uint32	getClientPacketWindow(){ return mClientPacketWindow;}
		
	private:

		NetConfig();

		static bool				mInsFlag;
		static NetConfig*		mSingleton;


		//
		// configuration variables
		//

		//Packetsize
		uint16					mReliableSizeServerServer;
		uint16					mUnReliableSizeServerServer;
		uint16					mReliableSizeServerClient;
		uint16					mUnReliableSizeServerClient;

		uint32					mServerPacketWindow;
		uint32					mClientPacketWindow;
};

#endif

