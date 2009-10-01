/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_PING_SERVER_H
#define ANH_PING_SERVER_H

#include <string>
#include <vector>
#include <boost/asio.hpp>

#include "Utils/typedefs.h"

/*! \class PingServer PingServer/PingServer.h "PingServer/PingServer.h"
 *
 * \brief The Ping Server is lightweight echo service.
 *
 * The Ping Server is a lightweight echo service that responds to client requests
 * by echoing back submitted data. This is used by the SWG client to determine network
 * latency in requests to the remote server.
 */
class PingServer
{
public:
    /*! \brief Start a ping service listening on all addresses for a specified port.
     *
     * \param port The port to listen for ping requests on.
     */
    explicit PingServer(int port);

    /*! \brief Start a ping service on a specific address and port.
     *
     * \param address The address to listen for ping requests on.
     * \param port The port to listen for ping requests on.
     */
    PingServer(const std::string& address, int port);

    /*! Default destructor */
    ~PingServer();

    /*! \brief Polls the socket for any incoming data.
     *
     * This method polls the socket for incoming data and triggers a handler
     * if anything is waiting.
     */
    void Poll();

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

    std::vector<uint8>                  receive_buffer_;

    uint64                              bytes_received_;
    uint64                              bytes_sent_;
};

#endif // ANH_PING_SERVER_H

