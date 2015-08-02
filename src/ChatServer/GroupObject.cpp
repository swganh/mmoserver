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

#include "Channel.h"
#include "ChatAvatarId.h"
#include "ChatManager.h"
#include "ChatMessageLib.h"
#include "ChatOpcodes.h"
#include "GroupManager.h"
#include "GroupObject.h"
#include "Player.h"




#include "Utils/logger.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"

#include "Utils/utils.h"

#include <cmath>

//======================================================================================================================

GroupObject::GroupObject(Player* leader, uint64 groupId)
{

    mChannel = NULL;
    mMemberCount = 1;
    mMembersUpdateCount = 0;
    mLootMode = 0;

    mMembers.reserve(20);
    leader->setGroupId(groupId);
    leader->setGroupMemberIndex(0);

    mId = groupId;
    mMasterLooter = leader->getCharId();

    mMembers.push_back(leader);

	DLOG(INFO) << "New group created by "<<leader->getName().getAnsi() << " with id " << groupId;
}


//======================================================================================================================

GroupObject::~GroupObject()
{
    if(mChannel != NULL)
    {
        gChatManager->unregisterChannel(mChannel);
        delete mChannel;
        DLOG(INFO) << "Group Channel destroyed.";
    }
    DLOG(INFO)<< "Group destroyed.";
}

//======================================================================================================================
// Generates a position index for each member
//
void GroupObject::resetIndexes()
{
    PlayerList::iterator listIt = mMembers.begin();
    uint16 index = 0;
    while(listIt != mMembers.end())
    {
        (*listIt)->setGroupMemberIndex(index);
        ++index;
        ++listIt;
    }
}

//======================================================================================================================

void GroupObject::addTempMember(Player* player)
{
    player->setGroupMemberIndex(0xFFFF); // <- flag for "pending" members
    player->setGroupId(mId);

    mMemberCount++;

    // debug
    /*
    BString		tmpValueStr = string(BSTRType_Unicode16,128);
    swprintf(tmpValueStr.getUnicode16(),L"[Debug] Temp member added. Free slots: %u   ",20 - group->mMemberCount);
    sendSystemMessage(group->getLeader(), tmpValueStr.getUnicode16());*/
}


//======================================================================================================================

void GroupObject::removeTempMember(Player* player)
{
    player->setGroupId(0);
    player->setGroupMemberIndex(0);

    mMemberCount--;

    /* debug
    BString		tmpValueStr = string(BSTRType_Unicode16,128);
    swprintf(tmpValueStr.getUnicode16(),L"[Debug] Temp member removed. Free slots: %u   ",20 - group->mMemberCount);
    sendSystemMessage(group->getLeader(), tmpValueStr.getUnicode16());*/

    if(mMemberCount <= 1)
    {
        getLeader()->setGroupId(0);
        getLeader()->setGroupMemberIndex(0); // just to be sure (should be 0 anyway)
        gGroupManager->removeGroup(mId);
    }
}

//======================================================================================================================

void GroupObject::addMember(Player* player)
{
    player->setGroupId(mId);
    mMembers.push_back(player);
}

//======================================================================================================================

void GroupObject::removeMember(Player* player)
{

    PlayerList::iterator listIt = mMembers.begin();
    while(listIt != mMembers.end())
    {
        // find the player to remove
        if((*listIt) == player)
        {
            // remove player from the group channel
            if(mChannel)
            {
                mChannel->removeUser(player);
                gChatMessageLib->sendChatOnDestroyRoom(player->getClient(), mChannel, 0);
            }

            player->setGroupId(0);

            // if he is the last member
            if(mMembers.size() <= 2)
            {
                disband();
                break;
            }

            // tell the zone to send the groupId delta
            // FIXME: player's network client is already dead!!
            gChatMessageLib->sendIsmGroupCREO6deltaGroupId(0, player);

            // remove from list
            mMembers.erase(listIt);

            broadcastDeltaRemove(player);

            // if player was the leader
            if(player->getGroupMemberIndex() == 0)
            {
                // TODO: delegate zone for handling inrange sys messages
                gChatMessageLib->sendGroupSystemMessage(getLeader()->getName(), BString("new_leader"), NULL, this, true);
            }
            else // if he wasnt leader,
            {
                // but was Master Looter
                if(player->getCharId() == mMasterLooter)
                {
                    // give MasterLootership to the new leader
                    this->changeMasterLooter(getLeader());
                }
            }

            // recalculate indexes
            resetIndexes();

            gChatMessageLib->sendSystemMessage(player,L"@group:removed");

            // minus minus
            mMemberCount--;

            // lets get out of this!
            break;
        }
        ++listIt;
    }
}

