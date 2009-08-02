#pragma once

#ifndef _WINSOCK2API_
#include <WINSOCK2.h>
#endif
#include <iostream>
#include <boost/asio.hpp>
#include "NetworkManager/Packet.h"
#include "Utils/typedefs.h"
#include "LogManager/LogManager.h"
#include "PingServer.h"


using namespace std;
using boost::asio::ip::udp;

class ASynchPingSocket
{

public:

	ASynchPingSocket();
	~ASynchPingSocket(void);

	void				StartServer(int port);
	void				StartServer(boost::asio::ip::address address, int port);
	void				StopServer();
	void				SendPacket(char message[8192] , uint32 size, udp::endpoint remote_ip);

	long				totalSent;		//Total number of Bytes Sent
	long				totalReceived;	//Total number of Bytes Received

private:
	
	void StartServer();
	void StartReceive();
	void DoReceive(const boost::system::error_code& error, size_t size);
	void HandleSent(const boost::system::error_code &error, std::size_t size);
		
	bool				Started;		//Bool to tell if we are started

	udp::endpoint		server_ip;			//Server IP Endpoint

	udp::endpoint		client_ip;			//Storage for Current Client End Point (Thread Safe)
	
	udp::socket*		socket;				//Server Socket

	boost::asio::io_service*			io_service;			//Boost IO Service

	boost::array<char, MAX_PACKET_SIZE> recv_buffer;	//Storage for Current Receive Buffer (Thread Safe)

};
