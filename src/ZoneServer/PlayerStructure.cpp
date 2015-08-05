
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

#include "PlayerStructure.h"
#include "PlayerObject.h"
#include "Inventory.h"
#include "CellObject.h"
#include "Bank.h"
#include "UICallback.h"
#include "UIManager.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
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

void PlayerStructure::handleUIEvent(BString strCharacterCash, BString strHarvesterCash, UIWindow* window)
{

    PlayerObject* player = window->getOwner();

    if(!player)
    {
        return;
    }

    switch(window->getWindowType())
    {
    case SUI_Window_Deposit_Power:
    {
        strCharacterCash.convert(BSTRType_ANSI);
        BString characterPower = strCharacterCash;

        strHarvesterCash.convert(BSTRType_ANSI);
        BString harvesterPower = strHarvesterCash;

        int32 harvesterPowerDelta = atoi(harvesterPower.getAnsi());

        gStructureManager->deductPower(player,harvesterPowerDelta);
        this->setCurrentPower(getCurrentPower()+harvesterPowerDelta);

        gWorldManager->getDatabase()->executeSqlAsync(0,0,"UPDATE %s.structure_attributes SET value='%u' WHERE structure_id=%" PRIu64 " AND attribute_id=384",gWorldManager->getDatabase()->galaxy(),getCurrentPower(),this->getId());
        
    }
    break;

    case SUI_Window_Pay_Maintenance:
    {
        strCharacterCash.convert(BSTRType_ANSI);
        strHarvesterCash.convert(BSTRType_ANSI);

        Bank* bank = dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
        Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
        int32 bankFunds = bank->credits();
        int32 inventoryFunds = inventory->getCredits();

        int32 funds = inventoryFunds + bankFunds;

        int32 characterMoneyDelta = atoi(strCharacterCash.getAnsi()) - funds;
        int32 harvesterMoneyDelta = atoi(strHarvesterCash.getAnsi()) - this->getCurrentMaintenance();

        // the amount transfered must be greater than zero
        if(harvesterMoneyDelta == 0 || characterMoneyDelta == 0)
        {
            return;
        }

        //lets get the money from the bank first
        if((bankFunds +characterMoneyDelta)< 0)
        {
            characterMoneyDelta += bankFunds;
            bankFunds = 0;

            inventoryFunds += characterMoneyDelta;

        }
        else
        {
            bankFunds += characterMoneyDelta;
        }

        if(inventoryFunds < 0)
        {
            return;
        }

        int32 maintenance = this->getCurrentMaintenance() + harvesterMoneyDelta;

        if(maintenance < 0)
        {
            return;
        }

        bank->credits(bankFunds);
        inventory->setCredits(inventoryFunds);

        gWorldManager->getDatabase()->destroyResult(gWorldManager->getDatabase()->executeSynchSql("UPDATE %s.banks SET credits=%u WHERE id=%" PRIu64 "",gWorldManager->getDatabase()->galaxy(),bank->credits(),bank->getId()));
        gWorldManager->getDatabase()->destroyResult(gWorldManager->getDatabase()->executeSynchSql("UPDATE %s.inventories SET credits=%u WHERE id=%" PRIu64 "",gWorldManager->getDatabase()->galaxy(),inventory->getCredits(),inventory->getId()));

        //send the appropriate deltas.
        gMessageLib->sendInventoryCreditsUpdate(player);
        gMessageLib->sendBankCreditsUpdate(player);

        //get the structures conditiondamage and see whether it needs repair
        uint32 damage = this->getDamage();

        if(damage)
        {
            uint32 cost = this->getRepairCost();
            uint32 all = cost*damage;
            if(maintenance <= (int32)all)
            {
                all -= (uint32)maintenance;
                damage = (uint32)(all/cost);
                maintenance = 0;
            }

            if(maintenance > (int32)all)
            {
                maintenance -= (int32)all;
                damage = 0;
            }

            //update the remaining damage in the db
            gWorldManager->getDatabase()->executeSqlAsync(0,0,"UPDATE %s.structures s SET s.condition= %u WHERE s.ID=%" PRIu64 "",gWorldManager->getDatabase()->galaxy(),damage,this->getId());
            
            this->setDamage(damage);

            //Update the structures Condition
            gMessageLib->sendHarvesterCurrentConditionUpdate(this);

        }

        gWorldManager->getDatabase()->executeSqlAsync(0,0,"UPDATE %s.structure_attributes SET value='%u' WHERE structure_id=%" PRIu64 " AND attribute_id=382",gWorldManager->getDatabase()->galaxy(),maintenance,this->getId());
        

        this->setCurrentMaintenance(maintenance);


    }
    break;

    }
}


