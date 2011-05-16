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

#ifndef ANH_NETWORKMANAGER_NETWORKCONFIG_H
#define ANH_NETWORKMANAGER_NETWORKCONFIG_H

#include <stdint.h>

/**
 * \brief A catalog of available Network Configuration options.
 */
class NetworkConfig
{
public:
	/**
	 * \brief Initializes the configuration options.
	 */
	NetworkConfig(uint16_t reliable_size_server_to_server, uint16_t unreliable_size_server_to_server, uint16_t reliable_size_server_to_client, uint16_t unreliable_size_server_to_client, uint32_t server_packet_window, uint32_t client_packet_window, uint32_t udp_buffer_size) 
		: reliable_size_server_to_server_(reliable_size_server_to_server)
		, unreliable_size_server_to_server_(unreliable_size_server_to_server)
		, reliable_size_server_to_client_(reliable_size_server_to_client)
		, unreliable_size_server_to_client_(unreliable_size_server_to_client)
		, server_packet_window_(server_packet_window)
		, client_packet_window_(client_packet_window)
		, udp_buffer_size_(udp_buffer_size)
	{
	}

	/**
	 * \brief Default destructor.
	 */
	~NetworkConfig() { }

	const uint16_t getServerToServerReliableSize() const {
		return reliable_size_server_to_server_;
	}

	const uint16_t getServerToClientReliableSize() const {
		return reliable_size_server_to_client_;
	}

	const uint16_t getServerToServerUnreliableSize() const {
		return unreliable_size_server_to_server_;
	}

	const uint16_t getServerToClientUnreliableSize() const {
		return unreliable_size_server_to_client_;
	}

	const uint16_t getServerPacketWindow() const {
		return server_packet_window_;
	}

	const uint16_t getClientPacketWindow() const {
		return client_packet_window_;
	}

	const uint32_t getUdpBufferSize() const {
		return udp_buffer_size_;
	}

private:
	uint16_t	reliable_size_server_to_server_;
	uint16_t	unreliable_size_server_to_server_;
	uint16_t	reliable_size_server_to_client_;
	uint16_t	unreliable_size_server_to_client_;
	uint32_t	server_packet_window_;
	uint32_t	client_packet_window_;
	uint32_t	udp_buffer_size_;
};

#endif