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
#include "GroupObject.h"
#include "Mail.h"
#include "Player.h"


#ifdef ERROR
#undef ERROR
#endif

#include "Utils/logger.h"

#include "Common/atMacroString.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"

#include <cmath>
#include <cassert>

void ChatMessageLib::sendChatPersistantMessagetoClient(DispatchClient* client,Mail* mail,uint32 mailId,uint32 mailCounter,uint8 status) const
{

    Message* newMessage;
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatPersistentMessageToClient);
    gMessageFactory->addString(mail->mSender);
    gMessageFactory->addString(gChatManager->mMainCategory);
    gMessageFactory->addString(gChatManager->mGalaxyName);
    gMessageFactory->addUint32(mailId);
    gMessageFactory->addUint32(mailCounter);
    gMessageFactory->addUint8(0);
    gMessageFactory->addString(mail->mSubject);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint8(status);
    gMessageFactory->addUint32(mail->mTime);
    newMessage = gMessageFactory->EndMessage();

    client->SendChannelA(newMessage,client->getAccountId(),CR_Client,2);
}

//======================================================================================================================

void ChatMessageLib::sendChatPersistantMessagetoClient(DispatchClient* client,Mail* mail) const
{
    Message* newMessage;
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatPersistentMessageToClient);
    gMessageFactory->addString(mail->mSender);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(mail->mId);
    gMessageFactory->addUint8(0);
    gMessageFactory->addString(mail->mText);
    gMessageFactory->addString(mail->mSubject);
    gMessageFactory->addString(mail->mAttachments);
    gMessageFactory->addUint8(MailStatus_Read);
    gMessageFactory->addUint32(mail->mTime);
    newMessage = gMessageFactory->EndMessage();

    client->SendChannelA(newMessage,client->getAccountId(),CR_Client,2);
}

//=======================================================================================================================

void ChatMessageLib::sendChatonPersistantMessage(DispatchClient* client, uint64 mailCounter) const
{
    Message* newMessage;
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnSendPersistentMessage);
    gMessageFactory->addUint32(4);
    gMessageFactory->addUint32(static_cast<uint32>(mailCounter));
    newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), CR_Chat, 6);
}

//======================================================================================================================

void ChatMessageLib::sendCancelAuctionMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128]) const
{

    if(!client)
        client = mClient;

    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("auction","seller_fail");
    aMS->addTO(BString(ItemName));
    aMS->addTextModule();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSendSystemMailMessage);
    gMessageFactory->addUint64(Sender);
    gMessageFactory->addUint64(Receiver);
    gMessageFactory->addString(BString("auctioner"));
    gMessageFactory->addString(BString("@auction:subject_auction_unsuccessful"));
    gMessageFactory->addUint32(0);//Unicode msg text in this case empty

    gMessageFactory->addString(aMS->assemble());

    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), CR_Chat, 6);

    delete(aMS);
}

//=======================================================================================================================

void ChatMessageLib::sendBidderCancelAuctionMail(DispatchClient* client, uint64 mSender, uint64 mReceiver, int8 mItemName[128]) const
{

    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("auction","buyer_canceled");
    aMS->addTO(BString(mItemName));
    aMS->addTextModule();


    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSendSystemMailMessage);
    gMessageFactory->addUint64(mSender);
    gMessageFactory->addUint64(mReceiver);
    gMessageFactory->addString(BString("auctioner"));
    gMessageFactory->addString(BString("@auction:subject_auction_cancelled"));
    gMessageFactory->addUint32(0);//Msg text in this case empty
    gMessageFactory->addString(aMS->assemble());

    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), CR_Chat, 6);

    delete(aMS);

}

//=======================================================================================================================

void ChatMessageLib::SendRetrieveAuctionItemResponseMessage(DispatchClient* client, uint64 mItemId, uint32 error) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opRetrieveAuctionItemResponseMessage);
    gMessageFactory->addUint64(mItemId);
    gMessageFactory->addUint32(error);
    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(),  CR_Client, 6);
}

//=======================================================================================================================

void ChatMessageLib::sendSoldInstantMail(DispatchClient* client,uint64 Sender, uint64 Receiver, int8 ItemName[128], int8 pBuyerName[128], int8 pRegion[128], uint32 pCredits) const
{
    //seller_success       Your auction of %TO has been sold to %TT for %DI credits

    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("auction","seller_success");
    aMS->addTO(BString(ItemName));
    aMS->addTT(BString(pBuyerName));
    aMS->addDI(pCredits);
    aMS->addTextModule();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSendSystemMailMessage);
    gMessageFactory->addUint64(Sender);
    gMessageFactory->addUint64(Receiver);
    gMessageFactory->addString(BString("auctioner"));
    gMessageFactory->addString(BString("@auction:subject_instant_seller"));
    gMessageFactory->addUint32(0);
    gMessageFactory->addString(aMS->assemble());
    delete(aMS);


    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), CR_Chat, 6);

}

