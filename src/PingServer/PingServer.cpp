/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/



#include "PingServer.h"

#include "LogManager/LogManager.h"

#include "ConfigManager/ConfigManager.h"
#include <boost/thread/thread.hpp>

#include "Utils/utils.h"

#if defined(__GNUC__)
// GCC implements tr1 in the <tr1/*> headers. This does not conform to the TR1
// spec, which requires the header without the tr1/ prefix.
#include <tr1/functional>
#else
#include <functional>
#endif

#include <iostream>

#define RECEIVE_BUFFER 512

PingServer::PingServer(int port)
    : io_service_()
    , socket_(io_service_)
    , receive_buffer_(RECEIVE_BUFFER)
{
	boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::udp::v4(), port);
	socket_.open(endpoint.protocol());
	socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	socket_.bind(endpoint); 

	AsyncReceive();
}

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
	LogManager::Init(G_LEVEL_NORMAL, "PingServer.log", LEVEL_NORMAL, true, true, false);
	ConfigManager::Init("PingServer.cfg");
	
	gLogger->logMsgF("PingServer %s", MSG_NORMAL, ConfigManager::getBuildString().c_str());

	// Read in the address and port to start the ping server on.
	int port            = gConfig->read<int>("BindPort");

    // Start the ping server.
	PingServer ping_server(port);
	gLogger->logMsgF("PingServer listening on port %d", MSG_NORMAL, port);

	while (true) {
		// Check for incoming messages and handle them.
		ping_server.Poll();
		boost::this_thread::sleep(boost::posix_time::milliseconds(1));

		// Stop the ping server if a key is hit.
		if (Anh_Utils::kbhit()) 
			if(std::cin.get() == 'q')
				break;
	}

	delete LogManager::getSingletonPtr();

	return 0;
}

//======================================================================================================================

