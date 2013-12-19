
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
#include "ObjectFactory.h"
#include "HarvesterObject.h"
#include "PlayerObject.h"
#include "Inventory.h"

#include "ResourceContainer.h"
#include "StructureManager.h"
#include "UIManager.h"

#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include <cassert>

//=============================================================================

HarvesterObject::HarvesterObject() : PlayerStructure()
{
    mType = ObjType_Structure;
    mCurrentExtractionRate = 0.0;

}

//=============================================================================

HarvesterObject::~HarvesterObject()
{

}



//=============================================================================
// gets the spec extractionrate
bool HarvesterObject::checkResourceList(uint64 id)
{

    HResourceList::iterator it = mResourceList.begin();
    while (it != mResourceList.end())
    {
        if((*it).first == id)
        {
            return true;
        }
        it++;
    }
    return false;

}


//=============================================================================
// gets the spec extractionrate

float HarvesterObject::getSpecExtraction()
{

    if(!this->hasAttribute("examine_extractionrate"))
    {
        this->addAttribute("examine_extractionrate","3.0");
    }

    return this->getAttribute<float>("examine_extractionrate");

}

//=============================================================================
// gets the spec hoppersize

float HarvesterObject::getHopperSize()
{

    if(!this->hasAttribute("examine_hoppersize"))
    {
        this->addAttribute("examine_hoppersize","3000.0");
    }

    return this->getAttribute<float>("examine_hoppersize");

}

//=============================================================================
// gets the current hoppersize

float HarvesterObject::getCurrentHopperSize()
{
    float resourceAmount = 0.0;
    //count all the resource data
    HResourceList::iterator it = mResourceList.begin();
    while (it != mResourceList.end())
    {
        resourceAmount += (*it).second;
        it++;

    }
    return   (resourceAmount);

}

//=============================================================================
//handles the radial selection

void HarvesterObject::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject);
    if(!player)
    {
        return;
    }

    switch(messageType)
    {
    case radId_StructureStatus:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_ViewStatus;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

    }
    break;

    case radId_depositPower:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_DepositPower;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

    }
    break;

    case radId_payMaintenance:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_PayMaintenance;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

    }
    break;

    case radId_serverTerminalManagementDestroy:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_Destroy;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

    }
    break;
    case radId_serverTerminalPermissionsAdmin:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_PermissionAdmin;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

    }
    break;

    case radId_serverTerminalPermissionsHopper:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_PermissionHopper;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

    }
    break;

    case radId_setName:
    {

        StructureAsyncCommand command;
        command.Command = Structure_Command_RenameStructure;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);


    }
    break;

    case radId_operateHarvester:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_OperateHarvester;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"HOPPER",command);

    }
    break;

    }
}

//=============================================================================
// prepares the custom radial for our harvester
void HarvesterObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(creatureObject);
    if(!player)
    {
        return;
    }

    RadialMenu* radial	= new RadialMenu();

    //radId_serverHouseManage
    radial->addItem(1,0,radId_examine,radAction_Default,"");
    radial->addItem(2,0,radId_serverHarvesterManage,radAction_ObjCallback,"Structure Management");
    radial->addItem(3,0,radId_serverTerminalManagement,radAction_ObjCallback,"Structure Permissions");

    radial->addItem(4,2,radId_serverTerminalManagementDestroy,radAction_ObjCallback,"Destroy Structure");//destroy
    radial->addItem(5,2,radId_StructureStatus,radAction_ObjCallback,"Status");//destroy
    radial->addItem(6,2,radId_payMaintenance,radAction_ObjCallback,"Pay Maintenance");//destroy
    radial->addItem(7,2,radId_setName,radAction_ObjCallback,"Set Name");//destroy
    radial->addItem(8,2,radId_operateHarvester,radAction_ObjCallback,"Operate Harvester");//destroy


    //generators dont need power
    uint32 type = this->getHarvesterFamily();
    if((type != 41)&&(type != 42)&&(type != 43))
        radial->addItem(9,2,radId_depositPower,radAction_ObjCallback,"Deposit Power");//destroy


    radial->addItem(10,3,radId_serverTerminalPermissionsAdmin,radAction_ObjCallback,"Admin List");//destroy
    radial->addItem(11,3,radId_serverTerminalPermissionsHopper,radAction_ObjCallback,"Hopper List");//destroy



    RadialMenuPtr radialPtr(radial);
    mRadialMenu = radialPtr;
}