//=======================================================================================================================

void ChatMessageLib::ItemExpiredMail(DispatchClient* client, uint64 Receiver, int8 ItemName[128]) const
{
    //seller_success       Your auction of %TO has been sold to %TT for %DI credits

    if(!client)
        client = mClient;

    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("auction","item_expired");
    aMS->addTO(BString(ItemName));
    aMS->addTextModule();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSendSystemMailMessage);
    gMessageFactory->addUint64(Receiver);//sender Id in this case auctioner
    gMessageFactory->addUint64(Receiver);
    gMessageFactory->addString(BString("auctioner"));
    gMessageFactory->addString(BString("@auction:subject_item_expired"));
    gMessageFactory->addUint32(0);
    gMessageFactory->addString(aMS->assemble());
    delete(aMS);


    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), CR_Chat, 6);

}

//=======================================================================================================================

void ChatMessageLib::sendSoldAuctionMail(DispatchClient* client,uint64 Sender, uint64 Receiver, int8 ItemName[128], int8 BuyerName[128], int8 Region[128], uint32 Credits) const
{
    //seller_success       Your auction of %TO has been sold to %TT for %DI credits

    if(!client)
        client = mClient;


    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("auction","seller_success");
    aMS->addTO(BString(ItemName));
    aMS->addTT(BString(BuyerName));
    aMS->addTextModule();

    aMS->addMBstf("auction","seller_success_location");
    aMS->addTT(Region);
    aMS->addTO("");
    aMS->addTextModule();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSendSystemMailMessage);
    gMessageFactory->addUint64(Sender);
    gMessageFactory->addUint64(Receiver);
    gMessageFactory->addString(BString("auctioner"));
    gMessageFactory->addString(BString("@auction:subject_instant_seller"));
    gMessageFactory->addUint32(0);
    gMessageFactory->addString(aMS->assemble());
    delete(aMS);


    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), CR_Chat, 6);

}

//=======================================================================================================================

void ChatMessageLib::sendAuctionOutbidMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128]) const
{
    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("auction","bidder_outbid");
    aMS->addTO(BString(ItemName));
    aMS->addTextModule();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSendSystemMailMessage);
    gMessageFactory->addUint64(Sender);
    gMessageFactory->addUint64(Receiver);
    gMessageFactory->addString(BString("auctioner"));
    gMessageFactory->addString(BString("@auction:subject_auction_outbid"));
    gMessageFactory->addUint32(0);
    gMessageFactory->addString(aMS->assemble());
    delete(aMS);

    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), CR_Chat, 6);

}

//=======================================================================================================================

void ChatMessageLib::sendAuctionWonMail(DispatchClient* client, uint64 Sender, uint64 Receiver,int8 ItemName[128],int8 Sellerame[32],uint32 Credits) const
{

    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("auction","buyer_success");
    aMS->addTO(BString(ItemName));
    aMS->addTT(BString(Sellerame));
    aMS->addDI(Credits);
    aMS->addTextModule();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSendSystemMailMessage);
    gMessageFactory->addUint64(Sender);
    gMessageFactory->addUint64(Receiver);
    gMessageFactory->addString(BString("auctioner"));
    gMessageFactory->addString(BString("@auction:subject_auction_buyer"));
    gMessageFactory->addUint32(0);
    gMessageFactory->addString(aMS->assemble());
    delete(aMS);


    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), CR_Chat, 6);

}

//======================================================================================================================

