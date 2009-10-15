/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_SERVICE_H
#define ANH_NETWORKMANAGER_SERVICE_H

#include "Utils/typedefs.h"
#include "Utils/concurrent_queue.h"

#include <list>


//======================================================================================================================

class NetworkClient;
class Session;
class SocketReadThread;
class SocketWriteThread;
class NetworkManager;
class NetworkCallback;

//======================================================================================================================

typedef Anh_Utils::concurrent_queue<Session*>	SessionQueue;
typedef std::list<NetworkCallback*>				NetworkCallbackList;

//======================================================================================================================

class Service
{
	public:
		
		Service(NetworkManager* networkManager, bool serverservice);
		~Service(void);

		void	Startup(int8* localAddress, uint16 localPort,uint32 mfHeapSize);
		void	Shutdown(void);
		void	Process(uint32 loop);

		void	Connect(NetworkClient* client, int8* address, uint16 port);

		void	AddSessionToProcessQueue(Session* session);
		void	AddNetworkCallback(NetworkCallback* callback){ mNetworkCallbackList.push_back(callback); }

		int8*	getLocalAddress(void);
		uint16	getLocalPort(void);
		uint32	getId(void){ return mId; };

		void	setId(uint32 id){ mId = id; };
		void	setQueued(bool b){ mQueued = b; }
		bool	isQueued(){ return mQueued; }

	private:

		//marks us as the serverservice / clientservice
	bool						mServerService;
	uint64						lasttime;
	uint64						avgTime;
	uint32                      avgPacketsbuild;

		NetworkManager*		mNetworkManager;
		SocketReadThread*	mSocketReadThread;
		SocketWriteThread*	mSocketWriteThread;

		static bool			mSocketsSubsystemInitComplete;
		uint32				mId;

		int8				mLocalAddressName[256];
		uint32				mLocalAddress;
		uint16				mLocalPort;
		SOCKET				mLocalSocket;

		SessionQueue		mSessionProcessQueue;
		NetworkCallbackList	mNetworkCallbackList;
		uint32				mSessionResendWindowSize;

		bool				mQueued;

};



//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SERVICE_



