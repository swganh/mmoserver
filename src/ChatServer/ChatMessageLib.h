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

#include "Utils/typedefs.h"
#include "TradeManagerHelp.h"
#include "GroupObject.h"
#include "CSRObjects.h"
#include "LogManager/LogManager.h"

#define	 gChatMessageLib	ChatMessageLib::getSingletonPtr()
#define	 SWG				BString("SWG")

class DispatchClient;
class Message;
class MessageDispatch;
class ItemDescriptionAttributes;
class AuctionItem;
class TradeManagerAsyncContainer;

typedef std::vector<ItemDescriptionAttributes*>	AttributesList;
typedef std::vector<Category*> CategoryList;
typedef std::vector<Article*> ArticleList;
typedef std::vector<Channel*> ChannelList;

class ChatMessageLib
{
public:
		
	static ChatMessageLib*	getSingletonPtr() { return mSingleton; }
	static ChatMessageLib*	Init(DispatchClient* client);

	ChatMessageLib(DispatchClient* client);
	
	//Mail
	void					sendChatonPersistantMessage(DispatchClient* client, uint64 mailCounter);
	void					sendChatPersistantMessagetoClient(DispatchClient* client,Mail* mail, uint32 mailId,uint32 mailCounter,uint8 status);
	void					sendChatPersistantMessagetoClient(DispatchClient* client,Mail* mail);