void ChatMessageLib::sendSystemMessageProper(Player* playerObject,uint8 system, BString customMessage,BString mainFile,BString mainVar,BString toFile,BString toVar,BString toCustom,int32 di,BString ttFile,BString ttVar,BString ttCustom,uint64 ttId,uint64 toId,uint64 tuId,BString tuFile,BString tuVar,BString tuCustom) const
{

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatSystemMessage);
    if(system)
        system = 2;
    gMessageFactory->addUint8(system);

    if(customMessage.getType() != BSTRType_Unicode16)
        customMessage.convert(BSTRType_Unicode16);

    if(toCustom.getType() != BSTRType_Unicode16)
        toCustom.convert(BSTRType_Unicode16);

    if(tuCustom.getType() != BSTRType_Unicode16)
        tuCustom.convert(BSTRType_Unicode16);

    if(ttCustom.getType() != BSTRType_Unicode16)
        ttCustom.convert(BSTRType_Unicode16);

    // simple message
    if(customMessage.getLength())
    {
        gMessageFactory->addString(customMessage.getUnicode16());
        gMessageFactory->addUint32(0);
    }
    // templated message
    else
    {
        gMessageFactory->addUint32(0);

        uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength()+ tuFile.getLength() + tuVar.getLength();

        gMessageFactory->addUint32(42 + ((uint32)ceil(((double)realSize) / 2.0)) + toCustom.getLength() + ttCustom.getLength()+ tuCustom.getLength());

        if(realSize % 2)
            gMessageFactory->addUint16(1);
        else
            gMessageFactory->addUint16(0);

        gMessageFactory->addUint8(1);
        gMessageFactory->addUint32(0xFFFFFFFF);
        gMessageFactory->addString(mainFile);
        gMessageFactory->addUint32(0);
        gMessageFactory->addString(mainVar);

        gMessageFactory->addUint64(tuId);
        gMessageFactory->addString(tuFile);
        gMessageFactory->addUint32(0);
        gMessageFactory->addString(tuVar);
        gMessageFactory->addString(tuCustom.getUnicode16());

        gMessageFactory->addUint64(ttId);
        gMessageFactory->addString(ttFile);
        gMessageFactory->addUint32(0);
        gMessageFactory->addString(ttVar);
        gMessageFactory->addString(ttCustom.getUnicode16());

        gMessageFactory->addUint64(toId);
        gMessageFactory->addString(toFile);
        gMessageFactory->addUint32(0);
        gMessageFactory->addString(toVar);
        gMessageFactory->addString(toCustom.getUnicode16());

        gMessageFactory->addInt32(di);
        gMessageFactory->addUint32(0);
        gMessageFactory->addUint8(0);

        if(realSize % 2)
            gMessageFactory->addUint8(0);
    }

    (playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getClient()->getAccountId(), CR_Client, 5);
}

//======================================================================================================================

void ChatMessageLib::sendSystemMessage(Player* target, BString message) const
{
    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatSystemMessage);
    gMessageFactory->addUint8(0);
    gMessageFactory->addString(message);
    gMessageFactory->addUint32(0);
    newMessage = gMessageFactory->EndMessage();

    target->getClient()->SendChannelA(newMessage, target->getClient()->getAccountId(), CR_Client, 5);
}

//======================================================================================================================

void ChatMessageLib::sendGroupSystemMessage(BString message, GroupObject* group, bool ignoreLeader) const
{
    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatSystemMessage);
    gMessageFactory->addUint8(0);
    gMessageFactory->addString(message);
    gMessageFactory->addUint32(0);
    newMessage = gMessageFactory->EndMessage();
    group->broadcastMessage(newMessage, ignoreLeader);

}

//======================================================================================================================

void ChatMessageLib::sendGroupSystemMessage(BString name, BString pointer, Player* target, GroupObject* group, bool unicode) const
{
    name.convert(BSTRType_Unicode16);
    uint32	pointerLength = (uint32)ceil((double)(pointer.getLength() / 2));

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatSystemMessage);
    gMessageFactory->addUint8(0);
    gMessageFactory->addUint32(0);

    gMessageFactory->addUint32(45 + pointerLength + name.getLength());
    gMessageFactory->addUint16(1);
    gMessageFactory->addUint8(1);
    gMessageFactory->addUint32(0xffffffff);
    gMessageFactory->addString(BString("group"));
    gMessageFactory->addUint32(0);
    gMessageFactory->addString(pointer);
    gMessageFactory->addUint64(0);
    gMessageFactory->addUint64(0);

    if(unicode)
    {
        gMessageFactory->addString(name);
        gMessageFactory->addUint32(0);
        gMessageFactory->addUint64(0);
        gMessageFactory->addUint32(0);
        gMessageFactory->addUint32(0);
    }
    else
    {
        gMessageFactory->addUint32(0);
        gMessageFactory->addUint32(0);
        gMessageFactory->addUint64(0);
        gMessageFactory->addUint32(0);
        gMessageFactory->addString(name);
    }

    gMessageFactory->addUint64(0);
    gMessageFactory->addUint64(0);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint16(0);

    newMessage = gMessageFactory->EndMessage();

    if(group == NULL)
    {
        target->getClient()->SendChannelA(newMessage, target->getClient()->getAccountId(), CR_Client, 5);
    }
    else
    {
        group->broadcastMessage(newMessage);
    }
}

