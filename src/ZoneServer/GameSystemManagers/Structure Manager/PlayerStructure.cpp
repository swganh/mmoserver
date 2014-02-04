
/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "PlayerStructure.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "Zoneserver/Objects/Inventory.h"
#include "CellObject.h"
//#include "ZoneServer/Objects/Bank.h"
//#include "ZoneServer/GameSystemManagers/UI Manager/UICallback.h"
//#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
#include "ZoneServer/WorldManager.h"
#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include <anh\app\swganh_kernel.h>
//=============================================================================

PlayerStructure::PlayerStructure() : TangibleObject()
{
    mMaxCondition	= 1000;
    mCondition		= 1000;
    mWillRedeed		= false;

    mState			= 0;

    mHousingAdminList.clear();
}

//=============================================================================
//
//
PlayerStructure::~PlayerStructure()
{

}

//=============================================================================
// gets the maintenance paid into the harvester - CAVE we need to query this asynch!
// as we modify the maintenance pool minutely  through the chatserver
uint32 PlayerStructure::getCurrentMaintenance()
{
    if (this->hasAttribute("examine_maintenance"))
    {
        uint32 maintenance = this->getAttribute<uint32>("examine_maintenance");
        return maintenance;
    }

    setCurrentMaintenance(0);
    return 0;
}


//=============================================================================
//
//
void PlayerStructure::setCurrentMaintenance(uint32 maintenance)
{
    if (this->hasAttribute("examine_maintenance"))
    {
        this->setAttribute("examine_maintenance",boost::lexical_cast<std::string>(maintenance));
        return;

    }


    this->addAttribute("examine_maintenance",boost::lexical_cast<std::string>(maintenance));
}

//=============================================================================
// gets the maintenance paid into the harvester - CAVE we need to query this asynch!
// as we modify the maintenance pool minutely  through the chatserver
uint32 PlayerStructure::getCurrentPower()
{
    if (this->hasAttribute("examine_power"))
    {
        uint32 power = this->getAttribute<uint32>("examine_power");
        return power;
    }

    setCurrentPower(0);
    return 0;
}



//=============================================================================
//
//
void PlayerStructure::setCurrentPower(uint32 power)
{
    if (this->hasAttribute("examine_power"))
    {
        this->setAttribute("examine_power",boost::lexical_cast<std::string>(power));
        return;

    }


    this->addAttribute("examine_power",boost::lexical_cast<std::string>(power));

}

//=============================================================================
//
//
bool PlayerStructure::canRedeed()
{
    if(!(this->getCondition() == this->getMaxCondition()))
    {
        setRedeed(false);
        return(false);
    }

    if(!(this->getCurrentMaintenance() >= (this->getMaintenanceRate()*45)))
    {
        setRedeed(false);
        return(false);
    }
    setRedeed(true);
    return true;
}



//=============================================================================
// now we have the maintenance data we can proceed with the delete UI
//
void PlayerStructure::deleteStructureDBDataRead(uint64 playerId)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

    gStructureManager->createNewStructureDestroyBox(player, this, canRedeed());

}


//=============================================================================
// now we have the adminlist we can proceed to display it
//
void PlayerStructure::sendStructureAdminList(uint64 playerId)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

    gMessageLib->sendAdminList(this,player);


}

//=============================================================================
// now we have the adminlist we can proceed to display it
//
void PlayerStructure::sendStructureBanList(uint64 playerId)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

    gMessageLib->sendBanList(this,player);


}

//=============================================================================
// now we have the adminlist we can proceed to display it
//
void PlayerStructure::sendStructureEntryList(uint64 playerId)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

    gMessageLib->sendEntryList(this,player);


}



//=============================================================================
// now we have the hopperlist we can proceed to display it
//
void PlayerStructure::sendStructureHopperList(uint64 playerId)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

    gMessageLib->sendHopperList(this,player);


}

//=============================================================================
// thats for the transferbox
//


//=============================================================================
//


//=============================================================================
//
//

bool PlayerStructure::hasAdminRights(uint64 id)
{
    ObjectIDList::iterator it = mHousingAdminList.begin();

    while(it != mHousingAdminList.end())
    {
        if((*it) == id)
            return(true);

        ++it;
    }
    return false;
}
