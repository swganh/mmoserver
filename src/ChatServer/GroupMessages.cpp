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

#include "ChatMessageLib.h"
#include "ChatOpcodes.h"
#include "Player.h"

#include "Common/atMacroString.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"

void ChatMessageLib::sendIsmInviteRequest(Player* sender, Player* target) const
{
    // lets tell the zone to pop the invite window
    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opIsmGroupInviteRequest);
    gMessageFactory->addUint32(sender->getClient()->getAccountId());  // the sender's id
    gMessageFactory->addUint32(target->getClient()->getAccountId()); // the reciever account id
    gMessageFactory->addUint64(sender->getGroupId()); // the group id
    newMessage = gMessageFactory->EndMessage();

    target->getClient()->SendChannelA(newMessage,target->getClient()->getAccountId(),static_cast<uint8>(target->getPlanetId())+8, 3);
}

//======================================================================================================================

void ChatMessageLib::sendIsmGroupLootModeResponse(Player* target) const
{
    // lets tell the zone to pop the invite window
    Message* newMessage;
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opIsmGroupLootModeResponse);
    gMessageFactory->addUint32(target->getClient()->getAccountId());  // the sender's id
    newMessage = gMessageFactory->EndMessage();
    target->getClient()->SendChannelA(newMessage,target->getClient()->getAccountId(),static_cast<uint8>(target->getPlanetId())+8, 3);
}

//======================================================================================================================

void ChatMessageLib::sendIsmGroupLootMasterResponse(Player* target) const
{
    // lets tell the zone to pop the invite window
    Message* newMessage;
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opIsmGroupLootMasterResponse);
    gMessageFactory->addUint32(target->getClient()->getAccountId());  // the sender's id
    newMessage = gMessageFactory->EndMessage();
    target->getClient()->SendChannelA(newMessage,target->getClient()->getAccountId(),static_cast<uint8>(target->getPlanetId())+8, 3);
}

void ChatMessageLib::sendIsmIsGroupLeaderResponse(Player* player, uint64 requestId, bool isLeader)
{
    gMessageFactory->StartMessage();

    gMessageFactory->addUint32(opIsmIsGroupLeaderResponse);

    gMessageFactory->addUint64(requestId);
    gMessageFactory->addUint64(player->getCharId()); //Player Id

    if(isLeader)
        gMessageFactory->addUint8(1);
    else
        gMessageFactory->addUint8(0);

    Message* newMessage = gMessageFactory->EndMessage();

    player->getClient()->SendChannelA(newMessage, player->getClient()->getAccountId(), static_cast<uint8>(player->getPlanetId())+8, 3);
}

//======================================================================================================================

void ChatMessageLib::sendIsmGroupCREO6deltaGroupId(uint64 groupId, Player* target) const
{
    if(!target)
        return;
    // lets tell the zone to advise in-range players
    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opIsmGroupCREO6deltaGroupId);
    gMessageFactory->addUint32(target->getClient()->getAccountId());  // the sender's id
    gMessageFactory->addUint64(groupId); // the new group id
    newMessage = gMessageFactory->EndMessage();

    target->getClient()->SendChannelA(newMessage,target->getClient()->getAccountId(),static_cast<uint8>(target->getPlanetId())+8, 3);
}

//======================================================================================================================

void ChatMessageLib::sendIsmInviteInRangeRequest(Player* sender, Player* target)
{
    Message* newMessage;
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opIsmGroupInviteInRangeRequest);
    gMessageFactory->addUint32(sender->getClient()->getAccountId());
    gMessageFactory->addUint32(target->getClient()->getAccountId());
    newMessage = gMessageFactory->EndMessage();

    sender->getClient()->SendChannelA(newMessage, sender->getClient()->getAccountId(), static_cast<uint8>(sender->getPlanetId())+8, 3);
}

//======================================================================================================================