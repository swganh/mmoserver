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

#include "BadgeRegionFactory.h"
#include "BadgeRegion.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include <cppconn/resultset.h>

#include "Utils/utils.h"

BadgeRegionFactory::BadgeRegionFactory(Database* database) : FactoryBase(database)
{}

//=============================================================================

BadgeRegionFactory::~BadgeRegionFactory()
{}

void BadgeRegionFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
}
//=============================================================================

void BadgeRegionFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id, uint16 subGroup, uint16 subType, DispatchClient* client)
{
    // Setup our statement
    int8 sql[128];
    sprintf(sql, "CALL %s.sp_BadgeGetByRegion(%" PRIu64 ");", mDatabase->galaxy(),  id);

    mDatabase->executeAsyncProcedure(sql, [=](DatabaseResult* result) {
        if(!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Unable to load badges with region id: " << id;
            return;
        }
        std::shared_ptr<BadgeRegion> badge_region = std::make_shared<BadgeRegion>(result_set->getUInt(2));

        badge_region->setId(result_set->getUInt64(1));
        badge_region->setRegionName(result_set->getString(3));
        badge_region->setNameFile(result_set->getString(4));
        badge_region->mPosition.x = result_set->getDouble(5);
        badge_region->mPosition.z = result_set->getDouble(6);
        badge_region->setWidth(result_set->getDouble(7));
        badge_region->setHeight(result_set->getDouble(8));
        badge_region->setParentId(result_set->getUInt64(9));
        badge_region->setLoadState(LoadState_Loaded);

        ofCallback->handleObjectReady(badge_region);
    });

}