//======================================================================================================================

void GroupObject::disband()
{
    PlayerList::iterator listIt = mMembers.begin();
    Player* targetPlayer;
    while(listIt != mMembers.end())
    {
        targetPlayer = (Player*)(*listIt);
        gChatMessageLib->sendChatOnDestroyRoom(targetPlayer->getClient(), mChannel, 0);
        gChatMessageLib->sendIsmGroupCREO6deltaGroupId(0, targetPlayer);
        gChatMessageLib->sendSceneDestroyObject(mId, targetPlayer);
        gChatMessageLib->sendSystemMessage(targetPlayer, L"@group:disbanded");
        targetPlayer->setGroupId(0);

        ++listIt;
    }

    gGroupManager->removeGroup(mId);
}

//======================================================================================================================

void GroupObject::changeLeader(Player* player)
{
    mMasterLooter = player->getCharId();
    // we create a new list containing the new hierarchy
    PlayerList tempList;
    tempList.reserve(20);

    tempList.push_back(player); // new leader
    tempList.push_back(getLeader()); // old leader

    // iterate trough old list
    PlayerList::iterator listIt = mMembers.begin() + 1; // starting at position 1 ( 0 is the old leader)

    while(listIt != mMembers.end())
    {
        // if not the new leader (its in position of new list 1 already)
        if((*listIt) != player)
        {
            tempList.push_back((*listIt)); // add to the new list
        }
        ++listIt;
    }

    // do i need this ? =0
    mMembers.empty();

    // assign the new list
    mMembers = tempList;

    // recalculate member indexes
    resetIndexes();

    // send the delta to everyone
    broadcastDeltaResetAll();

    // send the sys message to everyone
    gChatMessageLib->sendGroupSystemMessage(getLeader()->getName(), BString("new_leader"), NULL, this, true);
}

//======================================================================================================================

void GroupObject::changeLootMode(uint32 newLootMode)
{
    mLootMode = newLootMode;
    broadcastUpdateLootMode();
}

//======================================================================================================================

void GroupObject::changeMasterLooter(Player* newMasterLooter)
{
    mMasterLooter = newMasterLooter->getCharId();
    broadcastUpdateMasterLooter();
}

//======================================================================================================================

void GroupObject::sendCreate(Player* player)
{
    gChatMessageLib->sendSceneCreateObjectByCrc(mId, 0x788cf998, player);
    sendBaseline(player);
    gChatMessageLib->sendSceneEndBaselines(mId, player);
    gChatMessageLib->sendIsmGroupCREO6deltaGroupId(mId, player);
    gChatMessageLib->sendChatOnCreateRoom(player->getClient(), mChannel, 0);
    sendGroupLocationNotification(player);
    broadcastPlayerZoneNotification(player);
}

//======================================================================================================================

void GroupObject::sendUpdate(Player* player)	// Used when entering a new location (new scene)
{
    gChatMessageLib->sendSceneCreateObjectByCrc(mId, 0x788cf998, player);
    sendBaseline(player);
    gChatMessageLib->sendSceneEndBaselines(mId, player);
    gChatMessageLib->sendIsmGroupCREO6deltaGroupId(mId, player);
    sendGroupLocationNotification(player);
    broadcastPlayerZoneNotification(player);
}

//======================================================================================================================

