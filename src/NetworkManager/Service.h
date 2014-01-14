/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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

		Service(NetworkManager* networkManager, bool serverservice, uint32 id, int8* localAddress, uint16 localPort,uint32 mfHeapSize);
		~Service(void);

		void	Process();

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

		NetworkCallbackList	mNetworkCallbackList;
		SessionQueue				mSessionProcessQueue;
		int8								mLocalAddressName[256];
		NetworkManager*			mNetworkManager;
		SocketReadThread*		mSocketReadThread;
		SocketWriteThread*	mSocketWriteThread;
		SOCKET				mLocalSocket;
		uint64							avgTime;
		uint64							lasttime;
		uint32              avgPacketsbuild;
		uint32							mId;
		uint32							mLocalAddress;
		uint32							mSessionResendWindowSize;
		uint16							mLocalPort;
		bool								mQueued;
		bool								mServerService;	//marks us as the serverservice / clientservice

		static bool					mSocketsSubsystemInitComplete;
};



//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SERVICE_



