/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_ISERVICE_H
#define ANH_NETWORKMANAGER_ISERVICE_H

#include <boost/asio.hpp>
#include "Utils/typedefs.h"

class NetworkManager;
class NetworkCallback;
class Session;
class NetworkClient;
class Packet;

enum ServiceType
{
	SERVICE_TYPE_GAME = 1,
	SERVICE_TYPE_CLUSTER
};

class IService
{
public:
	IService( NetworkManager* networkManager, boost::asio::io_service& service ) { };
	virtual ~IService( void ) { };

	virtual void Startup( std::string localAddress, uint16 localPort,uint32 mfHeapSize ) = 0;
	virtual void Shutdown( void ) = 0;
	virtual void Process( void ) = 0;

	virtual std::string getLocalAddress( void ) = 0;
	virtual uint16 getLocalPort( void ) = 0;

	virtual void setId( uint32 ) = 0;
	virtual uint32 getId( void ) = 0;

	virtual void AddNetworkCallback( NetworkCallback* ) = 0;
};

#endif