void GroupObject::createChannel()
{
    int8 channelName[64];

    Channel* channel = new Channel();
    channel->setId(((uint32)(mId)) + 0xf0000000);
    channel->setCreator(gSystemAvatar);
    sprintf(channelName, "%" PRIu64 ".GroupChat", mId);
    channel->setName(BString(channelName));
    channel->setGalaxy(gChatManager->getGalaxyName());

    wchar_t temp[64];
    swprintf(temp,64,L"%"WidePRIu64, mId);
    channel->setTitle(temp);

	DLOG(INFO)  << "Group channel created: " << channel->getName().getAnsi() << " with id " << channel->getId();
    channel->setOwner(gSystemAvatar);
    mChannel = channel;
    gChatManager->registerChannel(mChannel);
}

//======================================================================================================================

void GroupObject::broadcastMessage(Message* message, bool ignoreLeader)
{

    PlayerList::iterator listIt = mMembers.begin();
    if(ignoreLeader) {
        listIt++;
    }

    while(listIt != mMembers.end())
    {
        gMessageFactory->StartMessage();
        gMessageFactory->addData(message->getData(),message->getSize());
        Message* messageCopy = gMessageFactory->EndMessage();
        (*listIt)->getClient()->SendChannelA(messageCopy, (*listIt)->getClient()->getAccountId(), CR_Client, 5);
        ++listIt;
    }
    gMessageFactory->DestroyMessage(message);
}

//======================================================================================================================
// Advise all the out-of-range members on the planet. That the position of this player has changed.
// Planet name is ignored here.
//
void GroupObject::broadcastPlayerPositionNotification(Player* player)
{
    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClientMfdStatusUpdateMessage);
    gMessageFactory->addUint16(0); // no need to put the planet into that one cause it will be sent in the same zone anyway
    gMessageFactory->addUint64(player->getCharId());

    gMessageFactory->addFloat(player->getPositionX());
    gMessageFactory->addFloat(0); // lets ignore Y anyway for 2d distances
    gMessageFactory->addFloat(player->getPositionZ());

    newMessage = gMessageFactory->EndMessage();

    PlayerList::iterator listIt = mMembers.begin();

    float squaredist;
    while(listIt != mMembers.end())
    {
        //only sending to out of range player (>256m) on the same planet
        // get the square dist

        squaredist =
            (player->getPositionX()-(*listIt)->getPositionX()) * (player->getPositionX()-(*listIt)->getPositionX())
            +
            (player->getPositionZ()-(*listIt)->getPositionZ()) * (player->getPositionZ()-(*listIt)->getPositionZ());


        // if target matches our needs
        if((*listIt) != player
                && (*listIt)->getPlanetId() == player->getPlanetId()
                && sqrt(squaredist) > 256)
        {
            // then advise it!
            gMessageFactory->StartMessage();
            gMessageFactory->addData(newMessage->getData(),newMessage->getSize());
            Message* messageCopy = gMessageFactory->EndMessage();
            (*listIt)->getClient()->SendChannelA(messageCopy, (*listIt)->getClient()->getAccountId(), CR_Client, 5);
        }
        ++listIt;
    }

    gMessageFactory->DestroyMessage(newMessage);

}

//======================================================================================================================
// Advise all the group members about the new location of this player.
// Used when player shuttles.
//
void GroupObject::broadcastPlayerZoneNotification(Player* player)
{
    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClientMfdStatusUpdateMessage);
    gMessageFactory->addString(BString(gChatManager->getPlanetNameById(player->getPlanetId())));
    gMessageFactory->addUint64(player->getCharId());

    gMessageFactory->addFloat(player->getPositionX());
    gMessageFactory->addFloat(0);
    gMessageFactory->addFloat(player->getPositionZ());

    newMessage = gMessageFactory->EndMessage();

    PlayerList::iterator listIt = mMembers.begin();

    while(listIt != mMembers.end())
    {
        // if target matches our needs
        if((*listIt) != player)
        {
            // advise it!
            gMessageFactory->StartMessage();
            gMessageFactory->addData(newMessage->getData(),newMessage->getSize());
            Message* messageCopy = gMessageFactory->EndMessage();
            (*listIt)->getClient()->SendChannelA(messageCopy, (*listIt)->getClient()->getAccountId(), CR_Client, 5);
        }
        ++listIt;
    }

    gMessageFactory->DestroyMessage(newMessage);
}

