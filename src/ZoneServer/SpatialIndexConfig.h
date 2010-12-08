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

#ifndef ANH_ZONESERVER_SPATIALINDEXCONFIG_H
#define ANH_ZONESERVER_SPATIALINDEXCONFIG_H

#include <stdint.h>

/**
 * \brief
 */
class SpatialIndexConfig
{
public:
	/**
	 * \breif
	 */
	SpatialIndexConfig(float fill_factor, uint32_t index_cap, uint32_t leaf_cap, float horizon)
		: fill_factor_(fill_factor)
		, index_cap_(index_cap)
		, leaf_cap_(leaf_cap)
		, horizon_(horizon) { }

	/**
	 * \breif Default destructor.
	 */
	~SpatialIndexConfig() { }

	const float getFillFactor() const {
		return fill_factor_;
	}

	const uint32_t getIndexCap() const {
		return index_cap_;
	}

	const uint32_t getLeafCap() const {
		return leaf_cap_;
	}

	const float getHorizon() const {
		return horizon_;
	}

private:
	float fill_factor_;
	uint32_t index_cap_;
	uint32_t leaf_cap_;
	float horizon_;
};

#endif // ANH_ZONESERVER_SPATIALINDEXCONFIG_H