/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef ANH_PING_SERVER_H
#define ANH_PING_SERVER_H

#include <string>
#include <vector>
#include <boost/asio.hpp>

#include "Common/Server.h"
#include "Utils/typedefs.h"

/*! \class PingServer PingServer/PingServer.h "PingServer/PingServer.h"
 *
 * \brief The Ping Server is lightweight echo service.
 *
 * The Ping Server is a lightweight echo service that responds to client requests
 * by echoing back submitted data. This is used by the SWG client to determine network
 * latency in requests to the remote server.
 */
class PingServer : public common::BaseServer
{
public:
    /*! \brief Start a ping service listening on all addresses for a specified port.
     *
     * \param argc The number of arguments in the command-line character array.
	 * \param argv Arguments from the command-line.
     */
    explicit PingServer(int argc, char* argv[]);

    /*! Default destructor */
    ~PingServer();

    /*! \brief Polls the socket for any incoming data.
     *
     * This method polls the socket for incoming data and triggers a handler
     * if anything is waiting.
     */
    void Process();

    /*! \returns Returns the bytes received since the Ping Server was started */
    uint64 BytesReceived() const;

    /*! \returns Returns the bytes sent since the Ping Server was started */
    uint64 BytesSent()     const;

private:
    /* Disable the default constructor, copy constructor and assignment operators */
    PingServer();
    PingServer(const PingServer&);
    PingServer& operator=(const PingServer&);

    void AsyncReceive();
    void HandleReceive(const boost::system::error_code& error, size_t bytesReceived);
    void HandleSend(const boost::system::error_code& error, size_t bytesSent);

    boost::asio::ip::udp::endpoint		remote_endpoint_;   //Storage for Current Client End Point (Thread Safe)
    boost::asio::io_service             io_service_;        //Boost IO Service
    boost::asio::ip::udp::socket        socket_;            //Server Socket
	uint16								server_port_;

    std::vector<uint8>                  receive_buffer_;

    uint64                              bytes_received_;
    uint64                              bytes_sent_;
};

#endif // ANH_PING_SERVER_H