//======================================================================================================================
// Tells a player where all the other group members are.
//
void GroupObject::sendGroupLocationNotification(Player* player)
{

    PlayerList::iterator listIt = mMembers.begin();

    while(listIt != mMembers.end())
    {
        if((*listIt) != player)
        {
            gMessageFactory->StartMessage();
            gMessageFactory->addUint32(opClientMfdStatusUpdateMessage);
            gMessageFactory->addString(BString(gChatManager->getPlanetNameById((*listIt)->getPlanetId())));
            gMessageFactory->addUint64((*listIt)->getCharId());
            gMessageFactory->addFloat(player->getPositionX());
            gMessageFactory->addFloat(0);
            gMessageFactory->addFloat(player->getPositionZ());
            Message* message = gMessageFactory->EndMessage();
            player->getClient()->SendChannelA(message, player->getClient()->getAccountId(), CR_Client, 5);
        }
        ++listIt;
    }
}

//======================================================================================================================

void GroupObject::sendBaseline(Player* player)
{
    // first we create the baseline data
    Message* baselineBody;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint16(8);
    gMessageFactory->addUint32(63);
    gMessageFactory->addUint32(mMembers.size());
    gMessageFactory->addUint32(mMembersUpdateCount);

    //1. Player List
    PlayerList::iterator listIt = mMembers.begin();
    Player* runningPlayer;

    // adding every player id and name
    while(listIt != mMembers.end())
    {
        runningPlayer = (Player*)(*listIt);

        gMessageFactory->addUint64(runningPlayer->getCharId());
        gMessageFactory->addString(runningPlayer->getName());

        ++listIt;
    }

    //2. Player Missions
    gMessageFactory->addUint32(mMembers.size());
    gMessageFactory->addUint32(mMembersUpdateCount);

    listIt = mMembers.begin();

    while(listIt != mMembers.end())
    {
        runningPlayer = (Player*)(*listIt);

        gMessageFactory->addUint64(0);
        gMessageFactory->addUint32(0);

        ++listIt;
    }

    //3. Group Options
    gMessageFactory->addUint16(0);
    gMessageFactory->addUint16(9);
    gMessageFactory->addUint32(4);
    gMessageFactory->addUint64(mMasterLooter); // master looter
    gMessageFactory->addUint32(mLootMode); //loot rule

    baselineBody = gMessageFactory->EndMessage();

    // Now crafting the baseline header
    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(0x68A75F0C); // baseline
    gMessageFactory->addUint64(mId);
    gMessageFactory->addUint32(0x47525550); // GRUP
    gMessageFactory->addUint8(6);

    // adding the baselines body to it
    gMessageFactory->addUint32(baselineBody->getSize());
    gMessageFactory->addData(baselineBody ->getData(), baselineBody->getSize());

    newMessage = gMessageFactory->EndMessage();

    gMessageFactory->DestroyMessage(baselineBody);

    player->getClient()->SendChannelA(newMessage, player->getClient()->getAccountId(), CR_Client, 5);

}

//======================================================================================================================

