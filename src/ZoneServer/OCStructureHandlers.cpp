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

//common includes
#include "ObjectController.h"
#include "OCStructureHandlers.h"

#include <cstdint>

#ifdef _MSC_VER
#include <regex>  // NOLINT
#else
#include <boost/regex.hpp>  // NOLINT
#endif

#include "Deed.h"
#include "HarvesterFactory.h"
#include "Heightmap.h"
#include "HarvesterObject.h"
#include "HouseObject.h"
#include "CellObject.h"
#include "PlayerStructure.h"
#include "PlayerObject.h"
#include "Inventory.h"
#include "Item.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectFactory.h"

#include "UIManager.h"
#include "StructureManager.h"
#include "ResourceManager.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include "StructureHeightmapAsyncContainer.h"

// GCC doesn't fully support regex yet (some methods are unimplemented at this
// time), so fall back to the boost regex (which is essentially the same
// interface, different namespace). Once GCC finishes their implementation all
// boost/regex references can be removed.
#ifdef WIN32
using ::std::regex;
using ::std::smatch;
using ::std::regex_search;
using ::std::sregex_token_iterator;
#else
using ::boost::regex;
using ::boost::smatch;
using ::boost::regex_search;
using ::boost::sregex_token_iterator;
#endif

const float MOVE_INCREMENT   = 0.01f;
const float ROTATE_INCREMENT = 90.0f;

//======================================================================================================================
//
// Modifies the Admin List
//
void	ObjectController::_handleModifyPermissionList(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)

{

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if(!player)
    {
        DLOG(INFO) << " ObjectController::_handleModifyPermissionList Player not found";
        return;
    }

    //find out where our structure is
    BString dataStr;
    message->getStringUnicode16(dataStr);

    BString playerStr,list,action;

    dataStr.convert(BSTRType_ANSI);

    int8 s1[64],s2[32],s3[32];
    sscanf(dataStr.getAnsi(),"%32s %16s %16s",s1, s2, s3);
    playerStr = s1;
    list = s2;
    action = s3;

    if(playerStr.getLength() > 40)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "permission_40_char"), player);
        return;
    }

    //TODO is target a structure?? used when using the commandline option
    uint64 id = player->getTargetId();
    Object* object = gWorldManager->getObjectById(id);
    PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

    //if we have no structure that way, see whether we have a structure were we just used the adminlist
    if(!structure)
    {
        id = player->getStructurePermissionId();
        Object* object = gWorldManager->getObjectById(id);
        structure = dynamic_cast<PlayerStructure*>(object);
    }

    if(!structure)
    {
        return;
    }

    //is the structure in Range???
    float fAdminListDistance = gWorldConfig->getConfiguration<float>("Player_Admin_List_Distance",(float)32.0);

    if(player->getParentId())
    {
        if(CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId())))
        {
            if(HouseObject* house = dynamic_cast<HouseObject*>(gWorldManager->getObjectById(cell->getParentId())))
            {
                if(house->getId() != structure->getId())
                {
                    gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "command_no_building"), player);
                    return;
                }
            }
        }

    }
    else if(glm::distance(player->mPosition, structure->mPosition) > fAdminListDistance)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "command_no_building"), player);
        return;
    }

    player->setStructurePermissionId(0);

    StructureAsyncCommand command;
    command.PlayerId = player->getId();
    command.StructureId = structure->getId();
    command.List = list;
    command.PlayerStr = playerStr;

    if(action == "add")
    {
        command.Command = Structure_Command_AddPermission;
        gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
    }

    if(action == "remove")
    {
        command.Command = Structure_Command_RemovePermission;
        gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
    }

}

