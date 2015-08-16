
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
#include "FactoryObject.h"
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

//============================================================================
FactoryObject::FactoryObject() : PlayerStructure()
{
    mType = ObjType_Structure;

}

//=============================================================================

FactoryObject::~FactoryObject()
{
    //hoppers are strored in the factories objectcontainer and get automatically destroyed on shutdown
}


//========================================================================0
// we only handle the hoppers here
// we get here on runtime when the last content has been loaded from db
// respectively if the content update is done
// when it is processed it sends the open container - by this we ensure, that the
// hopper is up to date when the player sees it

void FactoryObject::handleObjectReady(Object* object,DispatchClient* client, uint64 hopper)
{
    Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(hopper));
    if(!item)
    {
        assert(false && "FactoryObject::handleObjectReady WorldManager could not find hopper");
    }

    if((item->getId() == this->getIngredientHopper())||(item->getId() == this->getOutputHopper()))
    {
        PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getPlayerByAccId(client->getAccountId()));
        if(player)
        {
            gMessageLib->sendOpenedContainer(item->getId(),player);
        }
        else
        {
        }


    }
    else
    {
    }


}



//=============================================================================
//handles the radial selection

void FactoryObject::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject);
    if(!player)
    {
        return;
    }

    switch(messageType)
    {
    case radId_StopManufacture:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_StopFactory;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
    }
    break;

    case radId_StartManufacture:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_StartFactory;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
    }
    break;

    case radId_serverManfHopperInput:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_AccessInHopper;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"HOPPER",command);
    }
    break;

    case radId_serverManfHopperOutput:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_AccessOutHopper;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"HOPPER",command);
    }
    break;

    case radId_serverManfStationSchematic:
    {
        StructureAsyncCommand command;
        command.Command = Structure_Command_AccessSchem;
        command.PlayerId = player->getId();
        command.StructureId = this->getId();

        gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
    }
    break;

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
        //is there a manufacturing schematic inside ???

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

    }
}

//=============================================================================
// prepares the custom radial for our harvester
void FactoryObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(creatureObject);
    if(!player)
    {
        return;
    }

    RadialMenu* radial	= new RadialMenu();


    //radId_serverHouseManage
    uint8 i = 0;
    radial->addItem(++i,0,radId_examine,radAction_Default,"");
    radial->addItem(++i,0,radId_serverHarvesterManage,radAction_ObjCallback,"Structure Management");
    radial->addItem(++i,0,radId_serverTerminalManagement,radAction_ObjCallback,"Structure Permissions");
    radial->addItem(++i,0,radId_StructureOptions,radAction_ObjCallback,"Options");

    radial->addItem(++i,2,radId_serverTerminalManagementDestroy,radAction_ObjCallback,"Destroy Structure");
    radial->addItem(++i,2,radId_StructureStatus,radAction_ObjCallback,"Status");
    radial->addItem(++i,2,radId_payMaintenance,radAction_ObjCallback,"Pay Maintenance");
    radial->addItem(++i,2,radId_setName,radAction_ObjCallback,"Set Name");
    radial->addItem(++i,2,radId_depositPower,radAction_ObjCallback,"Deposit Power");


    radial->addItem(++i,3,radId_serverTerminalPermissionsAdmin,radAction_ObjCallback,"Admin List");
    radial->addItem(++i,3,radId_serverTerminalPermissionsHopper,radAction_ObjCallback,"Hopper List");

    if(mManSchematicID)
    {
        if(!mActive)
            radial->addItem(++i,4,radId_StartManufacture,radAction_ObjCallback,"Start manufacturing objects");
        else
            radial->addItem(++i,4,radId_StopManufacture,radAction_ObjCallback,"Stop manufacturing objects");

        radial->addItem(++i,4,radId_ListIngredients,radAction_ObjCallback,"List ingredients needed for station");

    }
    radial->addItem(++i,4,radId_serverManfStationSchematic,radAction_ObjCallback,"Access schematic slot");
    radial->addItem(++i,4,radId_serverManfHopperInput,radAction_ObjCallback,"Access station ingredient hopper");
    radial->addItem(++i,4,radId_serverManfHopperOutput,radAction_ObjCallback,"Access station output hopper");




    RadialMenuPtr radialPtr(radial);
    mRadialMenu = radialPtr;
}



void FactoryObject::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;

//	switch(asynContainer->mQueryType)
//	{


//		default:break;

//	}

    SAFE_DELETE(asynContainer);
}