	//Bazaar
	void					ItemExpiredMail(DispatchClient* client, uint64 Receiver, int8 ItemName[128]);
	void					sendCancelAuctionMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128]);
	void                    sendSoldInstantMail(DispatchClient* client,uint64 Sender, uint64 Receiver, int8 ItemName[128], int8 BuyerName[128], int8 Region[128], uint32 Credits);
	void                    sendSoldAuctionMail(DispatchClient* client,uint64 Sender, uint64 Receiver, int8 ItemName[128], int8 BuyerName[128], int8 Region[128], uint32 Credits);
	void                    sendBidderCancelAuctionMail(DispatchClient* client, uint64 mSender, uint64 mReceiver, int8 mItemName[128]);
	void                    sendAuctionOutbidMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128]);
	void					sendAuctionWonMail(DispatchClient* client, uint64 Sender, uint64 Receiver,int8 ItemName[128],int8 Sellerame[32],uint32 Credits);

	void					sendTestMail(DispatchClient* client, uint64 Sender, uint64 Receiver, int8 ItemName[128], string test);

	void					sendBidAuctionResponse(DispatchClient* client, uint32 error, uint64 AuctionID);
	void					sendIsVendorOwnerResponseMessage(DispatchClient* client, string mBazaarString, uint32 mPermission, uint32 error, uint64 mId);
	void					processSendCreateItem(DispatchClient* client, uint64 mPlayerID, uint64 mItemID,uint32 mItemGroup, uint32 mPlanetID);
	void                    SendRetrieveAuctionItemResponseMessage(DispatchClient* client, uint64 mItemId, uint32 error);
	void					SendGetAuctionDetailsResponse(TradeManagerAsyncContainer* asynContainer, AttributesList* mAttributesList);
	void					sendCanceLiveAuctionResponseMessage(DispatchClient* client, uint32 error, uint64 mAuctionID);
	void					sendBazaarTransactionMessage(DispatchClient* client, AuctionItem mAuctionItem, uint64 mBuyerId,   uint32 mTime, Player*  mPlayer, Bazaar* mBazaarInfo);
	void					sendBankTipDeductMessage(DispatchClient* client, uint64 receiverId,   uint32 amount, Player*  mPlayer);
	
	//void                    _sendBazaarTransactionMessage(DispatchClient* client, AuctionItem mAuctionItem, uint64 mCharId, uint32 mAmount,  uint32 mTime, Player*  mPlayer, string mName);

	void					sendSceneCreateObjectByCrc(uint64 objectId, uint32 objectCrc, Player* player);
	void					sendSceneDestroyObject(uint64 objectId, Player* player);
	void					sendSceneEndBaselines(uint64 objectId, Player* player);
	void					sendSystemMessage(Player* target, string message);
	void					sendSystemMessageProper(Player* playerObject,uint8 system,string customMessage,string mainFile,string mainVar,string toFile,string toVar,string toCustom,int32 di,string ttFile,string ttVar,string ttCustom,uint64 ttId,uint64 toId,uint64 tuId,string tuFile,string tuVar,string tuCustom);
	void					sendGroupSystemMessage(string message, GroupObject* group, bool ignoreLeader = false);
	void					sendGroupSystemMessage(string name, string pointer, Player* target, GroupObject* group, bool unicode = false);
	
	void					sendIsmInviteRequest(Player* sender, Player* target);
	void					sendIsmGroupLootModeResponse(Player* target);
	void					sendIsmGroupLootMasterResponse(Player* target);
	void					sendIsmGroupCREO6deltaGroupId(uint64 groupId, Player* target);

	void					sendFriendOnlineStatus(Player* player,Player* playerFriend,uint8 status, string category, string name);
	void					sendFindFriendCreateWaypoint(DispatchClient* client, Player* player, Player* friendPlayer);
	void					sendFindFriendRequestPosition(DispatchClient* client, Player* playerFriend, Player* player);

	//BankTips
	void					sendBanktipMail(DispatchClient* client, Player*  playerObject, string receiverName,uint64 receiverId, uint32 amount);

	//CSR
	void					sendConnectPlayerResponseMessage(DispatchClient* client);
	void					sendRequestCategoriesResponseMessage(DispatchClient* client, CategoryList* mCategoryMap);
	void					sendGetTicketsResponseMessage(DispatchClient* client, Ticket* mTicket);
	void					sendSearchKnowledgeBaseResponseMessage(DispatchClient* client, ArticleList* list);
	void					sendGetArticleResponseMessage(DispatchClient* client, Article* article);
	void					sendGetCommentsResponseMessage(DispatchClient* client, CommentList* list);

	//Chat
	void					sendChatRoomList(DispatchClient* client, ChannelList* list);
	void					sendChatOnEnteredRoom(DispatchClient* client, ChatAvatarId* player, Channel* channel, uint32 requestId);
	void					sendChatOnCreateRoom(DispatchClient* client, Channel* channel, uint32 requestId);
	void					sendChatOnDestroyRoom(DispatchClient* client, Channel* channel, uint32 requestId);
	void					sendChatQueryRoomResults(DispatchClient* client, Channel* channel, uint32 requestId);
	void					sendChatOnLeaveRoom(DispatchClient* client, ChatAvatarId* avatar, Channel* channel, uint32 requestId, uint32 errorCode=0);
	void					sendChatRoomMessage(Channel* channel, string galaxy, string sender, string message);
	void					sendChatOnSendRoomMessage(DispatchClient* client, uint32 errorcode, uint32 requestId);
	void					sendChatOnRemoveModeratorFromRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId);
	void					sendChatOnAddModeratorToRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId);
	void					sendChatOnInviteToRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId);
	void					sendChatOnBanAvatarFromRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId);
	void					sendChatOnUnBanAvatarFromRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId);

	void					sendChatFailedToEnterRoom(DispatchClient* client, ChatAvatarId* player, uint32 errorcode, Channel* channel, uint32 requestId);
	void					sendChatFailedToAddMod(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId);
	void					sendChatFailedToInvite(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId);
	void					sendChatFailedToRemoveMod(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId);
	void					sendChatFailedToBan(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId);
	void					sendChatFailedToUnban(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId);
	void					sendChatOnUninviteFromRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 requestId);
	void					sendChatFailedToUninviteFromRoom(DispatchClient* client, string galaxy, string sender, string target, Channel* channel, uint32 errorcode, uint32 requestId);

private:
	static ChatMessageLib*	mSingleton;
	static bool				mInsFlag;
	DispatchClient*			mClient;


};

#endif