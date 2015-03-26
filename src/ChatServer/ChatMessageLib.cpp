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
#include "CSROpcodes.h"
#include "GroupObject.h"
#include "Player.h"

#include "Common/atMacroString.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"

bool			ChatMessageLib::mInsFlag    = false;
ChatMessageLib*	ChatMessageLib::mSingleton  = NULL;

ChatMessageLib*	ChatMessageLib::Init(DispatchClient* client)
{
    if(!mInsFlag)
    {
        mSingleton = new ChatMessageLib(client);
        mInsFlag = true;

        return mSingleton;
    }
    else
        return mSingleton;
}
ChatMessageLib::ChatMessageLib(DispatchClient* client)
{
    mClient = client;
}

//======================================================================================================================

void ChatMessageLib::sendSceneCreateObjectByCrc(uint64 objectId, uint32 objectCrc, Player* player) const
{
    Message*		message;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSceneCreateObjectByCrc);
    gMessageFactory->addUint64(objectId);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(1);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(objectCrc);
    gMessageFactory->addUint8(0);
    message = gMessageFactory->EndMessage();
    player->getClient()->SendChannelA(message, player->getClient()->getAccountId(), CR_Client, 5);
}

//======================================================================================================================

void ChatMessageLib::sendSceneDestroyObject(uint64 objectId, Player* player) const
{
    Message*		message;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSceneDestroyObject);
    gMessageFactory->addUint64(objectId);
    gMessageFactory->addUint8(0);
    message = gMessageFactory->EndMessage();

    player->getClient()->SendChannelA(message, player->getClient()->getAccountId(), CR_Client, 5);
}
//======================================================================================================================

void ChatMessageLib::sendSceneEndBaselines(uint64 objectId, Player* player) const
{
    Message*		message;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSceneEndBaselines);
    gMessageFactory->addUint64(objectId);
    message = gMessageFactory->EndMessage();

    player->getClient()->SendChannelA(message, player->getClient()->getAccountId(), CR_Client, 5);
}
//======================================================================================================================



////////////////////////////////////////////////////////////////////////////////////////////
//sends the zone the necessary information to create a waypoint for a friend
////////////////////////////////////////////////////////////////////////////////////////////
void ChatMessageLib::sendFindFriendCreateWaypoint(DispatchClient* client, Player* player, Player* friendPlayer) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opFindFriendCreateWaypoint);
    gMessageFactory->addUint64(player->getCharId());
    gMessageFactory->addString(friendPlayer->getName());
    gMessageFactory->addUint32(friendPlayer->getPlanetId());
    gMessageFactory->addFloat(friendPlayer->getPositionX());
    gMessageFactory->addFloat(friendPlayer->getPositionZ());
    Message* newMessage = gMessageFactory->EndMessage();
    //planetID+8 is the zones ID
    client->SendChannelA(newMessage, client->getAccountId(),static_cast<uint8>(player->getPlanetId())+8, 6);

}

////////////////////////////////////////////////////////////////////////////////////////////
//requests the position of a char for a friend from the chatserver
////////////////////////////////////////////////////////////////////////////////////////////
void ChatMessageLib::sendFindFriendRequestPosition(DispatchClient* client, Player* playersFriend, Player* player) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opFindFriendRequestPosition);

    gMessageFactory->addUint64(playersFriend->getCharId());
    gMessageFactory->addUint64(player->getCharId());

    Message* newMessage = gMessageFactory->EndMessage();
    //planetID+8 is the zones ID
    client->SendChannelA(newMessage, client->getAccountId(), static_cast<uint8>(playersFriend->getPlanetId())+8, 6);

}

//======================================================================================================================