//======================================================================================================================

void ChatMessageLib::sendFriendOnlineStatus(Player* player,Player* playerFriend,uint8 status, BString category, BString name) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatFriendlistUpdate);
    gMessageFactory->addString(category);
    gMessageFactory->addString(name);
    gMessageFactory->addString(playerFriend->getName());
    gMessageFactory->addUint8(status);

    Message* newMessage = gMessageFactory->EndMessage();

    player->getClient()->SendChannelA(newMessage,player->getClient()->getAccountId(),CR_Client,3);
}

//======================================================================================================================

void ChatMessageLib::sendBanktipMail(DispatchClient* client, Player*  playerObject, BString receiverName,uint64 receiverId, uint32 amount) const
{

    if(!client)
        client = mClient;

    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("base_player","prose_wire_mail_target");
    aMS->addDI(amount);
    aMS->addTO(playerObject->getName());
    aMS->addTextModule();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSendSystemMailMessage);
    gMessageFactory->addUint64(playerObject->getCharId());
    gMessageFactory->addUint64(receiverId);
    gMessageFactory->addString(playerObject->getName());
    gMessageFactory->addString(BString("@base_player:wire_mail_subject"));
    gMessageFactory->addUint32(0);//Unicode msg text in this case empty

    gMessageFactory->addString(aMS->assemble());

    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), CR_Chat, 6);

    delete(aMS);

    aMS = new atMacroString();

    aMS->addMBstf("base_player","prose_wire_mail_self");
    aMS->addDI(amount);
    aMS->addTO(receiverName);
    aMS->addTextModule();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSendSystemMailMessage);
    gMessageFactory->addUint64(playerObject->getCharId());
    gMessageFactory->addUint64(playerObject->getCharId());
    gMessageFactory->addString(receiverName);
    gMessageFactory->addString(BString("@base_player:wire_mail_subject"));
    gMessageFactory->addUint32(0);//Unicode msg text in this case empty

    gMessageFactory->addString(aMS->assemble());

    newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), CR_Chat, 6);

    delete(aMS);
}

//======================================================================================================================

void ChatMessageLib::sendChatRoomList(DispatchClient* client, ChannelList* list) const
{
    ChannelList::iterator iter = list->begin();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatRoomlist);

    gMessageFactory->addUint32(list->size());
    while (iter != list->end())
    {
        gMessageFactory->addUint32((*iter)->getId());
        gMessageFactory->addUint32((uint32)(*iter)->isPrivate());
        gMessageFactory->addUint8((*iter)->isModerated());

        BString roomPathName = "SWG.";
        roomPathName << (*iter)->getGalaxy().getAnsi() << ".";
        roomPathName << (*iter)->getName().getAnsi();
        gMessageFactory->addString(roomPathName);

        gMessageFactory->addString(SWG);
        gMessageFactory->addString((*iter)->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
        BString* name = (ChatManager::getSingletonPtr()->getFirstName((BString &)((*iter)->getOwner()->getLoweredName())));
        if (name->getLength() == 0)
        {
            gMessageFactory->addString((*iter)->getOwner()->getLoweredName());
        }
        else
        {
            gMessageFactory->addString(*name);
        }
#else
        gMessageFactory->addString((*iter)->getOwner()->getLoweredName());
#endif

        gMessageFactory->addString(SWG);
        gMessageFactory->addString((*iter)->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
        name = (ChatManager::getSingletonPtr()->getFirstName((BString &)((*iter)->getCreator()->getLoweredName())));
        if (name->getLength() == 0)
        {
            gMessageFactory->addString((*iter)->getCreator()->getLoweredName());
        }
        else
        {
            gMessageFactory->addString(*name);
        }
#else
        gMessageFactory->addString((*iter)->getCreator()->getLoweredName());
#endif
        gMessageFactory->addString((*iter)->getTitle());

        gMessageFactory->addUint32(0);
        gMessageFactory->addUint32(0);
        ++iter;
    }
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 2);
}

//======================================================================================================================

