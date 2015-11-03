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
#include "BankTerminal.h"
#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Datapad.h"
#include "Item.h"
#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "SurveyTool.h"
#include "TravelMapHandler.h"
#include "UIManager.h"
#include "WaypointObject.h"
#include "WorldManager.h"
#include "WorldConfig.h"
#include "Wearable.h"
#include "ZoneOpcodes.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/Message.h"
#include "MessageLib/MessageLib.h"


//======================================================================================================================
//
// add friend
//

void ObjectController::_handleAddFriend(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	player		= dynamic_cast<PlayerObject*>(mObject);

    if(player->getContactListUpdatePending())
        return;
    else
        player->setContactListUpdatePending(true);

    BString	friendName;
    int8	sql[1024],end[16],*sqlPointer;

    message->getStringUnicode16(friendName);
    friendName.convert(BSTRType_ANSI);

    if(!friendName.getLength())
    {
        player->setContactListUpdatePending(false);
        return;
    }

    if(player->isConnected())
        gMessageLib->sendHeartBeat(player->getClient());

    friendName.toLower();

    // check if he's already our friend
    if(player->checkFriendList(friendName.getCrc()))
    {
        friendName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "friend_duplicate", L"", friendName.getUnicode16(), L""), player);
        player->setContactListUpdatePending(false);
        return;
    }

    // or ignored

    if(player->checkIgnoreList(friendName.getCrc()))
    {
        friendName.convert(BSTRType_Unicode16);

        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "friend_fail_is_ignored", L"", friendName.getUnicode16(), L""), player);
        player->setContactListUpdatePending(false);
        return;
    }

    // check our own name
    BString firstName = player->getFirstName().getAnsi();
    firstName.toLower();

    if(strcmp(firstName.getAnsi(),friendName.getAnsi()) == 0)
    {
        player->setContactListUpdatePending(false);
        return;
    }

    // pull the db query
    ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_AddFriend);
    asyncContainer->mString = friendName.getAnsi();

    sprintf(sql, "SELECT %s.sf_addFriend(%" PRIu64 ",'", mDatabase->galaxy(), player->getId());
    sprintf(end,"')");
    sqlPointer = sql + strlen(sql);
    sqlPointer += mDatabase->escapeString(sqlPointer,friendName.getAnsi(),friendName.getLength());
    strcat(sql,end);

    mDatabase->executeSqlAsync(this,asyncContainer,sql);
    
}

//======================================================================================================================
//
// remove friend
//

void ObjectController::_handleRemoveFriend(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if(player->getContactListUpdatePending())
        return;
    else
        player->setContactListUpdatePending(true);

    BString	friendName;
    int8	sql[1024],end[16],*sqlPointer;

    message->getStringUnicode16(friendName);
    friendName.convert(BSTRType_ANSI);

    if(!friendName.getLength())
    {
        player->setContactListUpdatePending(false);
        return;
    }


    if(player->isConnected())
        gMessageLib->sendHeartBeat(player->getClient());

    friendName.toLower();

    if(!player->checkFriendList(friendName.getCrc()))
    {
        player->setContactListUpdatePending(false);
        return;
    }
    ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_RemoveFriend);
    asyncContainer->mString = friendName.getAnsi();

    sprintf(sql, "SELECT %s.sf_removeFriend(%" PRIu64 ",'", mDatabase->galaxy(), player->getId());
    sprintf(end,"')");
    sqlPointer = sql + strlen(sql);
    sqlPointer += mDatabase->escapeString(sqlPointer,friendName.getAnsi(),friendName.getLength());
    strcat(sql,end);

    mDatabase->executeSqlAsync(this,asyncContainer,sql);
    

}

//======================================================================================================================
//
// add ignore
//