//======================================================================================================================
//
// Modifies the Admin List
//
void	ObjectController::_handleTransferStructure(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // requirement we have the structure targeted AND give the name of the recipient on the commandline
    // OR we have the recipient targeted and stand NEXT to the structure were about to transfer

    //do we have a valid donor ?
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if(!player)
    {
        return;
    }


    // is the player online and near 30m ?
    // we get the  players id as targetid if yes, otherwise we get the name as string
    // however, we do not want players that  are not online

    //now get the target player
    PlayerObject*	recipient	= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId));

    if(!recipient)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "no_transfer_target"), player);
        return;
    }

    //do we have a valid structure ??? check our target first
    uint64 id = player->getTargetId();
    Object* object = gWorldManager->getObjectById(id);
    PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

    if(!structure)
    {
        // we need to get the nearest structure that we own
        // for now dustoff
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "command_no_building"), player);
        return;
    }

    //is the structure in Range???
    float fTransferDistance = gWorldConfig->getConfiguration<float>("Player_Transfer_Structure_Distance",(float)8.0);
    if(glm::distance(player->mPosition, structure->mPosition) > fTransferDistance)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "command_no_building"), player);
        return;
    }

    StructureAsyncCommand command;
    command.PlayerId = player->getId();
    command.StructureId = structure->getId();
    command.RecipientId = recipient->getId();
    command.PlayerStr = recipient->getFirstName().getAnsi();
    command.Command = Structure_Command_TransferStructure;

    gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

}





//======================================================================================================================
//
// Modifies the Admin List
//
void	ObjectController::_handleNameStructure(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)

{
    // requirement we have the structure targeted AND give the name of the recipient on the commandline
    // OR we have the recipient targeted and stand NEXT to the structure were about to transfer

    //do we have a valid donor ?
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if(!player)
    {
        return;
    }

    //do we have a valid structure ??? check our target first
    uint64 id = player->getTargetId();
    Object* object = gWorldManager->getObjectById(id);
    PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

    if(!structure)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "command_no_building"), player);
        return;
    }

    //is the structure in Range???
    float fTransferDistance = gWorldConfig->getConfiguration<float>("Player_Structure_Operate_Distance",(float)10.0);
    if(glm::distance(player->mPosition, structure->mPosition) > fTransferDistance)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "command_no_building"), player);
        return;
    }

    //find out where our structure is
    BString dataStr;
    message->getStringUnicode16(dataStr);

    BString nameStr;

    dataStr.convert(BSTRType_ANSI);

    sscanf(dataStr.getAnsi(),"%s",nameStr.getAnsi());

    if(nameStr.getLength() > 68)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "not_valid_name"), player);
        return;
    }

    StructureAsyncCommand command;
    command.Command = Structure_Command_RenameStructure;
    command.PlayerId = player->getId();
    command.StructureId = structure->getId();

    gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

}

//======================================================================================================================
//
// provides the harvester with the current resources
//
void	ObjectController::_handleHarvesterGetResourceData(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);
    if(!player)    {
        return;
    }

    //do we have a valid structure ???
    uint64 id = targetId;
    Object* object = gWorldManager->getObjectById(id);
    PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

    if(!structure)
    {
        //gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
        return;
    }

    //is the structure in Range???
    float fTransferDistance = gWorldConfig->getConfiguration<float>("Player_Structure_Operate_Distance",(float)10.0);
    if(glm::distance(player->mPosition, structure->mPosition) > fTransferDistance)
    {
        DLOG(INFO) << " ObjectController::_handleHarvesterGetResourceData Structure not in Range";
        return;
    }

    HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

    StructureAsyncCommand command;

    command.Command = Structure_Command_GetResourceData;
    command.PlayerId = player->getId();
    command.StructureId = structure->getId();

    gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

    return;
    gMessageLib->sendHarvesterResourceData(structure,player);

    DLOG(INFO) << " ObjectController::_handleHarvesterGetResourceData :: hino 7 baseline";
    gMessageLib->sendBaselinesHINO_7(harvester,player);

    //add the structure to the timer so the resource amounts are updated while we look at the hopper
    //harvester->getTTS()->todo		= ttE_UpdateHopper;
    //harvester->getTTS()->playerId	= player->getId();
    //structure->getTTS()->projectedTime = 5000 + Anh_Utils::Clock::getSingleton()->getLocalTime();
    //gStructureManager->addStructureforHopperUpdate(harvester->getId());

    // this needs to be handled zoneserverside - otherwise the addition of a res will trigger a racecondition
    // between the sql write query and the sql read please note that the harvesting itself happens through stored procedures
    // and we cant keep the updatecounters synchronized


}



