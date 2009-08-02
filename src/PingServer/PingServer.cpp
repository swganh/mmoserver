/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "PingServer.h"
#include "ConfigManager/ConfigManager.h"
#include "LogManager/LogManager.h"

#include <boost/asio.hpp>

#include "ASynchPingSocket.h"
#include <conio.h>


#define RECEIVE_BUFFER 512


//======================================================================================================================

PingServer::PingServer()
{
}

//======================================================================================================================

PingServer::~PingServer()
{
}

//======================================================================================================================

void PingServer::start()
{
	std::string address;
	uint32 port = 0;

	gConfig->readInto(address,"BindAddress");
	gConfig->readInto(port,"BindPort");
	gLogger->logMsgF("PingServer listening at %s:%d",MSG_NORMAL,address.c_str(),port);


	//strcpy(mLocalAddressName,localAddress);
	//mLocalPort = localPort;
	
	boost::asio::ip::address pingAddress;
	pingAddress = boost::asio::ip::address::from_string(address.c_str());
	
	mAsynchPingThread = new ASynchPingSocket();
	mAsynchPingThread->StartServer(pingAddress,(int)port);

	}

//======================================================================================================================

void PingServer::shutdown()
{
	
}

//======================================================================================================================

void PingServer::process()
{
	
}

//======================================================================================================================

int main(int argc, char* argv)
{
	bool exit = false;

	LogManager::Init(G_LEVEL_NORMAL,"PingServer.log",LEVEL_NORMAL,false,true,false);
	ConfigManager::Init("PingServer.cfg");

	PingServer* pingServer = new PingServer();
	pingServer->start();

	while (!exit)
	{
		pingServer->process();
		msleep(1);

		if(kbhit())
			break;
	}

	pingServer->shutdown();
	delete(pingServer);

	return 0;
}

//======================================================================================================================

