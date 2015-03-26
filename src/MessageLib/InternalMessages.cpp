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

#include "MessageLib.h"

#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"



#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"


//======================================================================================================================
//
// request zone transfer through travel ticket
//
bool MessageLib::sendClusterZoneTransferRequestByTicket(PlayerObject* playerObject,uint64 ticketId,uint32 destinationPlanet)
{
    if(!playerObject || !playerObject->isConnected())
    {
        return(false);
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opClusterZoneTransferRequestByTicket);
    mMessageFactory->addUint32(destinationPlanet);
    mMessageFactory->addUint64(ticketId);

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Connection, 0);

    return(true);
}

//======================================================================================================================
//
// request zone transfer through admin command
//
bool MessageLib::sendClusterZoneTransferRequestByPosition(PlayerObject* playerObject, const glm::vec3& position, uint32 destinationPlanet)
{
    if(!playerObject || !playerObject->isConnected())
    {
        return(false);
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opClusterZoneTransferRequestByPosition);
    mMessageFactory->addUint32(destinationPlanet);
    mMessageFactory->addFloat(position.x);
    mMessageFactory->addFloat(position.z);

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Connection,0);

    return(true);
}

//======================================================================================================================
//
// zone transfer through travel ticket
//
bool MessageLib::sendClusterZoneTransferCharacter(PlayerObject* playerObject, uint32 destinationPlanet)
{
    if(!playerObject || !playerObject->isConnected())
    {
        return(false);
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opClusterZoneTransferCharacter);
    mMessageFactory->addUint64(playerObject->getId());
    mMessageFactory->addUint32(destinationPlanet);

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Connection, 0);

    return(true);
}

//======================================================================================================================
//
// group baselines
//
bool MessageLib::sendIsmGroupBaselineRequest(PlayerObject* targetPlayer)
{
    if (!_checkPlayer(targetPlayer))
    {
        return(false);
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmGroupBaselineRequest);
    mMessageFactory->addFloat(targetPlayer->mPosition.x);
    mMessageFactory->addFloat(targetPlayer->mPosition.z);

    targetPlayer->getClient()->SendChannelA(mMessageFactory->EndMessage(), targetPlayer->getAccountId(), CR_Chat, 2);

    return(true);
}

//======================================================================================================================
//
// group leave
//
bool MessageLib::sendIsmGroupLeave(PlayerObject* player)
{
    if (!_checkPlayer(player))
    {
        return(false);
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmGroupLeave);

    player->getClient()->SendChannelA(mMessageFactory->EndMessage(),player->getAccountId(),CR_Chat,2);

    return(true);
}

//======================================================================================================================
//
// group position
//
bool MessageLib::sendIsmGroupPositionNotification(PlayerObject* targetPlayer)
{
    if (!_checkPlayer(targetPlayer))
    {
        return(false);
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmGroupPositionNotification);
    mMessageFactory->addFloat(targetPlayer->mPosition.x);
    mMessageFactory->addFloat(targetPlayer->mPosition.z);

    targetPlayer->getClient()->SendChannelA(mMessageFactory->EndMessage(),targetPlayer->getAccountId(), CR_Chat, 2);

    return(true);
}

//======================================================================================================================
//
// group invite inrange response
//
bool MessageLib::sendIsmGroupInviteInRangeResponse(PlayerObject *sender, PlayerObject *target, bool inRange)
{
    if(!_checkPlayer(sender) || !_checkPlayer(target))
    {
        return(false);
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmGroupInviteInRangeResponse);
    mMessageFactory->addUint32(sender->getAccountId());
    mMessageFactory->addUint32(target->getAccountId());
    mMessageFactory->addUint8(inRange);

    sender->getClient()->SendChannelA(mMessageFactory->EndMessage(), sender->getAccountId(), CR_Chat, 2);

    return(true);
}

//======================================================================================================================
//
// report location to chatserver
//
bool MessageLib::sendFindFriendLocation(PlayerObject* friendPlayer, uint64 friendId,uint64 player, float X, float Z)
{
    if(!friendPlayer || !friendPlayer->isConnected())
    {
        return(false);
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opFindFriendSendPosition);
    mMessageFactory->addUint64(friendId);
    mMessageFactory->addUint64(player);
    mMessageFactory->addFloat(X);
    mMessageFactory->addFloat(Z);

    (friendPlayer->getClient())->SendChannelA(mMessageFactory->EndMessage(), friendPlayer->getAccountId(), CR_Chat, 4);

    return(true);
}

//======================================================================================================================
//
// Dust off for Banktips
//
bool MessageLib::sendBankTipDustOff(PlayerObject* playerObject, uint64 tipRecipient, uint32 amount, BString recipientName)
{
    if(!playerObject || !playerObject->isConnected())
    {
        return(false);
    }

    //informs chatserver on succesful banktip

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBankTipDustOff);
    mMessageFactory->addUint64(tipRecipient);
    mMessageFactory->addUint64(playerObject->getId());
    mMessageFactory->addUint32(amount);
    mMessageFactory->addString(recipientName);

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Chat, 4);

    return(true);
}

//======================================================================================================================
//
// group loot mode update
//
bool MessageLib::sendGroupLootModeResponse(PlayerObject* playerObject,uint32 selection)
{
    if(!playerObject || !playerObject->isConnected())
    {
        return(false);
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmGroupLootModeResponse);
    mMessageFactory->addUint32(selection);

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(),CR_Chat,2);

    return(true);
}

//======================================================================================================================

bool MessageLib::sendGroupLootMasterResponse(PlayerObject* masterLooter, PlayerObject* playerObject)
{
    if(!masterLooter || !playerObject || !playerObject->isConnected())
    {
        return(false);
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmGroupLootMasterResponse);
    mMessageFactory->addUint32(masterLooter->getAccountId());

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Chat,2);

    return(true);
}

//======================================================================================================================

void MessageLib::sendTutorialServerStatusRequest(DispatchClient* client, uint64 playerId, uint32 accountID)
{
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opTutorialServerStatusRequest);
    mMessageFactory->addUint64(playerId);

    client->SendChannelA(mMessageFactory->EndMessage(), accountID, CR_Connection, 5);
}

void MessageLib::sendGroupLeaderRequest(PlayerObject* sender, uint64 requestId, uint32 operation, uint64 groupId)
{
    mMessageFactory->StartMessage();

    mMessageFactory->addUint32(opIsmIsGroupLeaderRequest);
    mMessageFactory->addUint64(requestId);
    mMessageFactory->addUint64(sender->getId());
    mMessageFactory->addUint64(groupId);
    mMessageFactory->addUint32(operation);

    sender->getClient()->SendChannelA(mMessageFactory->EndMessage(), sender->getAccountId(), CR_Chat, 3);

}