void ObjectController::_handleAddIgnore(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if(player->getContactListUpdatePending())
    {
        return;
    }
    else
    {
        player->setContactListUpdatePending(true);
    }

    BString	ignoreName;
    int8	sql[2048],end[16],*sqlPointer;

    message->getStringUnicode16(ignoreName);
    ignoreName.convert(BSTRType_ANSI);

    if(!ignoreName.getLength())
    {
        player->setContactListUpdatePending(false);
        return;
    }

    if(player->isConnected())
        gMessageLib->sendHeartBeat(player->getClient());

    ignoreName.toLower();

    // check our ignorelist
    if(player->checkIgnoreList(ignoreName.getCrc()))
    {
        ignoreName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "ignore_duplicate", L"", ignoreName.getUnicode16(), L""), player);
        player->setContactListUpdatePending(false);
        return;
    }

    // friends CAN be ignored!!!
    //if(player->checkFriendList(ignoreName.getCrc()))
    //{
    //	ignoreName.convert(BSTRType_Unicode16);
    //	gMessageLib->sendSystemMessage(player,L"","cmnty","friend_fail_is_ignored","","",L"",0,"","",ignoreName);
    //	player->setContactListUpdatePending(false);
    //	return;
    //}

    // check our own name
    BString firstName = player->getFirstName().getAnsi();
    firstName.toLower();

    if(strcmp(firstName.getAnsi(),ignoreName.getAnsi()) == 0)
    {
        player->setContactListUpdatePending(false);
        return;
    }

    // pull the db query
    ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_AddIgnore);
    asyncContainer->mString = ignoreName.getAnsi();

    sprintf(sql, "SELECT %s.sf_addIgnore(%" PRIu64 ",'", mDatabase->galaxy(), player->getId());
    sprintf(end,"')");
    sqlPointer = sql + strlen(sql);
    sqlPointer += mDatabase->escapeString(sqlPointer,ignoreName.getAnsi(),ignoreName.getLength());
    strcat(sql,end);

    mDatabase->executeSqlAsync(this,asyncContainer,sql);
    

}

//======================================================================================================================
//
// remove ignore
//

void ObjectController::_handleRemoveIgnore(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    if(player->getContactListUpdatePending())
        return;
    else
        player->setContactListUpdatePending(true);

    BString	ignoreName;
    int8	sql[2048],end[16],*sqlPointer;

    message->getStringUnicode16(ignoreName);
    ignoreName.convert(BSTRType_ANSI);

    if(!ignoreName.getLength())
    {
        player->setContactListUpdatePending(false);
        return;
    }

    if(player->isConnected())
        gMessageLib->sendHeartBeat(player->getClient());

    ignoreName.toLower();

    if(!player->checkIgnoreList(ignoreName.getCrc()))
    {
        player->setContactListUpdatePending(false);
        return;
    }

    ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_RemoveIgnore);
    asyncContainer->mString = ignoreName.getAnsi();

    sprintf(sql, "SELECT %s.sf_removeIgnore(%" PRIu64 ",'", mDatabase->galaxy(), player->getId());
    sprintf(end,"')");
    sqlPointer = sql + strlen(sql);
    sqlPointer += mDatabase->escapeString(sqlPointer,ignoreName.getAnsi(),ignoreName.getLength());
    strcat(sql,end);

    mDatabase->executeSqlAsync(this,asyncContainer,sql);
    
}

//======================================================================================================================
//
// add friend db reply
//

void ObjectController::_handleAddFriendDBReply(uint32 retCode,BString friendName)
{
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    switch(retCode)
    {
        // no such name
    case 0:
    default:
    {
        friendName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "friend_not_found", L"", friendName.getUnicode16(), L""), player);

    }
    break;

    // add ok
    case 1:
    {
        // update list
        player->addFriend(friendName.getAnsi());
        gMessageLib->sendFriendListPlay9(player);

        // send notification
        friendName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "friend_added", L"", friendName.getUnicode16(), L""), player);

        // notify chat server
        if(player->isConnected())
        {
            gMessageFactory->StartMessage();
            gMessageFactory->addUint32(opNotifyChatAddFriend);
            gMessageFactory->addString(friendName);
            Message* message = gMessageFactory->EndMessage();

            player->getClient()->SendChannelA(message,player->getAccountId(),CR_Chat,2);
        }
    }
    break;
    }

    player->setContactListUpdatePending(false);
}

//======================================================================================================================
//
// add friend db reply
//

void ObjectController::_handleFindFriendDBReply(uint64 retCode,BString friendName)
{
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);
    friendName.convert(BSTRType_Unicode16);
    if(retCode == 0)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "friend_location_failed_noname", L"", friendName.getUnicode16(), L""), player);
        return;
    }

    PlayerObject*	searchObject	= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(retCode));

    if(!searchObject)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "friend_location_failed", L"", friendName.getUnicode16(), L""), player);
        return;
    }

    //are we on our targets friendlist???
    if(!searchObject->checkFriendList(player->getFirstName().getCrc()))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "friend_location_failed", L"", friendName.getUnicode16(), L""), player);
        return;
    }

    Datapad* datapad			= player->getDataPad();

    if(datapad && datapad->getCapacity())
    {
        //the datapad automatically checks for waypoint caspacity and gives the relevant error messages
        datapad->requestNewWaypoint(searchObject->getFirstName().getAnsi(),searchObject->mPosition,static_cast<uint16>(gWorldManager->getZoneId()),Waypoint_blue);
    }
}

