/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include <conio.h>

#include "PingServer.h"
#include "ConfigManager/ConfigManager.h"
#include "LogManager/LogManager.h"

#define RECEIVE_BUFFER 512

PingServer::PingServer(int port)
    : io_service_()
    , socket_(io_service_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
    , receive_buffer_(RECEIVE_BUFFER)
{}

PingServer::PingServer(const std::string& address, int port)
    : io_service_()
    , socket_(io_service_, boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(address), port))
    , receive_buffer_(RECEIVE_BUFFER)
{}

PingServer::~PingServer()
{}

void PingServer::Poll()
{
    io_service_.poll();
}

uint64 PingServer::BytesReceived() const
{
    return bytes_received_;
}

uint64 PingServer::BytesSent() const
{
    return bytes_sent_;
}

void PingServer::AsyncReceive()
{
    // Listen for an incoming message. When a message is received call HandleReceive.
    socket_.async_receive_from(
        boost::asio::buffer(receive_buffer_),
        remote_endpoint_,
        std::tr1::bind(
            &PingServer::HandleReceive, 
            this, 
            std::tr1::placeholders::_1, 
            std::tr1::placeholders::_2
        )
    );  
}

void PingServer::HandleReceive(const boost::system::error_code& error, size_t bytesReceived)
{
    bytes_received_ += bytesReceived;  

    // Check if an error occurred.
    if (error && error != boost::asio::error::message_size) {
        std::cout << "Error reading from socket: " << error << std::endl;
        
    // Otherwise return the ping response to the sender.
    } else {
        // Send the message that was just received back to the sender.
        socket_.async_send_to(
            boost::asio::buffer(&receive_buffer_[0], bytesReceived), 
            remote_endpoint_,
            std::tr1::bind(
                &PingServer::HandleSend, 
                this, 
                std::tr1::placeholders::_1, 
                std::tr1::placeholders::_2
            )
        );
    }

    // Now that the message has been handled start listening for new requests.
    AsyncReceive();
}

void PingServer::HandleSend(const boost::system::error_code& error, size_t bytesSent)
{
    bytes_sent_ += bytesSent;
}


//======================================================================================================================


int main(int argc, char* argv[])
{
	LogManager::Init(G_LEVEL_NORMAL, "PingServer.log", LEVEL_NORMAL, false, true, false);
	ConfigManager::Init("PingServer.cfg");

	std::string BuildString(ConfigManager::getBuildString());	
	gLogger->logMsgF("PingServer %s", MSG_NORMAL, BuildString.substr(11,BuildString.size()).c_str());

    // Read in the address and port to start the ping server on.
    std::string address = gConfig->read<std::string>("BindAddress");
	int port            = gConfig->read<int>("BindPort");

    // Start the ping server.
	PingServer ping_server(address, port);
	gLogger->logMsgF("PingServer listening at %s:%d", MSG_NORMAL, address.c_str(), port);

    while (true) {
        // Check for incoming messages and handle them.
        ping_server.Poll();

        // Stop the ping server if a key is hit.
        if (kbhit()) break;
	}

	return 0;
}

//======================================================================================================================

