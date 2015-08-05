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

#include "SpawnRegionFactory.h"
#include "SpawnRegion.h"
#include "ObjectFactoryCallback.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include <cppconn/resultset.h>

#include "Utils/utils.h"

//=============================================================================

SpawnRegionFactory::SpawnRegionFactory(Database* database) : FactoryBase(database) {}

//=============================================================================

SpawnRegionFactory::~SpawnRegionFactory() {}

//=============================================================================

void SpawnRegionFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result) {}

//=============================================================================

void SpawnRegionFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    // setup our statement
    int8 sql[128];
    sprintf(sql,"SELECT spawn_regions.id,spawn_regions.spawn_type,planet_regions.region_name,planet_regions.region_file,planet_regions.x,planet_regions.z,"
            "planet_regions.width,planet_regions.height,spawn_regions.parent_id,spawn_regions.mission"
            " FROM %s.spawn_regions"
            " INNER JOIN %s.planet_regions ON (spawn_regions.region_id = planet_regions.region_id)"
            " WHERE (spawn_regions.id = %" PRIu64 ")",mDatabase->galaxy(),mDatabase->galaxy(),id);

    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Unable to load SpawnRegion with region id: " << id;
            return;
        }
        std::shared_ptr<SpawnRegion> spawn = std::make_shared<SpawnRegion>();
        spawn->setId(result_set->getUInt64(1));
        spawn->setSpawnType(result_set->getUInt(2));
        spawn->setRegionName(result_set->getString(3));
        spawn->setNameFile(result_set->getString(4));
        spawn->mPosition.x = result_set->getDouble(5);
        spawn->mPosition.z = result_set->getDouble(6);
        spawn->setWidth(result_set->getDouble(7));
        spawn->setHeight(result_set->getDouble(8));
        spawn->setMission(result_set->getUInt(9));
        spawn->setLoadState(LoadState_Loaded);

        ofCallback->handleObjectReady(spawn);

    });
}

