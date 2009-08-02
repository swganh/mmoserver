#include "ASynchPingSocket.h"
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::udp;


ASynchPingSocket::ASynchPingSocket()
{
	io_service = new boost::asio::io_service();
	Started = false;
}

ASynchPingSocket::~ASynchPingSocket(void)
{
	StopServer();
	delete io_service;
	io_service = 0;
}

//Stops the Server
void ASynchPingSocket::StopServer()
{ 
	if(Started)
	{
		io_service->stop();
		delete socket;
		socket = 0;
		Started = false;
	}
}

// Starts a server listening on a specified IP and Port
void ASynchPingSocket::StartServer(boost::asio::ip::address address, int port)
{
	if(!Started)
	{
		server_ip = udp::endpoint(address, port);
		gLogger->logMsgF("Asynch Start receive : %s %u", MSG_HIGH, address.to_string().c_str(),port);   
		StartServer();
	}
}
// Starts a server listening on a specified Port (Any IPv4 Interface. Doesn't work on Mutlihomed OS such as Vista or Server 2008)
void ASynchPingSocket::StartServer(int port)
{
	if(!Started)
	{
		server_ip = udp::endpoint(udp::v4(), port);
		StartServer();
	}
}
void ASynchPingSocket::StartServer()
{
	socket = new udp::socket(*io_service, server_ip);
	Started = true;
	StartReceive();

	boost::thread netlib(boost::bind(&boost::asio::io_service::run, io_service));
}

//Sets the socket ready to receive
void ASynchPingSocket::StartReceive()
{
	socket->async_receive_from(
		boost::asio::buffer(recv_buffer),
		client_ip,
		boost::bind(
			&ASynchPingSocket::DoReceive,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

//Socket has received data!
void ASynchPingSocket::DoReceive(const boost::system::error_code &error, std::size_t size)
{
	totalReceived += size;

	//Check an error hasn't occurred (Except message size)
	if (!error || error == boost::asio::error::message_size)
    {
		//Get the relevant info
		udp::endpoint ep = client_ip;
		//boost::array<char, MAX_PACKET_SIZE> message = recv_buffer;

		//restart checking for receive
		StartReceive();
		SendPacket(&recv_buffer[0,0],size,client_ip);
		

		
	} else {
		//We have an error situation
		cout << "Error on Socket: " << error << endl;
		StartReceive();
	}

	
}

//Socket has finished sending stuff
void ASynchPingSocket::HandleSent(const boost::system::error_code &error, std::size_t size)
{
	totalSent += size;
}

//Sent a packet to the Client
void ASynchPingSocket::SendPacket(char	message[8192], uint32 size, udp::endpoint remote_ip)
{
	gLogger->logMsgF("Asynch Send Packet", MSG_HIGH);   
	gLogger->hexDump(message,size);
	socket->async_send_to(
		boost::asio::buffer(message,size),
		remote_ip,
		boost::bind(
			&ASynchPingSocket::HandleSent,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}
