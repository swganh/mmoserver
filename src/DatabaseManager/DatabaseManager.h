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

#ifndef DATABASE_MANAGER_DATABASE_MANAGER_H_
#define DATABASE_MANAGER_DATABASE_MANAGER_H_

#include <cstdint>
#include <list>
#include <memory>

#include <boost/noncopyable.hpp>

#include "DatabaseManager/DatabaseConfig.h"
#include "DatabaseManager/DatabaseType.h"

class Database;

/*! Manages multiple database connections.
*/
class DatabaseManager : private boost::noncopyable {
public:
	/**
	 * \brief Default constructor.
	 *
	 * \param db_config Database configuration options.
	 * \see DatabaseConfig
	 */
	explicit DatabaseManager(const DatabaseConfig& database_configuration)
		: database_configuration_(database_configuration) { }

    /*! Processes all current database connections.
    */
    void process();

    /*! Connects to a specified database.
    *
    * \param host The database host to connect to.
    * \param port The port of the database host to connect to.
    * \param user The username for accessing the requested schema.
    * \param pass The password for accessing the requested schema.
    * \param schema The database to connect to.
    *
    * \return The instance of the database created after successful connection.
    */
    Database* connect(DBType type, 
        const std::string& host, 
        uint16_t port, 
        const std::string& user, 
        const std::string& pass, 
        const std::string& dbname);

private:
    typedef std::list<std::shared_ptr<Database>> DatabaseList;
    DatabaseList database_list_;
	DatabaseConfig database_configuration_;
};

#endif  // DATABASE_MANAGER_DATABASE_MANAGER_H_