//======================================================================================================================
//
// Selects the resource for extraction
//
void	ObjectController::_handleHarvesterSelectResource(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if(!player)
    {
        return;
    }

    //do we have a valid structure ???
    uint64 id = targetId;
    Object* object = gWorldManager->getObjectById(id);
    PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

    if(!structure)
    {
        //gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
        return;
    }

    //is the structure in Range???
    float fTransferDistance = gWorldConfig->getConfiguration<float>("Player_Structure_Operate_Distance",(float)10.0);
    if(glm::distance(player->mPosition, structure->mPosition) > fTransferDistance)
    {
        DLOG(INFO) << " ObjectController::_handleHarvesterSelectResource Structure not in Range";
        return;
    }

    HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

    //get the relevant Resource
    BString dataStr;
    message->getStringUnicode16(dataStr);

    uint64 resourceId;
    swscanf(dataStr.getUnicode16(),L"%"WidePRIu64,&resourceId);

    Resource* tmpResource = gResourceManager->getResourceById(resourceId);

    if((!tmpResource)||(!tmpResource->getCurrent()))
    {
        DLOG(INFO) << " ObjectController::_handleHarvesterSelectResource No valid resource!";
        return;
    }

    harvester->setCurrentResource(resourceId);

    // update the current resource in the db
    mDatabase->executeSqlAsync(0,0,"UPDATE %s.harvesters SET ResourceID=%" PRIu64 " WHERE id=%" PRIu64 " ",mDatabase->galaxy(),resourceId,harvester->getId());

    CurrentResource* cR = reinterpret_cast<CurrentResource*>(tmpResource);
    //resource = reinterpret_cast<CurrentResource*>(gResourceManager->getResourceByNameCRC(resourceName.getCrc()));

    float posX, posZ;
    float ratio = 0.0;

    posX	= harvester->mPosition.x;
    posZ	= harvester->mPosition.z;


    if(cR)
    {
        ratio	= cR->getDistribution((int)posX + 8192,(int)posZ + 8192);
        if(ratio > 1.0)
        {
            ratio = 1.0;
        }
    }

    float ber = harvester->getSpecExtraction();

    harvester->setCurrentExtractionRate(ber*ratio);

    // now enter the new resource in the hoppers resource list if its isnt already in there
    // TODO keep the list up to date by removing unnecessary resources
    // to this end read the list anew and delete every resource with zero amount
    // have a stored function do this

    if(!harvester->checkResourceList(resourceId))
    {
        //do *not* add to list - otherwise we get a racecondition with the asynch update from db !!!
        //harvester->getResourceList()->push_back(std::make_pair(resourceId,float(0.0)));
        //add to db
        mDatabase->executeSqlAsync(0,0,"INSERT INTO %s.harvester_resources VALUES(%" PRIu64 ",%" PRIu64 ",0,0)",mDatabase->galaxy(),harvester->getId(),resourceId);
     
    }

    // update the current extractionrate in the db for the stored procedure handling the harvesting
    mDatabase->executeSqlAsync(0,0,"UPDATE %s.harvesters SET rate=%f WHERE id=%" PRIu64 " ",mDatabase->galaxy(),(ber*ratio),harvester->getId());
    

    //now send the updates
    gMessageLib->sendCurrentResourceUpdate(harvester,player);
    gMessageLib->sendCurrentExtractionRate(harvester,player);

}


