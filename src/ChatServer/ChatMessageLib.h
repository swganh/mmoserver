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

#ifndef ANH_CHATSERVER_MESSAGELIB_H
#define ANH_CHATSERVER_MESSAGELIB_H

// This was enabled for test of beeing able to display and present all player names, as they where typed at creation,
// in dialogs, windows and popups that have to do with user created channels. And still have them saved with all lowercase in the database.
// #define DISP_REAL_FIRST_NAME 1

#include <vector>
#include "Utils/typedefs.h"
#include "Utils/bstring.h"

#define	 gChatMessageLib	ChatMessageLib::getSingletonPtr()
#define	 SWG				BString("SWG")

class Article;
class Category;
class Channel;
class ChatAvatarId;
class Comment;
class DispatchClient;
class GroupObject;
class Mail;
class Message;
class MessageDispatch;
class Player;
class Ticket;
class TradeManagerAsyncContainer;
class StructureManagerAsyncContainer;

struct HarvesterHopperItem;
struct AuctionItem;
struct Bazaar;
struct ItemDescriptionAttributes;

typedef std::vector<Article*>                   ArticleList;
typedef std::vector<ItemDescriptionAttributes*>	AttributesList;
typedef std::vector<Category*>                  CategoryList;
typedef std::vector<Channel*>                   ChannelList;
typedef std::vector<Comment*>                   CommentList;
typedef std::vector<HarvesterHopperItem*>		HopperResourceList;

class ChatMessageLib
{
public:
    static ChatMessageLib*	getSingletonPtr() {
        return mSingleton;
    }
    static ChatMessageLib*	Init(DispatchClient* client);

    explicit ChatMessageLib(DispatchClient* client);

    //Mail
    void sendChatonPersistantMessage(DispatchClient* client, uint64 mailCounter) const;
    void sendChatPersistantMessagetoClient(DispatchClient* client,Mail* mail, uint32 mailId,uint32 mailCounter,uint8 status) const;
    void sendChatPersistantMessagetoClient(DispatchClient* client,Mail* mail) const;
    void sendSystemMailMessage(DispatchClient* client,Mail* mail, uint64 recepient);

