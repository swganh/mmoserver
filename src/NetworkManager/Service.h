/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_SERVICE_H
#define ANH_NETWORKMANAGER_SERVICE_H

#include <boost/asio.hpp>
#include "Utils/typedefs.h"
#include "Utils/concurrent_queue.h"
#include <list>

class MessageFactory;
class Packet;
class PacketFactory;
class SessionFactory;


//======================================================================================================================

class NetworkClient;
class Session;
class SocketReadThread;
class SocketWriteThread;
class NetworkManager;
class NetworkCallback;

//======================================================================================================================

typedef Anh_Utils::concurrent_queue<Session*>							SessionQueue;
typedef std::map<boost::asio::ip::udp::endpoint,Session*>				AddressSessionMap2;
typedef std::list<NetworkCallback*>										NetworkCallbackList;

//======================================================================================================================

class Service
{
	public:

		Service(NetworkManager* networkManager, bool serverservice);
		~Service(void);

		void	Startup(int8* localAddress, uint16 localPort,uint32 mfHeapSize);
		void	Shutdown(void);
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
		boost::asio::ip::udp::socket* getSocket() { return mSocket; }

	private:

		NetworkCallbackList					mNetworkCallbackList;
		SessionQueue						mSessionProcessQueue;
		int8								mLocalAddressName[256];
		NetworkManager*						mNetworkManager;
		SOCKET								mLocalSocket;
		uint64								avgTime;
		uint64								lasttime;
		uint32								avgPacketsbuild;
		uint32								mId;
		uint32								mLocalAddress;
		uint32								mSessionResendWindowSize;
		uint16								mLocalPort;
		bool								mQueued;
		bool								mServerService;	//marks us as the serverservice / clientservice
		AddressSessionMap2					mAddressSessionMap;
		static bool							mSocketsSubsystemInitComplete;

		PacketFactory*						mPacketFactory;
		SessionFactory*						mSessionFactory;
		MessageFactory*						mMessageFactory;

		//==============
		// ASIO
		//
		boost::asio::io_service				mIOService;
		boost::asio::ip::udp::socket*		mSocket;
		boost::asio::ip::udp::endpoint		mRecvEndpoint;

		//==============
		// Packet
		//
		Packet*								mRecvPacket;

		void	HandleRecvFrom( const boost::system::error_code& error, size_t bytesRecvd );
public: void	HandleSendTo( Packet* msg );
};



//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SERVICE_