//======================================================================================================================
//
// Turns a harvester on
//

void	ObjectController::_handleHarvesterActivate(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if(!player)
    {
        return;
    }

    //do we have a valid structure ???
    uint64 id = targetId;
    Object* object = gWorldManager->getObjectById(id);
    PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

    if(!structure)
    {
        //gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
        return;
    }

    //is the structure in Range???
    float fTransferDistance = gWorldConfig->getConfiguration<float>("Player_Structure_Operate_Distance",(float)10.0);
    if(glm::distance(player->mPosition, structure->mPosition) > fTransferDistance)
    {
        return;
    }

    HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

    harvester->setActive(true);

    //send the respective delta
    gMessageLib->sendHarvesterActive(harvester);

    //send the db update
    mDatabase->executeSqlAsync(0,0,"UPDATE %s.harvesters SET active= 1 WHERE id=%" PRIu64 " ",mDatabase->galaxy(),harvester->getId());
    

}

//======================================================================================================================
//
// Turns a harvester off
//

void	ObjectController::_handleHarvesterDeActivate(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if(!player)
    {
        return;
    }

    //do we have a valid structure ???
    uint64 id = targetId;
    Object* object = gWorldManager->getObjectById(id);
    PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

    if(!structure)
    {
        //gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
        return;
    }

    //is the structure in Range???
    float fTransferDistance = gWorldConfig->getConfiguration<float>("Player_Structure_Operate_Distance",(float)10.0);
    if(glm::distance(player->mPosition, structure->mPosition) > fTransferDistance)
    {
        return;
    }

    HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);

    harvester->setActive(false);

    //send the respective delta
    gMessageLib->sendHarvesterActive(harvester);

    //send the db update
    mDatabase->executeSqlAsync(0,0,"UPDATE %s.harvesters SET active = 0 WHERE id=%" PRIu64 " ",mDatabase->galaxy(),harvester->getId());
    

}


//======================================================================================================================
//
// Discards the contents of a harvesters Hopper
//
void	ObjectController::_handleDiscardHopper(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if(!player)
    {
        return;
    }

    //do we have a valid structure ???
    uint64 id = targetId;
    Object* object = gWorldManager->getObjectById(id);
    PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

    if(!structure)
    {
        //gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
        return;
    }

    //is the structure in Range???
    float fTransferDistance = gWorldConfig->getConfiguration<float>("Player_Structure_Operate_Distance",(float)10.0);
    if(glm::distance(player->mPosition, structure->mPosition) > fTransferDistance)
    {
        return;
    }

    StructureAsyncCommand command;

    command.Command = Structure_Command_DiscardHopper;
    command.PlayerId = player->getId();
    command.StructureId = structure->getId();

    gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);


}


//=============================================================================================================================
// discards x amount of specified resource

void	ObjectController::handleResourceEmptyHopper(Message* message)
{
    uint64 playerId;
    uint64 harvesterId;

    message->getUint64(playerId);
    message->getUint64(harvesterId);

    PlayerObject*   player  = dynamic_cast<PlayerObject*>(mObject);

    if(!player)
    {
        return;
    }

    //do we have a valid structure ???
    Object* object = gWorldManager->getObjectById(harvesterId);
    PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object);

    if(!structure)
    {
        //gMessageLib->sendSystemMessage(player,L"","player_structure","command_no_building");
        return;
    }

    //is the structure in Range???
    float fTransferDistance = gWorldConfig->getConfiguration<float>("Player_Structure_Operate_Distance",(float)10.0);
    if(glm::distance(player->mPosition, structure->mPosition) > fTransferDistance)
    {
        return;
    }

    uint64 resourceId;
    uint32 amount;
    uint8 b1, b2;

    message->getUint64(resourceId);
    message->getUint32(amount);
    message->getUint8(b1);
    message->getUint8(b2);

    StructureAsyncCommand command;

    if(b1 == 0)
    {
        command.Command		=	Structure_Command_RetrieveResource;
        command.PlayerId	=	player->getId();
        command.StructureId =	structure->getId();
        command.ResourceId	=	resourceId;
        command.Amount		=	amount;
        command.b1 = b1;
        command.b2 = b2;
    }
    if(b1 == 1)
    {
        command.Command		=	Structure_Command_DiscardResource;
        command.PlayerId	=	player->getId();
        command.StructureId =	structure->getId();
        command.ResourceId	=	resourceId;
        command.Amount		=	amount;
        command.b1 = b1;
        command.b2 = b2;

    }

    gStructureManager->checkNameOnPermissionList(structure->getId(),player->getId(),player->getFirstName().getAnsi(),"HOPPER",command);

}