void ChatMessageLib::sendChatOnEnteredRoom(DispatchClient* client, ChatAvatarId* player, Channel* channel, uint32 requestId) const
{
    ChatAvatarIdList::iterator iter = channel->getUserList()->begin();

    while (iter != channel->getUserList()->end())
    {
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opChatOnEnteredRoom);

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
        gMessageFactory->addString(player->getPlayer()->getName());
#else
        gMessageFactory->addString(player->getLoweredName());
#endif
        gMessageFactory->addUint32(0); //Errorcode
        gMessageFactory->addUint32(channel->getId());
        gMessageFactory->addUint32(((*iter) == player) ? requestId : 0);
        Message* message = gMessageFactory->EndMessage();

        (*iter)->getPlayer()->getClient()->SendChannelA(message, (*iter)->getPlayer()->getClient()->getAccountId(), CR_Client, 5);
        ++iter;
    }
}

//======================================================================================================================

void ChatMessageLib::sendChatFailedToEnterRoom(DispatchClient* client, ChatAvatarId* player, uint32 errorcode, Channel* channel, uint32 requestId) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnEnteredRoom);

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
    gMessageFactory->addString(player->getPlayer()->getName());
#else
    gMessageFactory->addString(player->getLoweredName());
#endif
    gMessageFactory->addUint32(errorcode); //Errorcode
    gMessageFactory->addUint32(channel->getId());
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 5);
}

//======================================================================================================================

void ChatMessageLib::sendChatOnCreateRoom(DispatchClient* client, Channel* channel, uint32 requestId) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnCreateRoom);

    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(channel->getId());
    gMessageFactory->addUint32((uint32)channel->isPrivate());
    gMessageFactory->addUint8(channel->isModerated());
    gMessageFactory->addString(channel->getFullPath());

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
    BString* name = (ChatManager::getSingletonPtr()->getFirstName((BString &)(channel->getOwner()->getLoweredName())));
    if (name->getLength() == 0)
    {
        gMessageFactory->addString(channel->getOwner()->getLoweredName());
    }
    else
    {
        gMessageFactory->addString(*name);
    }
#else
    gMessageFactory->addString(channel->getOwner()->getLoweredName());
#endif
    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
    name = (ChatManager::getSingletonPtr()->getFirstName((BString &)(channel->getCreator()->getLoweredName())));
    if (name->getLength() == 0)
    {
        gMessageFactory->addString(channel->getCreator()->getLoweredName());
    }
    else
    {
        gMessageFactory->addString(*name);
    }
#else
    gMessageFactory->addString(channel->getCreator()->getLoweredName());
#endif
    gMessageFactory->addString(channel->getTitle());
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 4);
}

//======================================================================================================================

void ChatMessageLib::sendChatOnDestroyRoom(DispatchClient* client, Channel* channel, uint32 requestId) const
{
    // For some odd reason we managed to come here from group::disband with a NULL-channel.
    if (channel)
    {
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opChatOnDestroyRoom);

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(channel->getOwner()->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
        BString* name = (ChatManager::getSingletonPtr()->getFirstName((BString &)(channel->getOwner()->getLoweredName())));
        if (name->getLength() == 0)
        {
            gMessageFactory->addString(channel->getOwner()->getLoweredName());
        }
        else
        {
            gMessageFactory->addString(*name);
        }
#else
        gMessageFactory->addString(channel->getOwner()->getLoweredName());
#endif
        gMessageFactory->addUint32(0); //Errorcode
        gMessageFactory->addUint32(channel->getId());
        gMessageFactory->addUint32(requestId);
        Message* message = gMessageFactory->EndMessage();

        client->SendChannelA(message, client->getAccountId(), CR_Client, 4);
    }
    else
    {
        // For debugging purpose
        LOG(WARNING) << "ChatMessageLib::sendChatOnDestroyRoom: ERROR: channel is NULL";
    }
}

//======================================================================================================================

void ChatMessageLib::sendChatOnLeaveRoom(DispatchClient* client, ChatAvatarId* avatar, Channel* channel, uint32 requestId, uint32 errorCode) const
{
    ChatAvatarIdList::iterator iter = channel->getUserList()->begin();

    while (iter != channel->getUserList()->end())
    {
        assert (avatar != NULL);
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opChatOnLeaveRoom);

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(avatar->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
        gMessageFactory->addString(avatar->getPlayer()->getName());
#else
        gMessageFactory->addString(avatar->getLoweredName());
#endif
        gMessageFactory->addUint32(errorCode);
        gMessageFactory->addUint32(channel->getId());
        gMessageFactory->addUint32(((*iter) == avatar) ? requestId : 0);
        Message* message = gMessageFactory->EndMessage();
        (*iter)->getPlayer()->getClient()->SendChannelA(message, (*iter)->getPlayer()->getClient()->getAccountId(), CR_Client, 5);
        ++iter;
    }
}

//======================================================================================================================

