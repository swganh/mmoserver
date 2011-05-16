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

#include "Common/Server.h"

#include <iostream>
#include <fstream>

/*! \brief Common is a catch-all library containing primarily base classes and
 * classes used for maintaining application lifetimes.
 */
namespace common {

BaseServer::BaseServer()
    : configuration_options_description_("Configuration Options")
{
    configuration_options_description_.add_options()
    ("help", "Displays this help dialog.")
    ("BindAddress", boost::program_options::value<std::string>()->default_value("127.0.0.1"), "Network listen address.")
    ("BindPort", boost::program_options::value<uint16_t>(), "Port the server listens for messages on.")
    ("ServiceMessageHeap", boost::program_options::value<uint32_t>()->default_value(8192), "")
    ("GlobalMessageHeap", boost::program_options::value<uint32_t>()->default_value(8192), "")
    ("DBServer", boost::program_options::value<std::string>()->default_value("localhost"), "Address of the MySQL Server.")
    ("DBPort", boost::program_options::value<uint16_t>()->default_value(3306), "Port of the MySQL Server.")
    ("DBName", boost::program_options::value<std::string>()->default_value("swganh"), "Name of the MySQL database schema.")
    ("DBUser", boost::program_options::value<std::string>()->default_value("root"), "Username of the database user account.")
    ("DBPass", boost::program_options::value<std::string>()->default_value(""), "Password of the database user account.")
    ("DBMinThreads", boost::program_options::value<uint32_t>()->default_value(4), "Minimum number of threads used for database work.")
    ("DBMaxThreads", boost::program_options::value<uint32_t>()->default_value(16), "Maximum number of threads used for database work.")
    ("ReliablePacketSizeServerToServer", boost::program_options::value<uint16_t>()->default_value(1400), "size of Packets for server server communication")
    ("UnreliablePacketSizeServerToServer", boost::program_options::value<uint16_t>()->default_value(1400), "size of Packets for server server communication")
    ("ReliablePacketSizeServerToClient", boost::program_options::value<uint16_t>()->default_value(496), "size of Packets for server client communication")
    ("UnreliablePacketSizeServerToClient", boost::program_options::value<uint16_t>()->default_value(496), "size of Packets for server client communication")
    ("ServerPacketWindowSize", boost::program_options::value<uint32_t>()->default_value(800), "")
    ("ClientPacketWindowSize", boost::program_options::value<uint32_t>()->default_value(80), "")
    ("UdpBufferSize", boost::program_options::value<uint32_t>()->default_value(4096), "Kernel UDP Buffer")
    ("DBGlobalSchema", boost::program_options::value<std::string>()->default_value("swganh_static"), "")
    ("DBGalaxySchema", boost::program_options::value<std::string>()->default_value("swganh"), "")
    ("DBConfigSchema", boost::program_options::value<std::string>()->default_value("swganh_config"), "")
    ;

}

BaseServer::~BaseServer()
{
}

bool BaseServer::Startup()
{
    return true;
}

void BaseServer::Shutdown()
{
}

void BaseServer::LoadOptions_(uint32_t argc, char* argv[])
{
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, configuration_options_description_), configuration_variables_map_);
    boost::program_options::notify(configuration_variables_map_);

    // The help argument has been flagged, display the
    // server options and throw a runtime_error exception
    // to stop server startup.
    if(configuration_variables_map_.count("help"))
    {
        std::cout << configuration_options_description_ << std::endl;
        throw std::runtime_error("Help option flagged.");
    }
}

void BaseServer::LoadOptions_(std::list<std::string> config_files)
{
    // Iterate through the configuration files
    // that are to be loaded. If a configuration file
    // is missing, throw a runtime_error.
    std::for_each(config_files.begin(), config_files.end(), [=] (const std::string& filename) {
        std::ifstream config_file(filename);
        if(!config_file)
            throw std::runtime_error("Could not open configuration file.");
        else
            boost::program_options::store(boost::program_options::parse_config_file(config_file, configuration_options_description_, true), configuration_variables_map_);
    });

    boost::program_options::notify(configuration_variables_map_);

    // The help argument has been flagged, display the
    // server options and throw a runtime_error exception
    // to stop server startup.
    if(configuration_variables_map_.count("help"))
    {
        std::cout << configuration_options_description_ << std::endl;
        throw std::runtime_error("Help option flagged.");
    }
}

void BaseServer::LoadOptions_(uint32_t argc, char* argv[], std::list<std::string> config_files)
{
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, configuration_options_description_), configuration_variables_map_);

    // Iterate through the configuration files
    // that are to be loaded. If a configuration file
    // is missing, throw a runtime_error.
    std::for_each(config_files.begin(), config_files.end(), [=] (const std::string& filename) {
        std::ifstream config_file(filename);
        if(!config_file)
            throw std::runtime_error("Could not open configuration file.");
        else
            boost::program_options::store(boost::program_options::parse_config_file(config_file, configuration_options_description_, true), configuration_variables_map_);
    });

    boost::program_options::notify(configuration_variables_map_);

    // The help argument has been flagged, display the
    // server options and throw a runtime_error exception
    // to stop server startup.
    if(configuration_variables_map_.count("help"))
    {
        std::cout << configuration_options_description_ << std::endl;
        throw std::runtime_error("Help option flagged.");
    }
}

}