//======================================================================================================================
//
// moves an item
//
void	ObjectController::HandleItemMoveForward_(
    uint64 targetId,
    Message* message,
    ObjectControllerCmdProperties* cmdProperties) {

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if (!player)	{
        assert(false && "ObjectController::HandleItemMoveForward_ Player not found");
        return;
    }

    // Verify that there was a target passed.
    if (!targetId) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }

    Object* object = gWorldManager->getObjectById(targetId);

    if(!object)	{
        assert(false && "ObjectController::HandleItemMoveForward_ item not found");
        return;
    }

    // Verify that the item and player are in the same structure.
    CellObject* playerCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
    if(!playerCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }
    uint64 playerStructure = playerCell->getParentId();

    CellObject* objectCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
    if(!objectCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }
    uint64 objectStructure = objectCell->getParentId();

    if (objectStructure != playerStructure) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }

    // Verify that the player has appropriate rights on this structure.
    if (playerCell) {
        if (BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(playerCell->getParentId()))) {
            if (!building->hasAdminRights(player->getId())) {
                gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "admin_move_only"), player);
                return;
            }
        }	else {
            assert(false && "ObjectController::HandleItemMoveForward_ no structure");
            return;
        }
    } else {
        //so were not in a building ??
        return;
    }

    // Move the object forward 1/10th of a meter.
    object->move(player->mDirection, MOVE_INCREMENT);

    gMessageLib->sendDataTransformWithParent053(object);
    object->updateWorldPosition();
}


//======================================================================================================================
//
// moves an item
//
void	ObjectController::HandleItemMoveBack_(
    uint64 targetId,
    Message* message,
    ObjectControllerCmdProperties* cmdProperties) {

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if (!player)	{
        assert(false && "ObjectController::HandleItemMoveBack_ Player not found");
        return;
    }

    // Verify that there was a target passed.
    if (!targetId) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }

    Object* object = gWorldManager->getObjectById(targetId);

    if(!object)	{
        assert(false && "ObjectController::HandleItemMoveBack_ item not found");
        return;
    }

    // Verify that the item and player are in the same structure.
    // Verify that the item and player are in the same structure.
    CellObject* playerCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
    if(!playerCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }
    uint64 playerStructure = playerCell->getParentId();

    CellObject* objectCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
    if(!objectCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }
    uint64 objectStructure = objectCell->getParentId();

    if (objectStructure != playerStructure) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }

    // Verify that the player has appropriate rights on this structure.
    if (playerCell) {
        if (BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(playerCell->getParentId()))) {
            if (!building->hasAdminRights(player->getId())) {
                gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "admin_move_only"), player);
                return;
            }
        }	else {
            assert(false && "ObjectController::HandleItemMoveBack_ no structure");
            return;
        }
    } else {
        //so were just outside ??
        return;
    }

    // Move the object back 1/10th of a meter.
    object->move(player->mDirection, -MOVE_INCREMENT);

    gMessageLib->sendDataTransformWithParent053(object);
    object->updateWorldPosition();
}


