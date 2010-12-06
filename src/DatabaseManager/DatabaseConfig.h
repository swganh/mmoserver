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

#ifndef ANH_DATABASEMANAGER_DATABASECONFIG_H
#define ANH_DATABASEMANAGER_DATABASECONFIG_H

#include <stdint.h>

/**
 * \brief A collection of Database Configuration options.
 */
class DatabaseConfig
{
public:
	
	/**
	 * \brief Initalizes the configuration options.
	 *
	 * \param db_min_threads The minium number of threads used for database work.
	 * \param db_max_threads The maximum number of threads used for database work.
	 */
	DatabaseConfig(uint32_t db_min_threads, uint32_t db_max_threads)
		: db_min_threads_(db_min_threads)
		, db_max_threads_(db_max_threads) { }

	/**
	 * \brief Default destructor.
	 */
	~DatabaseConfig() { }

	const uint32_t getDbMinThreads() const {
		return db_min_threads_;
	}

	const uint32_t getDbMaxThreads() const {
		return db_max_threads_;
	}

private:
	uint32_t db_min_threads_;
	uint32_t db_max_threads_;
};

#endif // ANH_DATABASEMANAGER_DATABASECONFIG_H