void ChatMessageLib::sendChatOnInviteToRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const
{
    // We should not send this info to everyone in the channel, escpecially since we can invite non-online players
    // Other users in channel will get "bla bla... %TO bla bla ...UNKNOWN"-spam.
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnInviteRoom);

    gMessageFactory->addString(channel->getFullPath());

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(galaxy);
    gMessageFactory->addString(sender);
    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getGalaxy());
    gMessageFactory->addString(target);

    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 6);

}

//======================================================================================================================

void ChatMessageLib::sendChatFailedToInvite(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnInviteRoom);

    gMessageFactory->addString(channel->getFullPath());

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(galaxy);
#ifdef DISP_REAL_FIRST_NAME
#else
    sender.toLower();
#endif
    gMessageFactory->addString(sender);

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
#else
    target.toLower();
#endif
    gMessageFactory->addString(target);

    gMessageFactory->addUint32(errorcode);
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 6);
}

//======================================================================================================================

void ChatMessageLib::sendChatOnUninviteFromRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const
{
    // We should not send this info to everyone in the channel, escpecially since we can invite non-online players
    // Other users in channel will get "bla bla... %TO bla bla ...UNKNOWN"-spam.

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnUninviteFromRoom);

    gMessageFactory->addString(channel->getFullPath());

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(galaxy);
    gMessageFactory->addString(sender);

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getGalaxy());
    gMessageFactory->addString(target);

    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 6);
}

//======================================================================================================================

void ChatMessageLib::sendChatFailedToUninviteFromRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnUninviteFromRoom);

    gMessageFactory->addString(channel->getFullPath());

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(galaxy);
#ifdef DISP_REAL_FIRST_NAME
#else
    sender.toLower();
#endif
    gMessageFactory->addString(sender);

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
#else
    target.toLower();
#endif
    gMessageFactory->addString(target);

    gMessageFactory->addUint32(errorcode);
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 6);
}

//======================================================================================================================

void ChatMessageLib::sendChatOnBanAvatarFromRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const
{
    ChatAvatarIdList::iterator iter = channel->getUserList()->begin();

#ifdef DISP_REAL_FIRST_NAME
#else
    sender.toLower();
    target.toLower();
#endif

    while (iter != channel->getUserList()->end())
    {
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opChatOnBanAvatarFromRoom);

        gMessageFactory->addString(channel->getFullPath());

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(galaxy);
        gMessageFactory->addString(sender);

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(channel->getGalaxy());
        gMessageFactory->addString(target);

        gMessageFactory->addUint32(0);
        gMessageFactory->addUint32(((*iter)->getPlayer()->getClient() == client) ? requestId : 0);
        Message* message = gMessageFactory->EndMessage();

        (*iter)->getPlayer()->getClient()->SendChannelA(message, (*iter)->getPlayer()->getClient()->getAccountId(), CR_Client, 6);
        ++iter;
    }
}

//======================================================================================================================

void ChatMessageLib::sendChatFailedToBan(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnBanAvatarFromRoom);

    gMessageFactory->addString(channel->getFullPath());

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(galaxy);
#ifdef DISP_REAL_FIRST_NAME
#else
    sender.toLower();
#endif
    gMessageFactory->addString(sender);

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
#else
    target.toLower();
#endif
    gMessageFactory->addString(target);

    gMessageFactory->addUint32(errorcode);
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 6);
}

//======================================================================================================================

void ChatMessageLib::sendChatOnUnBanAvatarFromRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const
{
    ChatAvatarIdList::iterator iter = channel->getUserList()->begin();

#ifdef DISP_REAL_FIRST_NAME
#else
    sender.toLower();
    target.toLower();
#endif

    while (iter != channel->getUserList()->end())
    {
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opChatOnUnbanAvatarFromRoom);

        gMessageFactory->addString(channel->getFullPath());

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(galaxy);
        gMessageFactory->addString(sender);

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(channel->getGalaxy());
        gMessageFactory->addString(target);

        gMessageFactory->addUint32(0);
        gMessageFactory->addUint32(((*iter)->getPlayer()->getClient() == client) ? requestId : 0);
        Message* message = gMessageFactory->EndMessage();

        (*iter)->getPlayer()->getClient()->SendChannelA(message, (*iter)->getPlayer()->getClient()->getAccountId(), CR_Client, 6);
        ++iter;
    }
}

//======================================================================================================================

void ChatMessageLib::sendChatFailedToUnban(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnUnbanAvatarFromRoom);

    gMessageFactory->addString(channel->getFullPath());

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(galaxy);
#ifdef DISP_REAL_FIRST_NAME
#else
    sender.toLower();