//======================================================================================================================
//
// moves an item
//
void	ObjectController::HandleItemMoveUp_(
    uint64 targetId,
    Message* message,
    ObjectControllerCmdProperties* cmdProperties) {

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if (!player)	{
        assert(false && "ObjectController::HandleItemMoveUp_ Player not found");
        return;
    }

    // Verify that there was a target passed.
    if (!targetId) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }

    Object* object = gWorldManager->getObjectById(targetId);

    if(!object)	{
        assert(false && "ObjectController::HandleItemMoveUp_ item not found");
        return;
    }

    // Verify that the item and player are in the same structure.
    // Verify that the item and player are in the same structure.
    CellObject* playerCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
    if(!playerCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }
    uint64 playerStructure = playerCell->getParentId();

    CellObject* objectCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
    if(!objectCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }
    uint64 objectStructure = objectCell->getParentId();

    if (objectStructure != playerStructure) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }

    // Verify that the player has appropriate rights on this structure.
    if (playerCell) {
        if (BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(playerCell->getParentId()))) {
            if (!building->hasAdminRights(player->getId())) {
                gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "admin_move_only"), player);
                return;
            }
        }	else {
            assert(false && "ObjectController::HandleItemMoveUp_ no structure");
            return;
        }
    } else {
        //so were just outside
        return;
    }

    object->mPosition.y += MOVE_INCREMENT;

    gMessageLib->sendDataTransformWithParent053(object);
    object->updateWorldPosition();
}


//======================================================================================================================
//
// moves an item
//
void ObjectController::HandleItemMoveDown_(
    uint64 targetId,
    Message* message,
    ObjectControllerCmdProperties* cmdProperties) {

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if (!player)	{
        assert(false && "ObjectController::HandleItemMoveDown_ Player not found");
        return;
    }

    // Verify that there was a target passed.
    if (!targetId) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }

    Object* object = gWorldManager->getObjectById(targetId);

    if(!object)	{
        assert(false && "ObjectController::HandleItemMoveDown_ item not found");
        return;
    }

    // Verify that the item and player are in the same structure.
    // Verify that the item and player are in the same structure.
    CellObject* playerCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
    if(!playerCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }
    uint64 playerStructure = playerCell->getParentId();

    CellObject* objectCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
    if(!objectCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }
    uint64 objectStructure = objectCell->getParentId();

    if (objectStructure != playerStructure) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return;
    }

    // Verify that the player has appropriate rights on this structure.
    if (playerCell) {
        if (BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(playerCell->getParentId()))) {
            if (!building->hasAdminRights(player->getId())) {
                gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "admin_move_only"), player);
                return;
            }
        }	else {
            assert(false && "ObjectController::HandleItemMoveDown_ no structure");
            return;
        }
    } else {
        //were just outside??
        return;
    }

    object->mPosition.y -= MOVE_INCREMENT;

    gMessageLib->sendDataTransformWithParent053(object);
    object->updateWorldPosition();
}


//======================================================================================================================
//
// rotates an item	 90d to right
//
void	ObjectController::HandleItemRotateRight_(
    uint64 targetId,
    Message* message,
    ObjectControllerCmdProperties* cmdProperties) {

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if (!player)	{
        assert(false && "ObjectController::HandleItemRotateRight_ Player not found");
        return;
    }

    // Verify that there was a target passed.
    if (!targetId) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }

    Object* object = gWorldManager->getObjectById(targetId);

    if(!object)	{
        assert(false && "ObjectController::HandleItemRotateRight_ item not found");
        return;
    }

    // Verify that the item and player are in the same structure.
    // Verify that the item and player are in the same structure.
    CellObject* playerCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
    if(!playerCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }
    uint64 playerStructure = playerCell->getParentId();

    CellObject* objectCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
    if(!objectCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }
    uint64 objectStructure = objectCell->getParentId();

    if (objectStructure != playerStructure) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }

    // Verify that the player has appropriate rights on this structure.
    if (playerCell) {
        if (BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(playerCell->getParentId()))) {
            if (!building->hasAdminRights(player->getId())) {
                gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "admin_move_only"), player);
                return;
            }
        }	else {
            assert(false && "ObjectController::HandleItemRotateRight_ no structure");
            return;
        }
    } else {
        //were just outside
        return;
    }

    // Rotate the object 90 degree's to the right
    object->rotateRight(ROTATE_INCREMENT);

    gMessageLib->sendDataTransformWithParent053(object);
    object->updateWorldPosition();
}