//=============================================================================
//

void PlayerStructure::handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window)
{

    PlayerObject* player = window->getOwner();

    // action is zero for ok !!!

    if(!player || (action) || player->isIncapacitated() || player->isDead())
    {
        return;
    }

    switch(window->getWindowType())
    {
    case SUI_Window_Factory_Schematics:
    {
        uint64 ManSchemId = 0;
        //check for use schematic
        BString b = window->getOption3();
        b.convert(BSTRType_ANSI);
        if(strcmp(b.getAnsi(),"false") == 0)
        {
            WindowAsyncContainerCommand* asyncContainer = (WindowAsyncContainerCommand*)window->getAsyncContainer();
            if(!asyncContainer)
            {
                return;
            }
            if(asyncContainer->SortedList.size())
                ManSchemId = asyncContainer->SortedList.at(element);
            else
            {
                SAFE_DELETE(asyncContainer);
                gMessageLib->SendSystemMessage(::common::OutOfBand("manf_station", "schematic_not_added"), player);
                return;
            }

            SAFE_DELETE(asyncContainer);

            StructureAsyncCommand command;
            command.Command = Structure_Command_AddSchem;
            command.PlayerId = player->getId();
            command.StructureId = this->getId();
            command.SchematicId = ManSchemId;

            gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"HOPPER",command);
        }
        else if(strcmp(b.getAnsi(),"true") == 0) //remove schematic pressed
        {

            WindowAsyncContainerCommand* asyncContainer = (WindowAsyncContainerCommand*)window->getAsyncContainer();
            SAFE_DELETE(asyncContainer);

            StructureAsyncCommand command;
            command.Command = Structure_Command_RemoveSchem;
            command.PlayerId = player->getId();
            command.StructureId = this->getId();
            command.SchematicId = ManSchemId;

            gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"HOPPER",command);
        }



    }
    break;

    case SUI_Window_Structure_Status:
    {
        //we want to refresh
        StructureAsyncCommand command;
        command.Command = Structure_Command_ViewStatus;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

    }
    break;

    case SUI_Window_Structure_Delete:
    {
        gStructureManager->createNewStructureDeleteConfirmBox(player,this );

    }
    break;

    case SUI_Window_Structure_Rename:
    {

        inputStr.convert(BSTRType_ANSI);

        if(!inputStr.getLength())
        {
            //hmmm no answer - remain as it is?
            return;
        }

        if(inputStr.getLength() > 68)
        {
            //hmmm no answer - remain as it is?
            gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "not_valid_name"), player);
            return;

        }

        //inputStr.convert(BSTRType_Unicode16);
        this->setCustomName(inputStr.getAnsi());

        gMessageLib->sendNewHarvesterName(this);

        //update db!!!
        // pull the db query


        int8	sql[255],end[128],*sqlPointer;

        sprintf(sql,"UPDATE %s.structures SET structures.name = '",gWorldManager->getDatabase()->galaxy());
        sprintf(end,"' WHERE structures.ID = %" PRIu64 "",this->getId());
        sqlPointer = sql + strlen(sql);
        sqlPointer += gWorldManager->getDatabase()->escapeString(sqlPointer,inputStr.getAnsi(),inputStr.getLength());
        strcat(sql,end);

        gWorldManager->getDatabase()->executeSqlAsync(0,0,sql);
        

    }
    break;

    case SUI_Window_Structure_Delete_Confirm:
    {
        inputStr.convert(BSTRType_ANSI);
        if(inputStr.getCrc() == this->getCode().getCrc())
        {
            if((this->checkStatesEither(PlayerStructureState_Destroy)))
            {
                //dont start structure destruction more than once
                return;
            }
            this->toggleStateOn(PlayerStructureState_Destroy);

            //delete it
            mTTS.todo		= ttE_Delete;
            mTTS.playerId	= player->getId();
            gStructureManager->addStructureforDestruction(this->getId());
        }
        else
        {
            int8 text[255];
            sprintf(text,"@player_structure:incorrect_destroy_code");
            gUIManager->createNewMessageBox(NULL,"","SWG::ANH",text,player);
        }
        //we need to get the input
    }
    }
}

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
