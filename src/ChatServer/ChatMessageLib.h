/*
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CHATSERVER_MESSAGELIB_H
#define ANH_CHATSERVER_MESSAGELIB_H

// This was enabled for test of beeing able to display and present all player names, as they where typed at creation,
// in dialogs, windows and popups that have to do with user created channels. And still have them saved with all lowercase in the database.
// #define DISP_REAL_FIRST_NAME 1

#include <vector>
#include "Utils/typedefs.h"

#define	 gChatMessageLib	ChatMessageLib::getSingletonPtr()
#define	 SWG				BString("SWG")

class Article;
class AuctionItem;
class Bazaar;
class Category;
class Channel;
class ChatAvatarId;
class Comment;
class DispatchClient;
class GroupObject;
class ItemDescriptionAttributes;
class Mail;
class Message;
class MessageDispatch;
class Player;
class Ticket;
class TradeManagerAsyncContainer;

typedef std::vector<Article*>                   ArticleList;
typedef std::vector<ItemDescriptionAttributes*>	AttributesList;
typedef std::vector<Category*>                  CategoryList;
typedef std::vector<Channel*>                   ChannelList;
typedef std::vector<Comment*>                   CommentList;

class ChatMessageLib
{
public:
	static ChatMessageLib*	getSingletonPtr() { return mSingleton; }
	static ChatMessageLib*	Init(DispatchClient* client);

	explicit ChatMessageLib(DispatchClient* client);
	
	//Mail
	void sendChatonPersistantMessage(DispatchClient* client, uint64 mailCounter) const;
	void sendChatPersistantMessagetoClient(DispatchClient* client,Mail* mail, uint32 mailId,uint32 mailCounter,uint8 status) const;
	void sendChatPersistantMessagetoClient(DispatchClient* client,Mail* mail) const;

	//Bazaar
	void ItemExpiredMail(DispatchClient* client, uint64 Receiver, int8 ItemName[128]) const;
	void sendCancelAuctionMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128]) const;
	void sendSoldInstantMail(DispatchClient* client,uint64 Sender, uint64 Receiver, int8 ItemName[128], int8 BuyerName[128], int8 Region[128], uint32 Credits) const;
	void sendSoldAuctionMail(DispatchClient* client,uint64 Sender, uint64 Receiver, int8 ItemName[128], int8 BuyerName[128], int8 Region[128], uint32 Credits) const;
	void sendBidderCancelAuctionMail(DispatchClient* client, uint64 mSender, uint64 mReceiver, int8 mItemName[128]) const;
	void sendAuctionOutbidMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128]) const;
	void sendAuctionWonMail(DispatchClient* client, uint64 Sender, uint64 Receiver,int8 ItemName[128],int8 Sellerame[32],uint32 Credits) const;

	void sendTestMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128], string test) const;

	void sendBidAuctionResponse(DispatchClient* client, uint32 error, uint64 AuctionID) const;
	void sendIsVendorOwnerResponseMessage(DispatchClient* client, string mBazaarString, uint32 mPermission, uint32 error, uint64 mId) const;
	void processSendCreateItem(DispatchClient* client, uint64 mPlayerID, uint64 mItemID,uint32 mItemGroup, uint32 mPlanetID) const;
	void SendRetrieveAuctionItemResponseMessage(DispatchClient* client, uint64 mItemId, uint32 error) const;
	void SendGetAuctionDetailsResponse(TradeManagerAsyncContainer* asynContainer, AttributesList* mAttributesList) const;
	void sendCanceLiveAuctionResponseMessage(DispatchClient* client, uint32 error, uint64 mAuctionID) const;
	void sendBazaarTransactionMessage(DispatchClient* client, AuctionItem mAuctionItem, uint64 mBuyerId,   uint32 mTime, Player*  mPlayer, Bazaar* mBazaarInfo) const;
	void sendBankTipDeductMessage(DispatchClient* client, uint64 receiverId,   uint32 amount, Player*  mPlayer) const;
	
	//void                    _sendBazaarTransactionMessage(DispatchClient* client, AuctionItem mAuctionItem, uint64 mCharId, uint32 mAmount,  uint32 mTime, Player*  mPlayer, string mName);

	void sendSceneCreateObjectByCrc(uint64 objectId, uint32 objectCrc, Player* player) const;
	void sendSceneDestroyObject(uint64 objectId, Player* player) const;
	void sendSceneEndBaselines(uint64 objectId, Player* player) const;
	void sendSystemMessage(Player* target, string message) const;
	void sendSystemMessageProper(Player* playerObject,uint8 system,string customMessage,string mainFile,string mainVar,string toFile,string toVar,string toCustom,int32 di,string ttFile,string ttVar,string ttCustom,uint64 ttId,uint64 toId,uint64 tuId,string tuFile,string tuVar,string tuCustom) const;
	void sendGroupSystemMessage(string message, GroupObject* group, bool ignoreLeader = false) const;
	void sendGroupSystemMessage(string name, string pointer, Player* target, GroupObject* group, bool unicode = false) const;
	
	void sendIsmInviteRequest(Player* sender, Player* target) const;
	void sendIsmGroupLootModeResponse(Player* target) const;
	void sendIsmGroupLootMasterResponse(Player* target) const;
	void sendIsmGroupCREO6deltaGroupId(uint64 groupId, Player* target) const;

	void sendFriendOnlineStatus(Player* player,Player* playerFriend,uint8 status, string category, string name) const;
	void sendFindFriendCreateWaypoint(DispatchClient* client, Player* player, Player* friendPlayer) const;
	void sendFindFriendRequestPosition(DispatchClient* client, Player* playerFriend, Player* player) const;

	//BankTips
	void sendBanktipMail(DispatchClient* client, Player*  playerObject, string receiverName,uint64 receiverId, uint32 amount) const;

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
	void sendChatRoomMessage(Channel* channel, string galaxy, string sender, string message) const;
	void sendChatOnSendRoomMessage(DispatchClient* client, uint32 errorcode, uint32 requestId) const;
	void sendChatOnRemoveModeratorFromRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId) const;
	void sendChatOnAddModeratorToRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId) const;
	void sendChatOnInviteToRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId) const;
	void sendChatOnBanAvatarFromRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId) const;
	void sendChatOnUnBanAvatarFromRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId) const;

	void sendChatFailedToEnterRoom(DispatchClient* client, ChatAvatarId* player, uint32 errorcode, Channel* channel, uint32 requestId) const;
	void sendChatFailedToAddMod(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId) const;
	void sendChatFailedToInvite(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId) const;
	void sendChatFailedToRemoveMod(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId) const;
	void sendChatFailedToBan(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId) const;
	void sendChatFailedToUnban(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId) const;
	void sendChatOnUninviteFromRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId) const;
	void sendChatFailedToUninviteFromRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId) const;

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