//======================================================================================================================
//
// rotates an item 90d to left
//
void ObjectController::HandleItemRotateLeft_(
    uint64 targetId,
    Message* message,
    ObjectControllerCmdProperties* cmdProperties) {

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if (!player)	{
        assert(false && "ObjectController::HandleItemRotateLeft_ Player not found");
        return;
    }

    // Verify that there was a target passed.
    if (!targetId) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }

    Object* object = gWorldManager->getObjectById(targetId);

    if(!object)	{
        assert(false && "ObjectController::HandleItemRotateLeft_ item not found");
        return;
    }

    // Verify that the item and player are in the same structure.
    // Verify that the item and player are in the same structure.
    CellObject* playerCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
    if(!playerCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }
    uint64 playerStructure = playerCell->getParentId();

    CellObject* objectCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
    if(!objectCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }
    uint64 objectStructure = objectCell->getParentId();

    if (objectStructure != playerStructure) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }

    // Verify that the player has appropriate rights on this structure.
    if (playerCell) {
        if (BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(playerCell->getParentId()))) {
            if (!building->hasAdminRights(player->getId())) {
                gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "admin_move_only"), player);
                return;
            }
        }	else {
            assert(false && "ObjectController::HandleItemRotateLeft_ no structure");
            return;
        }
    } else {
        //were just outside
        return;
    }

    // Rotate the item 90 degrees to the left
    object->rotateLeft(ROTATE_INCREMENT);

    gMessageLib->sendDataTransformWithParent053(object);
    object->updateWorldPosition();
}

//======================================================================================================================
//
// rotates an item
//
void ObjectController::HandleRotateFurniture_(
    uint64 targetId,
    Message* message,
    ObjectControllerCmdProperties* cmdProperties) {

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if (!player)	{
        assert(false && "ObjectController::HandleRotateFurniture_ Player not found");
        return;
    }

    // Verify that there was a target passed.
    if (!targetId) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }

    Object* object = gWorldManager->getObjectById(targetId);

    if(!object)	{
        assert(false && "ObjectController::HandleRotateFurniture_ item not found");
        return;
    }

    // Verify that the item and player are in the same structure.
    CellObject* playerCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
    if(!playerCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }
    uint64 playerStructure = playerCell->getParentId();

    CellObject* objectCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
    if(!objectCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }
    uint64 objectStructure = objectCell->getParentId();

    if (objectStructure != playerStructure) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_what"), player);
        return;
    }

    // Verify that the player has appropriate rights on this structure.
    if (playerCell) {
        if (BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(playerCell->getParentId()))) {
            if (!building->hasAdminRights(player->getId())) {
                gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "admin_move_only"), player);
                return;
            }
        }	else {
            assert(false && "ObjectController::HandleRotateFurniture_ no structure");
            return;
        }
    } else {
        //were just outside??
        return;
    }

    // Read the message out of the packet.
    BString tmp;
    message->getStringUnicode16(tmp);

    // If the string has no length the message is ill-formatted, send the
    // proper format to the client.
    if (!tmp.getLength()) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "formet_rotratefurniture_degrees"), player);
        return;
    }

    // Convert the string to an ansi string for ease with the regex.
    tmp.convert(BSTRType_ANSI);
    std::string input_string(tmp.getAnsi());

    static const regex pattern("(right|left) ([0-9]+)");
    smatch result;

    regex_search(input_string, result, pattern);

    // If the pattern doesn't match all elements then send the proper format
    // to the client.
    if (result.length() < 2) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "formet_rotratefurniture_degrees"), player);
        return;
    }

    // Gather the results of the pattern for validation and use.
    std::string direction(result[1]);
    float degrees = boost::lexical_cast<float>(result[2]);

    // If the the specified amount is not within the valid range notify the client.
    if (degrees < 1.0f || degrees > 180.0f) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "rotate_params"), player);
        return;
    }

    // Rotate by the necessary amount.
    if (direction.compare("left") == 0) {
        object->rotateLeft(degrees);
    } else {
        object->rotateRight(degrees);
    }

    // Update the world with the changes.
    gMessageLib->sendDataTransformWithParent053(object);
    object->updateWorldPosition();
}