//======================================================================================================================
//
// remove friend db reply
//

void ObjectController::_handleRemoveFriendDBReply(uint32 retCode,BString friendName)
{
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    switch(retCode)
    {
        // no such name
    case 0:
    default:
    {
        friendName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "friend_not_found", L"", friendName.getUnicode16(), L""), player);
    }
    break;

    // remove ok
    case 1:
    {
        // update list
        player->removeFriend(friendName.getCrc());
        gMessageLib->sendFriendListPlay9(player);

        // send notification
        friendName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "friend_removed", L"", friendName.getUnicode16(), L""), player);

        if(player->isConnected())
        {
            // notify chat server
            gMessageFactory->StartMessage();
            gMessageFactory->addUint32(opNotifyChatRemoveFriend);
            gMessageFactory->addString(friendName);
            Message* message = gMessageFactory->EndMessage();

            player->getClient()->SendChannelA(message,player->getAccountId(),CR_Chat,2);
        }
    }
    break;
    }

    player->setContactListUpdatePending(false);
}


//======================================================================================================================
//
// add ignore db reply
//

void ObjectController::_handleAddIgnoreDBReply(uint32 retCode,BString ignoreName)
{

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    switch(retCode)
    {
        // no such name
    case 0:
    default:
    {
        ignoreName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "ignore_not_found", L"", ignoreName.getUnicode16(), L""), player);
    }
    break;

    // add ok
    case 1:
    {
        // update list
        player->addIgnore(ignoreName.getAnsi());
        gMessageLib->sendIgnoreListPlay9(player);

        // send notification
        ignoreName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "ignore_added", L"", ignoreName.getUnicode16(), L""), player);

        // notify chat server
        if(player->isConnected())
        {
            gMessageFactory->StartMessage();
            gMessageFactory->addUint32(opNotifyChatAddIgnore);
            gMessageFactory->addString(ignoreName);
            Message* message = gMessageFactory->EndMessage();

            player->getClient()->SendChannelA(message,player->getAccountId(),CR_Chat,2);
        }
    }
    break;
    }

    player->setContactListUpdatePending(false);
}

//======================================================================================================================
//
// remove ignore db reply
//

void ObjectController::_handleRemoveIgnoreDBReply(uint32 retCode,BString ignoreName)
{
    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

    switch(retCode)
    {
        // no such name
    case 0:
    default:
    {
        ignoreName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "ignore_not_found", L"", ignoreName.getUnicode16(), L""), player);
    }
    break;

    // remove ok
    case 1:
    {
        // update list
        player->removeIgnore(ignoreName.getCrc());
        gMessageLib->sendIgnoreListPlay9(player);

        // send notification
        ignoreName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("cmnty", "ignore_removed", L"", ignoreName.getUnicode16(), L""), player);

        // notify chat server
        if(player->isConnected())
        {
            gMessageFactory->StartMessage();
            gMessageFactory->addUint32(opNotifyChatRemoveIgnore);
            gMessageFactory->addString(ignoreName);
            Message* message = gMessageFactory->EndMessage();

            player->getClient()->SendChannelA(message,player->getAccountId(),CR_Chat,2);
        }
    }
    break;
    }

    player->setContactListUpdatePending(false);
}

//======================================================================================================================

//

//======================================================================================================================
//
// find a friend
//

void ObjectController::_handlefindfriend(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
    BString			friendName;

    message->getStringUnicode16(friendName);

    if(!friendName.getLength())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("ui_cmnty", "friend_location_failed_usage"), playerObject);
        return;
    }

    if(playerObject->isConnected())
    {
        // query the chat server
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opNotifyChatFindFriend);
        gMessageFactory->addString(friendName);
        Message* message = gMessageFactory->EndMessage();

        playerObject->getClient()->SendChannelA(message,playerObject->getAccountId(),CR_Chat,2);
    }
}