    //Bazaar
    void ItemExpiredMail(DispatchClient* client, uint64 Receiver, int8 ItemName[128]) const;
    void sendCancelAuctionMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128]) const;
    void sendSoldInstantMail(DispatchClient* client,uint64 Sender, uint64 Receiver, int8 ItemName[128], int8 BuyerName[128], int8 Region[128], uint32 Credits) const;
    void sendSoldAuctionMail(DispatchClient* client,uint64 Sender, uint64 Receiver, int8 ItemName[128], int8 BuyerName[128], int8 Region[128], uint32 Credits) const;
    void sendBidderCancelAuctionMail(DispatchClient* client, uint64 mSender, uint64 mReceiver, int8 mItemName[128]) const;
    void sendAuctionOutbidMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128]) const;
    void sendAuctionWonMail(DispatchClient* client, uint64 Sender, uint64 Receiver,int8 ItemName[128],int8 Sellerame[32],uint32 Credits) const;

    void sendTestMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128], BString test) const;

    void sendBidAuctionResponse(DispatchClient* client, uint32 error, uint64 AuctionID) const;
    void sendIsVendorOwnerResponseMessage(DispatchClient* client, BString mBazaarString, uint32 mPermission, uint32 error, uint64 mId) const;
    void processSendCreateItem(DispatchClient* client, uint64 mPlayerID, uint64 mItemID,uint32 mItemGroup, uint32 mPlanetID) const;
    void SendRetrieveAuctionItemResponseMessage(DispatchClient* client, uint64 mItemId, uint32 error) const;
    void SendGetAuctionDetailsResponse(TradeManagerAsyncContainer* asynContainer, AttributesList* mAttributesList) const;
    void sendCanceLiveAuctionResponseMessage(DispatchClient* client, uint32 error, uint64 mAuctionID) const;
    void sendBazaarTransactionMessage(DispatchClient* client, AuctionItem mAuctionItem, uint64 mBuyerId,   uint32 mTime, Player*  mPlayer, Bazaar* mBazaarInfo) const;
    void sendBankTipDeductMessage(DispatchClient* client, uint64 receiverId,   uint32 amount, Player*  mPlayer) const;

    //structures
    void SendHarvesterHopperUpdate(StructureManagerAsyncContainer* asynContainer, HopperResourceList* mHopperList) const;
    void sendStructureConditionActiveUpdate(DispatchClient* client, uint64 structureID, uint32 mPlanetID, uint32 condition) const;

    //void                    _sendBazaarTransactionMessage(DispatchClient* client, AuctionItem mAuctionItem, uint64 mCharId, uint32 mAmount,  uint32 mTime, Player*  mPlayer, string mName);

    void sendSceneCreateObjectByCrc(uint64 objectId, uint32 objectCrc, Player* player) const;
    void sendSceneDestroyObject(uint64 objectId, Player* player) const;
    void sendSceneEndBaselines(uint64 objectId, Player* player) const;
    void sendSystemMessage(Player* target, BString message) const;
    void sendSystemMessageProper(Player* playerObject,uint8 system,BString customMessage,BString mainFile,BString mainVar,BString toFile,BString toVar,BString toCustom,int32 di,BString ttFile,BString ttVar,BString ttCustom,uint64 ttId,uint64 toId,uint64 tuId,BString tuFile,BString tuVar,BString tuCustom) const;
    void sendGroupSystemMessage(BString message, GroupObject* group, bool ignoreLeader = false) const;
    void sendGroupSystemMessage(BString name, BString pointer, Player* target, GroupObject* group, bool unicode = false) const;

    void sendIsmInviteRequest(Player* sender, Player* target) const;
    void sendIsmGroupLootModeResponse(Player* target) const;
    void sendIsmGroupLootMasterResponse(Player* target) const;
    void sendIsmGroupCREO6deltaGroupId(uint64 groupId, Player* target) const;
    void sendIsmInviteInRangeRequest(Player* sender, Player* target);
    void sendIsmIsGroupLeaderResponse(Player* player, uint64 requestId, bool isLeader);

    void sendFriendOnlineStatus(Player* player,Player* playerFriend,uint8 status, BString category, BString name) const;
    void sendFindFriendCreateWaypoint(DispatchClient* client, Player* player, Player* friendPlayer) const;
    void sendFindFriendRequestPosition(DispatchClient* client, Player* playerFriend, Player* player) const;

    //BankTips
    void sendBanktipMail(DispatchClient* client, Player*  playerObject, BString receiverName,uint64 receiverId, uint32 amount) const;

    //CSR
    void sendConnectPlayerResponseMessage(DispatchClient* client) const;
    void sendRequestCategoriesResponseMessage(DispatchClient* client, CategoryList* mCategoryMap) const;
    void sendGetTicketsResponseMessage(DispatchClient* client, Ticket* mTicket) const;
    void sendSearchKnowledgeBaseResponseMessage(DispatchClient* client, ArticleList* list) const;
    void sendGetArticleResponseMessage(DispatchClient* client, Article* article) const;
    void sendGetCommentsResponseMessage(DispatchClient* client, CommentList* list) const;

    //Chat
    void sendChatRoomList(DispatchClient* client, ChannelList* list) const;
    void sendChatOnEnteredRoom(DispatchClient* client, ChatAvatarId* player, Channel* channel, uint32 requestId) const;
    void sendChatOnCreateRoom(DispatchClient* client, Channel* channel, uint32 requestId) const;
    void sendChatOnDestroyRoom(DispatchClient* client, Channel* channel, uint32 requestId) const;
    void sendChatQueryRoomResults(DispatchClient* client, Channel* channel, uint32 requestId) const;
    void sendChatOnLeaveRoom(DispatchClient* client, ChatAvatarId* avatar, Channel* channel, uint32 requestId, uint32 errorCode=0) const;
    void sendChatRoomMessage(Channel* channel, BString galaxy, BString sender, BString message) const;
    void sendChatOnSendRoomMessage(DispatchClient* client, uint32 errorcode, uint32 requestId) const;
    void sendChatOnRemoveModeratorFromRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const;
    void sendChatOnAddModeratorToRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const;
    void sendChatOnInviteToRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const;
    void sendChatOnBanAvatarFromRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const;
    void sendChatOnUnBanAvatarFromRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const;

    void sendChatFailedToEnterRoom(DispatchClient* client, ChatAvatarId* player, uint32 errorcode, Channel* channel, uint32 requestId) const;
    void sendChatFailedToAddMod(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const;
    void sendChatFailedToInvite(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const;
    void sendChatFailedToRemoveMod(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const;
    void sendChatFailedToBan(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const;
    void sendChatFailedToUnban(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const;
    void sendChatOnUninviteFromRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 requestId) const;
    void sendChatFailedToUninviteFromRoom(DispatchClient* client, BString galaxy, BString sender, BString target, Channel* channel, uint32 errorcode, uint32 requestId) const;

private:
    /* Disable the default constructor, copy constructor and assignment operators */
    ChatMessageLib();
    ChatMessageLib(const ChatMessageLib&);
    ChatMessageLib& operator=(const ChatMessageLib&);

    static ChatMessageLib*	mSingleton;
    static bool				mInsFlag;
    DispatchClient*			mClient;
};

#endif