//=============================================================================
// creates resource containers ín our inventory
void HarvesterObject::createResourceContainer(uint64 resID, PlayerObject* player, uint32 amount)
{
    //now create the resource container

    ObjectIDList*			invObjects	= dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getObjects();
    ObjectIDList::iterator	listIt		= invObjects->begin();

    uint32 rAmount = amount;

    while(listIt != invObjects->end())
    {
        // we are looking for resource containers
        ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById((*listIt)));
        if(resCont)
        {
            uint32 targetAmount	= resCont->getAmount();
            uint32 maxAmount	= resCont->getMaxAmount();
            uint32 newAmount;

            if((resCont->getResourceId() == resID))
            {
                //find out how much we can add to the container
                uint32 addAmount = maxAmount - targetAmount;

                if(addAmount >rAmount)
                {
                    addAmount = rAmount;
                    rAmount = 0;
                }
                else
                    rAmount -= addAmount;

                if(addAmount)
                {
                    // find out how big our container is now
                    newAmount = targetAmount + addAmount;

                    // update target container
                    resCont->setAmount(newAmount);

                    gMessageLib->sendResourceContainerUpdateAmount(resCont,player);

                    gWorldManager->getDatabase()->executeSqlAsync(NULL,NULL,"UPDATE %s.resource_containers SET amount=%u WHERE id=%" PRIu64 "",gWorldManager->getDatabase()->galaxy(),newAmount,resCont->getId());
                    
                }
            }
        }

        ++listIt;
    }

    // or need to create a new one

    while(rAmount)
    {
        uint32 a = 100000;

        if( a > rAmount)
            a = rAmount;

        gObjectFactory->requestNewResourceContainer(dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)),resID,player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId(),99,a);
        rAmount -= a;

    }


}