void GroupObject::broadcastDeltaResetAll()
{
    mMembersUpdateCount++;
    Message* baselineBody;

    gMessageFactory->StartMessage();

    // two updates : reset all & set master looter to the new leaders'id

    gMessageFactory->addUint16(2);
    gMessageFactory->addUint16(1);
    gMessageFactory->addUint32(1);
    gMessageFactory->addUint32(mMembersUpdateCount);
    gMessageFactory->addUint8(3); //reset all
    gMessageFactory->addUint16((uint16)mMembers.size());

    PlayerList::iterator listIt;
    Player* runningPlayer;

    listIt = mMembers.begin();
    while(listIt != mMembers.end())
    {
        runningPlayer = (Player*)(*listIt);

        gMessageFactory->addUint64(runningPlayer->getCharId());
        gMessageFactory->addString(runningPlayer->getName());

        ++listIt;
    }


    // modify part 6 (master looter id)
    gMessageFactory->addUint16(6);
    gMessageFactory->addUint64(mMasterLooter);

    baselineBody = gMessageFactory->EndMessage();

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(0x12862153); // deltas
    gMessageFactory->addUint64(mId);
    gMessageFactory->addUint32(0x47525550); // GRUP
    gMessageFactory->addUint8(6);

    gMessageFactory->addUint32(baselineBody->getSize());
    gMessageFactory->addData(baselineBody ->getData(), baselineBody->getSize());

    newMessage = gMessageFactory->EndMessage();

    gMessageFactory->DestroyMessage(baselineBody);

    // update counter need to match the amount of modifications we did here
    mMembersUpdateCount +=  mMembers.size();

    broadcastMessage(newMessage);
}

//======================================================================================================================

void GroupObject::broadcastDeltaAdd(Player* player)
{
    mMembersUpdateCount++;

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(0x12862153); // deltas
    gMessageFactory->addUint64(mId);
    gMessageFactory->addUint32(0x47525550); // GRUP
    gMessageFactory->addUint8(6);

    gMessageFactory->addUint32(25 + (player->getName().getLength()));
    gMessageFactory->addUint16(1);
    gMessageFactory->addUint16(1);

    gMessageFactory->addUint32(1);
    gMessageFactory->addUint32(mMembersUpdateCount);

    gMessageFactory->addUint8(1);

    gMessageFactory->addUint16(player->getGroupMemberIndex());
    gMessageFactory->addUint64(player->getCharId());
    gMessageFactory->addString(player->getName());

    newMessage = gMessageFactory->EndMessage();

    broadcastMessage(newMessage);
}

//======================================================================================================================

void GroupObject::broadcastDeltaRemove(Player* player)
{
    mMembersUpdateCount++;

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(0x12862153); // deltas
    gMessageFactory->addUint64(mId);
    gMessageFactory->addUint32(0x47525550); // GRUP
    gMessageFactory->addUint8(6);

    gMessageFactory->addUint32(15);
    gMessageFactory->addUint16(1);
    gMessageFactory->addUint16(1);

    gMessageFactory->addUint32(1);
    gMessageFactory->addUint32(mMembersUpdateCount);

    gMessageFactory->addUint8(0);

    gMessageFactory->addUint16(player->getGroupMemberIndex());

    newMessage = gMessageFactory->EndMessage();

    broadcastMessage(newMessage);
}

//======================================================================================================================

void GroupObject::broadcastUpdateLootMode()
{
    //mMembersUpdateCount++;

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(0x12862153); // deltas
    gMessageFactory->addUint64(mId);
    gMessageFactory->addUint32(0x47525550); // GRUP
    gMessageFactory->addUint8(6);

    gMessageFactory->addUint32(8);
    gMessageFactory->addUint16(2);
    gMessageFactory->addUint16(7);

    gMessageFactory->addUint32(mLootMode);

    newMessage = gMessageFactory->EndMessage();

    broadcastMessage(newMessage);
}

//======================================================================================================================

void GroupObject::broadcastUpdateMasterLooter()
{
    //mMembersUpdateCount++;

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(0x12862153); // deltas
    gMessageFactory->addUint64(mId);
    gMessageFactory->addUint32(0x47525550); // GRUP
    gMessageFactory->addUint8(6);

    gMessageFactory->addUint32(12);
    gMessageFactory->addUint16(2);
    gMessageFactory->addUint16(6);

    gMessageFactory->addUint64(mMasterLooter);

    newMessage = gMessageFactory->EndMessage();

    broadcastMessage(newMessage);
}

//======================================================================================================================