#endif
    gMessageFactory->addString(sender);

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
#else
    target.toLower();
#endif
    gMessageFactory->addString(target);

    gMessageFactory->addUint32(errorcode);
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 6);
}

//======================================================================================================================

void ChatMessageLib::sendChatQueryRoomResults(DispatchClient* client, Channel* channel, uint32 requestId) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatQueryRoomResults);

    ChatAvatarIdList::iterator iter = channel->getUserList()->begin();
    gMessageFactory->addUint32(channel->getUserList()->size());
    while (iter != channel->getUserList()->end())
    {
        gMessageFactory->addString(SWG);
        gMessageFactory->addString((*iter)->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
        gMessageFactory->addString((*iter)->getPlayer()->getName());
#else
        gMessageFactory->addString((*iter)->getLoweredName());
#endif
        ++iter;
    }
    NameByCrcMap::iterator seconditer = channel->getInvited()->begin();
    gMessageFactory->addUint32(channel->getInvited()->size());
    while (seconditer != channel->getInvited()->end())
    {
        gMessageFactory->addString(SWG);
        gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
        // gMessageFactory->addString(*(ChatManager::getSingletonPtr()->getFirstName((string &)(((*seconditer).second)))));
        BString test(*(*seconditer).second);
        gMessageFactory->addString(*(ChatManager::getSingletonPtr()->getFirstName(test)));
#else
        gMessageFactory->addString(((*seconditer).second));
#endif
        ++seconditer;
    }

    seconditer = channel->getModeratorList()->begin();
    gMessageFactory->addUint32(channel->getModeratorList()->size());
    while (seconditer != channel->getModeratorList()->end())
    {
        gMessageFactory->addString(SWG);
        gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
        BString test(*(*seconditer).second);
        gMessageFactory->addString(*(ChatManager::getSingletonPtr()->getFirstName(test)));
        //gMessageFactory->addString(*(ChatManager::getSingletonPtr()->getFirstName((string &)(*((*seconditer).second)))));
#else
        gMessageFactory->addString(((*seconditer).second));
#endif
        ++seconditer;
    }

    seconditer = channel->getBanned()->begin();
    gMessageFactory->addUint32(channel->getBanned()->size());
    while (seconditer != channel->getBanned()->end())
    {
        gMessageFactory->addString(SWG);
        gMessageFactory->addString(channel->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
        BString test(*(*seconditer).second);
        gMessageFactory->addString(*(ChatManager::getSingletonPtr()->getFirstName(test)));
        // gMessageFactory->addString(*(ChatManager::getSingletonPtr()->getFirstName((string &)(*((*seconditer).second)))));
#else
        gMessageFactory->addString(((*seconditer).second));
#endif
        ++seconditer;
    }

    gMessageFactory->addUint32(requestId);
    gMessageFactory->addUint32(channel->getId());
    gMessageFactory->addUint32((uint32)channel->isPrivate());
    gMessageFactory->addUint8(channel->isModerated());

    gMessageFactory->addString(channel->getFullPath());

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getOwner()->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
    BString* name = (ChatManager::getSingletonPtr()->getFirstName((BString &)(channel->getOwner()->getLoweredName())));
    if (name->getLength() == 0)
    {
        gMessageFactory->addString(channel->getOwner()->getLoweredName());
    }
    else
    {
        gMessageFactory->addString(*name);
    }
#else
    gMessageFactory->addString(channel->getOwner()->getLoweredName());
#endif
    gMessageFactory->addString(SWG);
    gMessageFactory->addString(channel->getCreator()->getGalaxy());
#ifdef DISP_REAL_FIRST_NAME
    name = (ChatManager::getSingletonPtr()->getFirstName((BString &)(channel->getCreator()->getLoweredName())));
    if (name->getLength() == 0)
    {
        gMessageFactory->addString(channel->getCreator()->getLoweredName());
    }
    else
    {
        gMessageFactory->addString(*name);
    }
#else
    gMessageFactory->addString(channel->getCreator()->getLoweredName());
#endif

    gMessageFactory->addString(channel->getTitle());
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(0);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 7);
}

//======================================================================================================================