void HarvesterObject::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;

    switch(asynContainer->mQueryType)
    {

    case Structure_ResourceRetrieve:
    {
        PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));
        HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));

        DataBinding* binding = gWorldManager->getDatabase()->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);

        uint64 count;
        uint32 error;
        count = result->getRowCount();

        if(!count)
        {
            assert(false && "HarvesterObject::handleDatabaseJobComplete Structure_ResourceRetrieve did not find any resources");
            return;
        }

        result->getNextRow(binding,&error);
        if(error > 0)
        {
            //mmh there was something fishy ... no changes db side
            gMessageLib->sendResourceEmptyHopperResponse(harvester,player,error, asynContainer->command.b1, asynContainer->command.b2);
            return;
        }

        createResourceContainer(asynContainer->command.ResourceId, player, asynContainer->command.Amount);


        StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_ResourceDiscardUpdateHopper,player->getClient());
        asyncContainer->mStructureId	= asynContainer->mStructureId;
        asyncContainer->mPlayerId		= asynContainer->mPlayerId;
        asyncContainer->command			= asynContainer->command;

        gWorldManager->getDatabase()->executeSqlAsync(harvester,asyncContainer,"SELECT hr.resourceID, hr.quantity FROM %s.harvester_resources hr WHERE hr.ID = '%" PRIu64 "' ",gWorldManager->getDatabase()->galaxy(), harvester->getId());
        

    }
    break;

    case Structure_ResourceDiscard:
    {
        PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));
        HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));

        DataBinding* binding = gWorldManager->getDatabase()->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);

        uint64 count;
        uint32 error;
        count = result->getRowCount();

        if(!count)
        {
            assert(false && "HarvesterObject::handleDatabaseJobComplete Structure_ResourceDiscard did not find resource");
            return;
        }

        result->getNextRow(binding,&error);
        if(result > 0)
        {
            //mmh there was something fishy ... no changes db side
            gMessageLib->sendResourceEmptyHopperResponse(harvester,player,error, asynContainer->command.b1, asynContainer->command.b2);
            return;
        }

        StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_ResourceDiscardUpdateHopper,player->getClient());
        asyncContainer->mStructureId	= asynContainer->mStructureId;
        asyncContainer->mPlayerId		= asynContainer->mPlayerId;
        asyncContainer->command			= asyncContainer->command;

        gWorldManager->getDatabase()->executeSqlAsync(harvester,asyncContainer,"SELECT hr.resourceID, hr.quantity FROM %s.harvester_resources hr WHERE hr.ID = '%" PRIu64 "' ",gWorldManager->getDatabase()->galaxy(),harvester->getId());
        
    }
    break;

    case Structure_ResourceDiscardUpdateHopper:
    {
        uint64 count = result->getRowCount();

        HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));
        PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

        DataBinding* binding = gWorldManager->getDatabase()->createDataBinding(2);
        binding->addField(DFT_uint64,offsetof(HarvesterHopperItem,ResourceID),8,0);
        binding->addField(DFT_float,offsetof(HarvesterHopperItem,Quantity),4,1);

        HResourceList*	hRList = harvester->getResourceList();

        hRList->clear();

        HarvesterHopperItem hopperTemp;
        for(uint64 i=0; i <count; i++)
        {
            result->getNextRow(binding,&hopperTemp);
            hRList->push_back(std::make_pair(hopperTemp.ResourceID,hopperTemp.Quantity));
        }

        //now send the update to the client
        gMessageLib->SendHarvesterHopperUpdate(harvester,player);

        gMessageLib->sendResourceEmptyHopperResponse(harvester,player,0, asynContainer->command.b2, asynContainer->command.b2);

        gWorldManager->getDatabase()->destroyDataBinding(binding);

    }
    break;

    case Structure_GetResourceData:
    {
        uint64 count = result->getRowCount();

        HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));
        PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

        DataBinding* binding = gWorldManager->getDatabase()->createDataBinding(2);
        binding->addField(DFT_uint64,offsetof(HarvesterHopperItem,ResourceID),8,0);
        binding->addField(DFT_float,offsetof(HarvesterHopperItem,Quantity),4,1);

        HResourceList*	hRList = harvester->getResourceList();

        hRList->clear();

        HarvesterHopperItem hopperTemp;
        for(uint64 i=0; i <count; i++)
        {
            result->getNextRow(binding,&hopperTemp);
            hRList->push_back(std::make_pair(hopperTemp.ResourceID,hopperTemp.Quantity));
        }

        //now send the update to the client
        gMessageLib->sendHarvesterResourceData(harvester,player);
        gMessageLib->sendBaselinesHINO_7(harvester,player);

        gWorldManager->getDatabase()->destroyDataBinding(binding);

    }
    break;

    //read in the current resource hoppers contents
    case Structure_HopperUpdate:
    {
        uint64 count = result->getRowCount();

        HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));
        PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

        DataBinding* binding = gWorldManager->getDatabase()->createDataBinding(2);
        binding->addField(DFT_uint64,offsetof(HarvesterHopperItem,ResourceID),8,0);
        binding->addField(DFT_float,offsetof(HarvesterHopperItem,Quantity),4,1);

        HResourceList*	hRList = harvester->getResourceList();

        hRList->clear();

        HarvesterHopperItem hopperTemp;
        for(uint64 i=0; i <count; i++)
        {
            result->getNextRow(binding,&hopperTemp);
            hRList->push_back(std::make_pair(hopperTemp.ResourceID,hopperTemp.Quantity));
        }

        //now send the update to the client
        gMessageLib->SendHarvesterHopperUpdate(harvester,player);

        gWorldManager->getDatabase()->destroyDataBinding(binding);
    }
    break;

    case Structure_HopperDiscard:
    {
        //PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

        PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));
        if(!player)
        {
            return;
        }

        StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_HopperUpdate,player->getClient());

        asyncContainer->mStructureId	= this->getId();
        asyncContainer->mPlayerId		= player->getId();

        int8 sql[250];
        sprintf(sql,"SELECT hr.resourceID, hr.quantity FROM %s.harvester_resources hr WHERE hr.ID = '%" PRIu64 "' ",gWorldManager->getDatabase()->galaxy(),this->getId());

        gWorldManager->getDatabase()->executeSqlAsync(this,asyncContainer,sql);
        

    }
    break;


    default:
        break;

    }

    SAFE_DELETE(asynContainer);
}