bool HandleMoveFurniture(
    Object* object,
    Object* target,
    Message* message,
    ObjectControllerCmdProperties* cmdProperties) {

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(object);

    if (!player)	{
        assert(false && "ObjectController::HandleItemMoveDown_ Player not found");
        return false;
    }

    // Verify that there was a target passed.
    if (!target) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return false;
    }

    if(!target)	{
        assert(false && "ObjectController::HandleItemMoveDown_ item not found");
        return false;
    }

    // Verify that the item and player are in the same structure.
    CellObject* playerCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(player->getParentId()));
    if(!playerCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return false;
    }
    uint64 playerStructure = playerCell->getParentId();

    CellObject* objectCell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
    if(!objectCell)	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return false;
    }
    uint64 objectStructure = objectCell->getParentId();

    if (objectStructure != playerStructure) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "move_what"), player);
        return false;
    }

    // Verify that the player has appropriate rights on this structure.
    if (playerCell) {
        if (BuildingObject* building = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(playerCell->getParentId()))) {
            if (!building->hasAdminRights(player->getId())) {
                gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "admin_move_only"), player);
                return false;
            }
        }	else {
            assert(false && "ObjectController::HandleItemMoveDown_ no structure");
            return false;
        }
    } else {
        //were just outside ??
        return false;
    }

    // Read the message out of the packet.
    BString tmp;
    message->getStringUnicode16(tmp);

    // If the string has no length the message is ill-formatted, send the
    // proper format to the client.
    if (!tmp.getLength()) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "format_movefurniture_distance"), player);
        return false;
    }

    // Convert the string to an ansi string for ease with the regex.
    tmp.convert(BSTRType_ANSI);
    std::string input_string(tmp.getAnsi());

    static const regex pattern("(forward|back|up|down) ([0-9]+)");
    smatch result;

    regex_search(input_string, result, pattern);

    // If the pattern doesn't match all elements then send the proper format
    // to the client.
    if (result.length() < 2) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "format_movefurniture_distance"), player);
        return false;
    }

    // Gather the results of the pattern for validation and use.
    std::string direction(result[1]);
    float distance = boost::lexical_cast<float>(result[2]);

    // If the the specified amount is not within the valid range notify the client.
    if ((distance < 1.0f) || (distance > 500.0f)) {
        gMessageLib->SendSystemMessage(::common::OutOfBand("player_structure", "movefurniture_params"), player);
        return false;
    }

    // Move object an MOVE_INCREMENT times by the amount and direction specified.
    if (direction == "forward") {
        target->move(player->mDirection, distance * MOVE_INCREMENT);
    } else if (direction == "back") {
        target->move(player->mDirection, -distance * MOVE_INCREMENT);
    } else if (direction == "up") {
        target->mPosition.y += distance * MOVE_INCREMENT;
    } else if (direction == "down") {
        target->mPosition.y -= distance * MOVE_INCREMENT;
    }

    // Update the world with the changes.
    gMessageLib->sendDataTransformWithParent053(target);
    target->updateWorldPosition();

    return true;
}
