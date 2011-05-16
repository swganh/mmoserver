/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "PingServer.h"

// Fix for issues with glog redefining this constant
#ifdef ERROR
#undef ERROR
#endif

#include <glog/logging.h>

#include <boost/thread/thread.hpp>

#include "Common/BuildInfo.h"
#include "Utils/utils.h"

#include <iostream>
#include <fstream>
#include <functional>

#define RECEIVE_BUFFER 512

PingServer::PingServer(int argc, char* argv[])
    : BaseServer()
	, io_service_()
    , socket_(io_service_)
    , receive_buffer_(RECEIVE_BUFFER)
{	
	// Load Configuration Options
	std::list<std::string> config_files;
	config_files.push_back("config/general.cfg");
	config_files.push_back("config/pingserver.cfg");
	LoadOptions_(argc, argv, config_files);

	boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::udp::v4(), configuration_variables_map_["BindPort"].as<uint16_t>());
    socket_.open(endpoint.protocol());
    socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket_.bind(endpoint);


    AsyncReceive();
}

PingServer::~PingServer()
{}

void PingServer::Process()
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
        std::bind(
            &PingServer::HandleReceive,
            this,
            std::placeholders::_1,
            std::placeholders::_2
        )
    );
}

void PingServer::HandleReceive(const boost::system::error_code& error, size_t bytesReceived)
{
    bytes_received_ += bytesReceived;

    // Check if an error occurred.
    if (error && error != boost::asio::error::message_size) {
        LOG(WARNING) << "Error reading from socket: " << error.message().c_str();

        // Otherwise return the ping response to the sender.
    } else {
        // Send the message that was just received back to the sender.
        socket_.async_send_to(
            boost::asio::buffer(&receive_buffer_[0], bytesReceived),
            remote_endpoint_,
            std::bind(
                &PingServer::HandleSend,
                this,
                std::placeholders::_1,
                std::placeholders::_2
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
    // Initialize the google logging.
    google::InitGoogleLogging(argv[0]);

	#ifndef _WIN32
		google::InstallFailureSignalHandler();
	#endif

    FLAGS_log_dir = "./logs";
    FLAGS_stderrthreshold = 1;

    //set stdout buffers to 0 to force instant flush
    setvbuf( stdout, NULL, _IONBF, 0);
    
    LOG(WARNING) <<  "PingServer - Build " << GetBuildString().c_str();

    try {
		PingServer ping_server(argc, argv);

		LOG(WARNING) << "Welcome to your SWGANH Experience!";

		while (true) {
			// Check for incoming messages and handle them.
			ping_server.Process();
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));

			// Stop the ping server if a key is hit.
			if (Anh_Utils::kbhit())
				if(std::cin.get() == 'q')
					break;
		}

	} catch( std::exception& e ) {
		LOG(ERROR) << e.what();
		std::cin.get();
		return 0;
	}

    return 0;
}

//======================================================================================================================