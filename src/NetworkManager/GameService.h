/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_GAMESERVICE_H
#define ANH_NETWORKMANAGER_GAMESERVICE_H

#include <boost/asio.hpp>

#include "IService.h"
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

typedef std::queue<Session*>											SessionQueue;
typedef std::map<boost::asio::ip::udp::endpoint,Session*>				AddressSessionMap;
typedef std::list<NetworkCallback*>										NetworkCallbackList;

//======================================================================================================================

class GameService : public IService
{
	public:

		GameService(NetworkManager* networkManager, boost::asio::io_service* service);
		virtual ~GameService(void);

		void	Startup(std::string localAddress, uint16 localPort,uint32 mfHeapSize);
		void	Shutdown(void);
		void	Process();

		void	AddSessionToProcessQueue(Session* session);
		void	AddNetworkCallback(NetworkCallback* callback){ mNetworkCallbackList.push_back(callback); }

		std::string									getLocalAddress(void);
		uint16										getLocalPort(void);
		uint32										getId(void){ return mId; };
		void										setId(uint32 id){ mId = id; };
		boost::asio::ip::udp::socket*				getSocket() { return mSocket; }

	private:

		NetworkCallbackList					mNetworkCallbackList;
		SessionQueue						mSessionProcessQueue;
		NetworkManager*						mNetworkManager;
		uint32								mId;
		std::string							mLocalAddress;
		uint16								mLocalPort;
		bool								mQueued;
		AddressSessionMap					mAddressSessionMap;

		PacketFactory*						mPacketFactory;
		SessionFactory*						mSessionFactory;
		MessageFactory*						mMessageFactory;

		//==============
		// ASIO
		//
		boost::asio::io_service*			mIOService;
		boost::asio::ip::udp::socket*		mSocket;
		boost::asio::ip::udp::endpoint		mRecvEndpoint;

		//==============
		// Packet
		//
		Packet*								mRecvPacket;

		void	HandleRecvFrom( const boost::system::error_code& error, size_t bytesRecvd );
public: void	HandleSendTo( Session* source, Packet* msg );
};



//======================================================================================================================

#endif //ANH_NETWORKMANAGER_GAMESERVICE_