void ChatMessageLib::sendChatOnRemoveModeratorFromRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const
{
    ChatAvatarIdList::iterator iter = channel->getUserList()->begin();

#ifdef DISP_REAL_FIRST_NAME
#else
    target.toLower();
    sender.toLower();
#endif

    while (iter != channel->getUserList()->end())
    {
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opChatOnRemoveModFromRoom);

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(galaxy);
        gMessageFactory->addString(target);

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(galaxy);
        gMessageFactory->addString(sender);

        gMessageFactory->addUint32(0);
        gMessageFactory->addString(channel->getFullPath());
        gMessageFactory->addUint32(((*iter)->getPlayer()->getClient() == client) ? requestId : 0);
        Message* message = gMessageFactory->EndMessage();

        (*iter)->getPlayer()->getClient()->SendChannelA(message, (*iter)->getPlayer()->getClient()->getAccountId(), CR_Client, 6);
        ++iter;
    }

}

//======================================================================================================================

void ChatMessageLib::sendChatFailedToRemoveMod(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnRemoveModFromRoom);

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(galaxy);
#ifdef DISP_REAL_FIRST_NAME
#else
    target.toLower();
#endif
    gMessageFactory->addString(target);

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(galaxy);
#ifdef DISP_REAL_FIRST_NAME
#else
    sender.toLower();
#endif
    gMessageFactory->addString(sender);

    gMessageFactory->addUint32(errorcode);
    gMessageFactory->addString(channel->getFullPath());
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 6);
}

//======================================================================================================================

void ChatMessageLib::sendChatOnAddModeratorToRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const
{
    ChatAvatarIdList::iterator iter = channel->getUserList()->begin();

#ifdef DISP_REAL_FIRST_NAME
#else
    target.toLower();
    sender.toLower();
#endif

    while (iter != channel->getUserList()->end())
    {
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opChatOnModerateRoom);

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(galaxy);
        gMessageFactory->addString(target);

        gMessageFactory->addString(SWG);
        gMessageFactory->addString(galaxy);
        gMessageFactory->addString(sender);

        gMessageFactory->addUint32(0);
        gMessageFactory->addString(channel->getFullPath());
        gMessageFactory->addUint32(((*iter)->getPlayer()->getClient() == client) ? requestId : 0);
        Message* message = gMessageFactory->EndMessage();

        (*iter)->getPlayer()->getClient()->SendChannelA(message, (*iter)->getPlayer()->getClient()->getAccountId(), CR_Client, 6);
        ++iter;
    }

}

//======================================================================================================================

void ChatMessageLib::sendChatFailedToAddMod(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnModerateRoom);

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(galaxy);
#ifdef DISP_REAL_FIRST_NAME
#else
    target.toLower();
#endif
    gMessageFactory->addString(target);

    gMessageFactory->addString(SWG);
    gMessageFactory->addString(galaxy);
#ifdef DISP_REAL_FIRST_NAME
#else
    sender.toLower();
#endif
    gMessageFactory->addString(sender);

    gMessageFactory->addUint32(errorcode);
    gMessageFactory->addString(channel->getFullPath());
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 6);
}

//======================================================================================================================

void ChatMessageLib::sendChatRoomMessage(Channel* channel, BString galaxy, BString sender, BString message) const
{
    ChatAvatarIdList::iterator iter = channel->getUserList()->begin();

#ifdef DISP_REAL_FIRST_NAME
#else
    sender.toLower();
#endif

    // Check ignore list.
    BString loweredName = sender;
    loweredName.toLower();
    uint32 loweredNameCrc = loweredName.getCrc();

    while (iter != channel->getUserList()->end())
    {
        // If sender present at recievers ignore list, don't send.
        if ((*iter)->getPlayer()->checkIgnore(loweredNameCrc))
        {
            // Ignore player
        }
        else
        {

            DispatchClient* client = (*iter)->getPlayer()->getClient();

            if (client == NULL)
            {
                LOG(WARNING) << "sendChatRoomMessage: Client not found for channel " <<  channel->getId();
            }
            else
            {
                gMessageFactory->StartMessage();
                gMessageFactory->addUint32(opChatRoomMessage);

                gMessageFactory->addString(SWG);
                gMessageFactory->addString(galaxy);
                gMessageFactory->addString(sender);

                gMessageFactory->addUint32(channel->getId());
                gMessageFactory->addString(message);
                gMessageFactory->addUint32(0);
                Message* response = gMessageFactory->EndMessage();
                client->SendChannelA(response, client->getAccountId(), CR_Client, 5);

            }
        }
        ++iter;
    }
}

//======================================================================================================================

void ChatMessageLib::sendChatOnSendRoomMessage(DispatchClient *client, uint32 errorcode,uint32 requestId) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opChatOnSendRoomMessage);
    gMessageFactory->addUint32(errorcode);
    gMessageFactory->addUint32(requestId);
    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, client->getAccountId(), CR_Client, 3);
}
