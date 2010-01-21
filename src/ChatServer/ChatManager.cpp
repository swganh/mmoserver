/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Channel.h"
#include "ChatAvatarId.h"
#include "ChatManager.h"
#include "ChatMessageLib.h"
#include "ChatOpcodes.h"
#include "GroupManager.h"
#include "GroupObject.h"
#include "Mail.h"
#include "Player.h"

#include "LogManager/LogManager.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Common/atMacroString.h"
#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"

#include "Utils/typedefs.h"
#include "Utils/utils.h"

#include <cassert>
#include <cstring>
#include <ctime>

//======================================================================================================================

bool			ChatManager::mInsFlag = false;
ChatManager*	ChatManager::mSingleton = NULL;

//======================================================================================================================

ChatManager::ChatManager(Database* database,MessageDispatch* dispatch) :
mDatabase(database),
mMessageDispatch(dispatch)
{
	mMainCategory = "SWG";

	_registerCallbacks();
	_loadCommandMap();
	_loadDatabindings();

	ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_GalaxyName);
	// Commented out the filter for now, at a later time this needs to be updated to not be bound to a single galaxy
	// mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT name FROM galaxy;"); // WHERE galaxy_id=3");
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT name FROM galaxy WHERE galaxy_id=2");

	asyncContainer = new ChatAsyncContainer(ChatQuery_Channels);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT chat_channels.id,chat_channels.name,chat_channels.private,chat_channels.moderated,chat_channels.creator, chat_channels.owner, chat_channels.title FROM chat_channels;");

	asyncContainer = new ChatAsyncContainer(ChatQuery_PlanetNames);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT planet.name FROM planet ORDER BY planet.planet_id;");
}

//======================================================================================================================

ChatManager* ChatManager::Init(Database* database,MessageDispatch* dispatch)
{
	if(mInsFlag == false)
	{
		mSingleton = new ChatManager(database,dispatch);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//======================================================================================================================

ChatManager::~ChatManager()
{
	ChannelList::iterator index = mvChannels.begin();
	while(!mvChannels.empty())
	{
		delete (*index);
		mvChannels.erase(index);
		index = mvChannels.begin();
	}
	_destroyDatabindings();
	_unregisterCallbacks();

	mInsFlag = false;
	mSingleton = NULL;
}

//======================================================================================================================

void ChatManager::_loadCommandMap()
{
	mCommandMap.insert(std::make_pair(opChatRequestRoomlist,&ChatManager::_processRoomlistRequest));
	mCommandMap.insert(std::make_pair(opChatCreateRoom,&ChatManager::_processCreateRoom));
	mCommandMap.insert(std::make_pair(opChatDestroyRoom,&ChatManager::_processDestroyRoom));
	mCommandMap.insert(std::make_pair(opChatEnterRoomById,&ChatManager::_processEnterRoomById));
	mCommandMap.insert(std::make_pair(opChatQueryRoom,&ChatManager::_processRoomQuery));
	mCommandMap.insert(std::make_pair(opChatRoomMessage,&ChatManager::_processRoomMessage));
	mCommandMap.insert(std::make_pair(opChatSendToRoom,&ChatManager::_processSendToRoom));
	mCommandMap.insert(std::make_pair(opChatAddModeratorToRoom,&ChatManager::_processAddModeratorToRoom));
	mCommandMap.insert(std::make_pair(opChatInviteAvatarToRoom,&ChatManager::_processInviteAvatarToRoom));
	mCommandMap.insert(std::make_pair(opChatUninviteFromRoom,&ChatManager::_processUninviteAvatarFromRoom));
	mCommandMap.insert(std::make_pair(opChatRemoveModFromRoom,&ChatManager::_processRemoveModFromRoom));
	mCommandMap.insert(std::make_pair(opChatRemoveAvatarFromRoom,&ChatManager::_processRemoveAvatarFromRoom));
	mCommandMap.insert(std::make_pair(opChatBanAvatarFromRoom,&ChatManager::_processBanAvatarFromRoom));
	mCommandMap.insert(std::make_pair(opChatUnbanAvatarFromRoom,&ChatManager::_processUnbanAvatarFromRoom));
	mCommandMap.insert(std::make_pair(opChatAvatarId,&ChatManager::_processAvatarId));
	mCommandMap.insert(std::make_pair(opChatInstantMessageToCharacter,&ChatManager::_processInstantMessageToCharacter));
	mCommandMap.insert(std::make_pair(opChatPersistentMessageToServer,&ChatManager::_processPersistentMessageToServer));
	mCommandMap.insert(std::make_pair(opChatRequestPersistentMessage,&ChatManager::_processRequestPersistentMessage));
	mCommandMap.insert(std::make_pair(opChatDeletePersistentMessage,&ChatManager::_processDeletePersistentMessage));
	mCommandMap.insert(std::make_pair(opChatFriendlistUpdate,&ChatManager::_processFriendlistUpdate));
	mCommandMap.insert(std::make_pair(opChatAddFriend,&ChatManager::_processAddFriend));

	mCommandMap.insert(std::make_pair(opClusterClientConnect,&ChatManager::_processClusterClientConnect));
	mCommandMap.insert(std::make_pair(opClusterClientDisconnect,&ChatManager::_processClusterClientDisconnect));
	mCommandMap.insert(std::make_pair(opClusterZoneTransferCharacter,&ChatManager::_processZoneTransfer));
	mCommandMap.insert(std::make_pair(opChatNotifySceneReady,&ChatManager::_processWhenLoaded));


	mCommandMap.insert(std::make_pair(opNotifyChatAddFriend,&ChatManager::_processNotifyChatAddFriend));
	mCommandMap.insert(std::make_pair(opNotifyChatRemoveFriend,&ChatManager::_processNotifyChatRemoveFriend));
	mCommandMap.insert(std::make_pair(opNotifyChatAddIgnore,&ChatManager::_processNotifyChatAddIgnore));
	mCommandMap.insert(std::make_pair(opNotifyChatRemoveIgnore,&ChatManager::_processNotifyChatRemoveIgnore));

	mCommandMap.insert(std::make_pair(opSendSystemMailMessage,&ChatManager::_processSystemMailMessage));
	mCommandMap.insert(std::make_pair(opNotifyChatFindFriend,&ChatManager::_processFindFriendMessage));
	mCommandMap.insert(std::make_pair(opFindFriendSendPosition,&ChatManager::_processFindFriendGotPosition));

	mCommandMap.insert(std::make_pair(opIsmGroupSay,&ChatManager::_processGroupSaySend));
	mCommandMap.insert(std::make_pair(opIsmBroadcastGalaxy,&ChatManager::_processBroadcastGalaxy));
	mCommandMap.insert(std::make_pair(opIsmScheduleShutdown,&ChatManager::_processScheduleShutdown));
	mCommandMap.insert(std::make_pair(opIsmCancelShutdown,&ChatManager::_processCancelScheduledShutdown));

}

//======================================================================================================================

void ChatManager::_loadChannels(DatabaseResult* result)
{
	uint64 count = result->getRowCount();

	for(uint64 i = 0;i < count;i++)
	{
		string creator;

		string owner;
		Channel* channel = new Channel();
		result->GetNextRow(mChannelBinding,channel->getChannelData());

		result->ResetRowIndex(static_cast<int>(i));
		result->GetNextRow(mCreatorBinding,&creator);

		result->ResetRowIndex(static_cast<int>(i));
		result->GetNextRow(mOwnerBinding,&owner);

		if (strcmp(creator.getRawData(), "SYSTEM") == 0)
			channel->setCreator(gSystemAvatar);
		else
		{
			ChatAvatarId* avatar = new ChatAvatarId();
			avatar->setPlayer(creator);
			avatar->setGalaxy(mGalaxyName);

			channel->setCreator(avatar);
		}

		if (strcmp(owner.getRawData(), "SYSTEM") == 0)
			channel->setOwner(gSystemAvatar);
		else
		{
			ChatAvatarId* avatar = new ChatAvatarId();
			avatar->setPlayer(owner);
			avatar->setGalaxy(mGalaxyName);

			channel->setOwner(avatar);
		}
		channel->setGalaxy(mGalaxyName);

		uint32 crc = channel->getName().getCrc();

		mChannelNameMap.insert(std::make_pair(crc, channel));
		mChannelMap.insert(std::make_pair(channel->getId(),channel));
		mvChannels.push_back(channel);

		ChatAsyncContainer* modContainer = new ChatAsyncContainer(ChatQuery_Moderators);
		ChatAsyncContainer* bannedContainer = new ChatAsyncContainer(ChatQuery_Banned);
		ChatAsyncContainer* inviteContainer = new ChatAsyncContainer(ChatQuery_Invited);

		modContainer->mChannel = channel;
		bannedContainer->mChannel = channel;
		inviteContainer->mChannel = channel;

		mDatabase->ExecuteSqlAsync(this, modContainer, "SELECT char_name FROM chat_channels_moderators WHERE channel_id = %u", channel->getId());
		mDatabase->ExecuteSqlAsync(this, bannedContainer, "SELECT char_name FROM chat_channels_banned WHERE channel_id = %u", channel->getId());
		mDatabase->ExecuteSqlAsync(this, inviteContainer, "SELECT char_name FROM chat_channels_invited WHERE channel_id = %u", channel->getId());
	}
}

//======================================================================================================================

void ChatManager::_loadDatabindings()
{
	mPlayerBinding = mDatabase->CreateDataBinding(2);
	mPlayerBinding->addField(DFT_bstring, offsetof(PlayerData, name),				64, 0);
	mPlayerBinding->addField(DFT_bstring, offsetof(PlayerData, last_name),	64, 1);

	mChannelBinding = mDatabase->CreateDataBinding(5);
	mChannelBinding->addField(DFT_uint32,	offsetof(ChannelData,id),			4,		0);
	mChannelBinding->addField(DFT_bstring,	offsetof(ChannelData,name),			65,		1);
	mChannelBinding->addField(DFT_uint8,	offsetof(ChannelData,is_private),	1,		2);
	mChannelBinding->addField(DFT_uint8,	offsetof(ChannelData,is_moderated),	1,		3);
	mChannelBinding->addField(DFT_bstring,	offsetof(ChannelData,title),		256,	6);

	mCreatorBinding = mDatabase->CreateDataBinding(1);
	mCreatorBinding->addField(DFT_bstring, 0, 33, 4);

	mOwnerBinding = mDatabase->CreateDataBinding(1);
	mOwnerBinding->addField(DFT_bstring, 0, 33, 5);

	mMailBinding = mDatabase->CreateDataBinding(7);
	mMailBinding->addField(DFT_uint32,	offsetof(Mail,mId),					4,		0);
	mMailBinding->addField(DFT_bstring,	offsetof(Mail,mSender),				64,		1);
	mMailBinding->addField(DFT_bstring,	offsetof(Mail,mSubject),			256,	2);
	mMailBinding->addField(DFT_bstring,	offsetof(Mail,mText),				8192,	3);
	mMailBinding->addField(DFT_uint32,	offsetof(Mail,mTime),				4,		4);
	mMailBinding->addField(DFT_raw,		offsetof(Mail,mAttachmentRaw),		2048,	5);
	mMailBinding->addField(DFT_uint32,	offsetof(Mail,mAttachmentSize),		4,		6);

	mMailHeaderBinding = mDatabase->CreateDataBinding(5);
	mMailHeaderBinding->addField(DFT_uint32,	offsetof(Mail,mId),			4,0);
	mMailHeaderBinding->addField(DFT_bstring,	offsetof(Mail,mSender),		128,	1);
	mMailHeaderBinding->addField(DFT_bstring,	offsetof(Mail,mSubject),	256,	2);
	mMailHeaderBinding->addField(DFT_uint8,		offsetof(Mail,mStatus),		1,		3);
	mMailHeaderBinding->addField(DFT_uint32,	offsetof(Mail,mTime),		4,		4);



}

//======================================================================================================================

void ChatManager::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mPlayerBinding);
	mDatabase->DestroyDataBinding(mChannelBinding);
	mDatabase->DestroyDataBinding(mMailBinding);
	mDatabase->DestroyDataBinding(mMailHeaderBinding);
}

//======================================================================================================================

void ChatManager::registerChannel(Channel* channel)
{
	mChannelMap.insert(std::make_pair(channel->getId(),channel));
}

//======================================================================================================================

void ChatManager::unregisterChannel(Channel* channel)
{
	ChannelMap::iterator it = mChannelMap.find(channel->getId());

	if(it !=  mChannelMap.end())
	{
		mChannelMap.erase(it);
	}
	else
	{
		gLogger->logMsgF("Could not find channel for removing. %u",MSG_NORMAL,channel->getId());
	}
}

//======================================================================================================================

void ChatManager::_registerCallbacks()
{
	mMessageDispatch->RegisterMessageCallback(opClusterClientConnect,this);
	mMessageDispatch->RegisterMessageCallback(opClusterClientDisconnect,this);
	mMessageDispatch->RegisterMessageCallback(opClusterZoneTransferCharacter,this);
	mMessageDispatch->RegisterMessageCallback(opChatNotifySceneReady,this);
	mMessageDispatch->RegisterMessageCallback(opChatRequestRoomlist,this);
	mMessageDispatch->RegisterMessageCallback(opChatCreateRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatDestroyRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatEnterRoomById,this);
	mMessageDispatch->RegisterMessageCallback(opChatQueryRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatRoomMessage,this);
	mMessageDispatch->RegisterMessageCallback(opChatSendToRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatAddModeratorToRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatInviteAvatarToRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatUninviteFromRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatRemoveModFromRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatRemoveAvatarFromRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatBanAvatarFromRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatUnbanAvatarFromRoom,this);
	mMessageDispatch->RegisterMessageCallback(opChatAvatarId,this);

	mMessageDispatch->RegisterMessageCallback(opChatInstantMessageToCharacter,this);

	mMessageDispatch->RegisterMessageCallback(opChatPersistentMessageToServer,this);
	mMessageDispatch->RegisterMessageCallback(opChatRequestPersistentMessage,this);
	mMessageDispatch->RegisterMessageCallback(opChatDeletePersistentMessage,this);

	mMessageDispatch->RegisterMessageCallback(opChatFriendlistUpdate,this);
	mMessageDispatch->RegisterMessageCallback(opChatAddFriend,this);

	mMessageDispatch->RegisterMessageCallback(opNotifyChatAddFriend,this);
	mMessageDispatch->RegisterMessageCallback(opNotifyChatRemoveFriend,this);
	mMessageDispatch->RegisterMessageCallback(opNotifyChatAddIgnore,this);
	mMessageDispatch->RegisterMessageCallback(opNotifyChatRemoveIgnore,this);
	mMessageDispatch->RegisterMessageCallback(opNotifyChatFindFriend,this);
	mMessageDispatch->RegisterMessageCallback(opFindFriendSendPosition,this);

	mMessageDispatch->RegisterMessageCallback(opSendSystemMailMessage,this);

	mMessageDispatch->RegisterMessageCallback(opIsmGroupSay,this);
	mMessageDispatch->RegisterMessageCallback(opIsmBroadcastGalaxy,this);
	mMessageDispatch->RegisterMessageCallback(opIsmScheduleShutdown,this);
	mMessageDispatch->RegisterMessageCallback(opIsmCancelShutdown,this);
}

//======================================================================================================================

void ChatManager::_unregisterCallbacks()
{
	mMessageDispatch->UnregisterMessageCallback(opClusterClientConnect);
	mMessageDispatch->UnregisterMessageCallback(opClusterClientDisconnect);
	mMessageDispatch->UnregisterMessageCallback(opClusterZoneTransferCharacter);
	mMessageDispatch->UnregisterMessageCallback(opChatNotifySceneReady);
	mMessageDispatch->UnregisterMessageCallback(opConnectPlayerMessage);
	mMessageDispatch->UnregisterMessageCallback(opChatRequestRoomlist);
	mMessageDispatch->UnregisterMessageCallback(opChatCreateRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatDestroyRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatEnterRoomById);
	mMessageDispatch->UnregisterMessageCallback(opChatQueryRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatRoomMessage);
	mMessageDispatch->UnregisterMessageCallback(opChatSendToRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatAddModeratorToRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatInviteAvatarToRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatUninviteFromRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatRemoveModFromRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatRemoveAvatarFromRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatBanAvatarFromRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatUnbanAvatarFromRoom);
	mMessageDispatch->UnregisterMessageCallback(opChatAvatarId);

	mMessageDispatch->UnregisterMessageCallback(opChatInstantMessageToCharacter);

	mMessageDispatch->UnregisterMessageCallback(opChatPersistentMessageToServer);
	mMessageDispatch->UnregisterMessageCallback(opChatRequestPersistentMessage);
	mMessageDispatch->UnregisterMessageCallback(opChatDeletePersistentMessage);

	mMessageDispatch->UnregisterMessageCallback(opChatFriendlistUpdate);
	mMessageDispatch->UnregisterMessageCallback(opChatAddFriend);

	mMessageDispatch->UnregisterMessageCallback(opNotifyChatAddFriend);
	mMessageDispatch->UnregisterMessageCallback(opNotifyChatRemoveFriend);
	mMessageDispatch->UnregisterMessageCallback(opNotifyChatAddIgnore);
	mMessageDispatch->UnregisterMessageCallback(opNotifyChatRemoveIgnore);
	mMessageDispatch->UnregisterMessageCallback(opNotifyChatFindFriend);
	mMessageDispatch->UnregisterMessageCallback(opFindFriendSendPosition);

	mMessageDispatch->UnregisterMessageCallback(opSendSystemMailMessage);

	//mMessageDispatch->UnregisterMessageCallback(opConnectPlayerMessage);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupSay);
	mMessageDispatch->UnregisterMessageCallback(opIsmBroadcastGalaxy);
	mMessageDispatch->UnregisterMessageCallback(opIsmScheduleShutdown);
	mMessageDispatch->UnregisterMessageCallback(opIsmCancelShutdown);
}

//======================================================================================================================

void ChatManager::handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client)
{
	CommandMap::iterator it = mCommandMap.find(opcode);

	if(it != mCommandMap.end())
		(this->*((*it).second))(message,client);
	else
		gLogger->logMsgF("Unhandled DispatchMsg %u",MSG_NORMAL,opcode);
}

//======================================================================================================================

void ChatManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	ChatAsyncContainer* asyncContainer = (ChatAsyncContainer*)ref;

	// gLogger->logMsg("ChatManager::handleDatabaseJobComplete",MSG_NORMAL);

	switch(asyncContainer->mQueryType)
	{

		case ChatQuery_FindFriend:
		{
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);

			uint64	ret		= 0;
			uint64	count	= result->getRowCount();

			result->GetNextRow(binding,&ret);
			if(count == 1)
				count = ret;

			mDatabase->DestroyDataBinding(binding);

			_handleFindFriendDBReply(asyncContainer->mSender,count,asyncContainer->mName);

		}
		break;

		case ChatQuery_Player:
		{
			uint64 count = result->getRowCount();
			assert(count == 1);

			PlayerAccountMap::iterator it = mPlayerAccountMap.find(asyncContainer->mClient->getAccountId());

			if(it != mPlayerAccountMap.end())
			{
				Player* player = (*it).second;

				result->GetNextRow(mPlayerBinding,player->getPlayerData());

				player->setKey();

				mPlayerNameMap.insert(std::make_pair(player->getKey(),player));

				// query friendslist
				ChatAsyncContainer* asContainer = new ChatAsyncContainer(ChatQuery_PlayerFriends);
				asContainer->mClient = asyncContainer->mClient;
				asContainer->mReceiver	= player;

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT characters.firstname FROM chat_friendlist "
															"INNER JOIN characters ON (chat_friendlist.friend_id = characters.id) "
															"WHERE (chat_friendlist.character_id = %"PRIu64")",asContainer->mReceiver->getCharId());
			}
			else
				gLogger->logMsgF("Could not find account %u",MSG_NORMAL,asyncContainer->mClient->getAccountId());
		}
		break;

		case ChatQuery_GalaxyName:
		{
			uint64 count = result->getRowCount();
			if (count != 1)
				gLogger->logMsg("Could not find Galaxy ",MSG_NORMAL);
			assert(count == 1);


			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,64);

			result->GetNextRow(binding,&mGalaxyName);

			gLogger->logMsgF("Main: [%s] Galaxy: [%s]",MSG_NORMAL,mMainCategory.getAnsi(),mGalaxyName.getAnsi());

			mDatabase->DestroyDataBinding(binding);
		}
		break;


		case ChatQuery_CheckCharacter:
		{
			uint64 receiverId = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);

			uint64 count = result->getRowCount();
			assert(count <= 1);

			if(count)
			{
				result->GetNextRow(binding,&receiverId);

				ChatAsyncContainer* asContainer = new ChatAsyncContainer(ChatQuery_CreateMail);
				asContainer->mMail = asyncContainer->mMail;
				asContainer->mMailCounter = asyncContainer->mMailCounter;	// Contains mailId
				asContainer->mSender = asyncContainer->mSender;
				asContainer->mReceiverId = receiverId;

				int8 sql[20000],*sqlPointer;
				int8 footer[64];
				int8 receiverStr[64];
				sprintf(receiverStr,"',%"PRIu64",'",receiverId);
				sprintf(footer,",%u,%"PRIu32")",(asyncContainer->mMail->mAttachments.getLength() << 1),asyncContainer->mMail->mTime);
				sprintf(sql,"SELECT sf_MailCreate('");

				sqlPointer = sql + strlen(sql);

				//if a system send Mail, the sender may be NULL


				sqlPointer += mDatabase->Escape_String(sqlPointer,asyncContainer->mMail->getSender().getAnsi(),asyncContainer->mMail->getSender().getLength());

				strcat(sql,receiverStr);
				sqlPointer = sql + strlen(sql);
				sqlPointer += mDatabase->Escape_String(sqlPointer,asyncContainer->mMail->mSubject.getAnsi(),asyncContainer->mMail->mSubject.getLength());
				*sqlPointer++ = '\'';
				*sqlPointer++ = ',';
				*sqlPointer++ = '\'';
				sqlPointer += mDatabase->Escape_String(sqlPointer,asyncContainer->mMail->mText.getAnsi(),asyncContainer->mMail->mText.getLength());
				*sqlPointer++ = '\'';
				*sqlPointer++ = ',';
				*sqlPointer++ = '\'';
				sqlPointer += mDatabase->Escape_String(sqlPointer,asyncContainer->mMail->mAttachments.getRawData(),(asyncContainer->mMail->mAttachments.getLength() << 1));
				*sqlPointer++ = '\'';
				*sqlPointer++ = '\0';
				strcat(sql,footer);

				mDatabase->ExecuteSqlAsync(this,asContainer,sql);
			}
			else
			{
				if(asyncContainer->mSender)
					gChatMessageLib->sendChatonPersistantMessage((asyncContainer->mSender)->getClient(),asyncContainer->mMailCounter);

				SAFE_DELETE(asyncContainer->mMail);
			}

			mDatabase->DestroyDataBinding(binding);
		}
		break;


		case ChatQuery_CreateMail:
		{
			uint32 dbMailId = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint64 count = result->getRowCount();
			assert(count == 1);

			result->GetNextRow(binding,&dbMailId);

			// query ignoreslist
			ChatAsyncContainer* asContainer = new ChatAsyncContainer(ChatMailQuery_PlayerIgnores);

			asContainer->mMail = asyncContainer->mMail;
			asContainer->mSender = asyncContainer->mSender;
			asContainer->mRequestId = dbMailId;
			asContainer->mMailCounter = asyncContainer->mMailCounter;
			asContainer->mReceiverId = asyncContainer->mReceiverId;

			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT characters.firstname FROM chat_ignorelist "
														"INNER JOIN characters ON (chat_ignorelist.ignore_id = characters.id) "
														"WHERE (chat_ignorelist.character_id = %"PRIu64")",asyncContainer->mReceiverId);
			mDatabase->DestroyDataBinding(binding);
		}
		break;


		case ChatMailQuery_PlayerIgnores:
		{
			Message* newMessage;
			string	name;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,64);

			uint64 count = result->getRowCount();

			Player*	receiver = getPlayerbyId(asyncContainer->mReceiverId);

			ContactMap ignoreList;

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&name);
				name.toLower();
				if (receiver)
				{
					gLogger->logMsgF(" %s ChatMailQuery_PlayerIgnores [%s]",MSG_NORMAL,receiver->getName().getAnsi(), name.getAnsi());
				}
				ignoreList.insert(std::make_pair(name.getCrc(),name.getAnsi()));
			}

			bool bIgnore = false;

			// If receiver have sender ignored, auto delete mail (don't send it and remove from db).
			if (asyncContainer->mSender != NULL)
			{
				string ignoreName = asyncContainer->mSender->getName();
				ignoreName.toLower();

				// check receivers ignorelist.
				ContactMap::iterator it = ignoreList.find(ignoreName.getCrc());
				bIgnore = (it != ignoreList.end());
				if (bIgnore)
				{
					mDatabase->ExecuteSqlAsync(NULL,NULL,"DELETE FROM chat_mail WHERE id=%u",asyncContainer->mRequestId);
				}
			}
			if ((receiver != NULL) && (!bIgnore))
			{
				// send out new mail notification, if not receiving mail from Ignored player.
				asyncContainer->mMail->mSubject.convert(BSTRType_Unicode16);
				// Note: asyncContainer->mRequestId is the mailId...
				gChatMessageLib->sendChatPersistantMessagetoClient(receiver->getClient(),asyncContainer->mMail, asyncContainer->mRequestId,asyncContainer->mMailCounter,MailStatus_New);
			}

			//when we send a system Mail there will be no sender
			if (asyncContainer->mSender != NULL)
			{
				//NEVER END A MESSAGE AND THEN DONT SEND IT !!!!!!!!!!!!!!!!!!!!!!!!!!!
				// send status to sender
				gMessageFactory->StartMessage();
				gMessageFactory->addUint32(opChatOnSendPersistentMessage);
				gMessageFactory->addUint32(0);
				gMessageFactory->addUint32(asyncContainer->mMailCounter);
				newMessage = gMessageFactory->EndMessage();


				((asyncContainer->mSender)->getClient())->SendChannelA(newMessage,((asyncContainer->mSender)->getClient())->getAccountId(),CR_Client,3);
			}

			SAFE_DELETE(asyncContainer->mMail);

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ChatQuery_MailById:
		{
			Mail mail = Mail();
			mail.mSubject.setLength(512);
			mail.mText.setLength(8192);

			uint64 count = result->getRowCount();
			assert(count == 1);

			result->GetNextRow(mMailBinding,&mail);

			memcpy(mail.mAttachments.getRawData(),mail.mAttachmentRaw,mail.mAttachmentSize);

			mail.mAttachments.setLength(static_cast<uint16>(mail.mAttachmentSize >> 1));
			mail.mSubject.convert(BSTRType_Unicode16);
			mail.mText.convert(BSTRType_Unicode16);

			gChatMessageLib->sendChatPersistantMessagetoClient(asyncContainer->mClient,&mail);

			mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE chat_mail SET status = 1 WHERE id=%u",mail.mId);
		}
		break;

		case ChatQuery_MailHeaders:
		{
			Mail		mail;
			uint64		count = result->getRowCount();

			// Update client with all mail.
			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(mMailHeaderBinding,&mail);
				mail.mSubject.convert(BSTRType_Unicode16);

				// gLogger->logMsg("ChatQuery_MailHeaders",MSG_NORMAL);

				if(!mail.mStatus)
				{
					gChatMessageLib->sendChatPersistantMessagetoClient(asyncContainer->mClient,&mail, mail.mId,1,MailStatus_New);
				}
				else
				{
					gChatMessageLib->sendChatPersistantMessagetoClient(asyncContainer->mClient,&mail, mail.mId,1,MailStatus_Read);
				}
			}
		}
		break;

		case ChatQuery_PlayerFriends:
		{
			string	name;
			Player*	player = asyncContainer->mReceiver;

			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,64);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&name);
				name.toLower();
				player->getFriendsList()->insert(std::make_pair(name.getCrc(),name.getAnsi()));
			}

			mDatabase->DestroyDataBinding(binding);

			// check for online friends


			// We are actually sending this info from CharacterLoginHandler::handleDispatchMessage at the opCmdSceneReady event.
			// updateFriendsOnline(player,true);

			// We should not send any info to self about all the friend in MY friends list.
			// checkOnlineFriends(player);

			// query ignoreslist
			ChatAsyncContainer* asContainer = new ChatAsyncContainer(ChatQuery_PlayerIgnores);
			asContainer->mClient	= asyncContainer->mClient;
			asContainer->mReceiver	= asyncContainer->mReceiver;

			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT characters.firstname FROM chat_ignorelist "
														"INNER JOIN characters ON (chat_ignorelist.ignore_id = characters.id) "
														"WHERE (chat_ignorelist.character_id = %"PRIu64")",asContainer->mReceiver->getCharId());
		}
		break;

		case ChatQuery_PlayerIgnores:
		{
			string	name;
			Player*	player = asyncContainer->mReceiver;

			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,64);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&name);
				name.toLower();
				// gLogger->logMsgF(" %s AddIgnore [%s]",MSG_NORMAL,player->getName().getAnsi(), name.getAnsi());
				player->getIgnoreList()->insert(std::make_pair(name.getCrc(),name.getAnsi()));
			}

			mDatabase->DestroyDataBinding(binding);

			// Update channel info.
			ChatAsyncContainer* asContainer = new ChatAsyncContainer(ChatQuery_CharChannels);
			asContainer->mClient = asyncContainer->mClient;

			Player* currentPlayer = getPlayerByAccId(asyncContainer->mClient->getAccountId());

			mDatabase->ExecuteSqlAsync(this, asContainer,"SELECT channel_id"
											" FROM chat_char_channels"
											" WHERE	(character_id = %"PRIu64")", currentPlayer->getCharId());

		}
		break;

		case ChatQuery_CharChannels:
		{
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);
			uint32 roomId;

			uint64 count = result->getRowCount();

			// gLogger->logMsg("Adding user to channel(s):", MSG_NORMAL);
			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&roomId);
				Channel* channel = getChannelById(roomId);
				Player* player = getPlayerByAccId(asyncContainer->mClient->getAccountId());
				if(player == NULL)
				{
					gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL, asyncContainer->mClient->getAccountId());
					continue;
				}
				// gLogger->logMsgF("Adding %s to %s", MSG_NORMAL, player->getName().getAnsi(), channel->getName().getAnsi());
				ChatAvatarId* avatar = new ChatAvatarId();
				avatar->setPlayer(player);
				avatar->setGalaxy(mGalaxyName);

				//If I'm banned or not invited to a private channel then stay out.
				if (channel->isBanned(avatar->getLoweredName()))
				{
					gLogger->logMsg("Player was banned\n");
					continue;
				}
				if (channel->isPrivate())
				{
					if (!channel->isInvited(avatar->getLoweredName()))
					{
						gLogger->logMsg("Player was not invited to private channel\n");
						continue;
					}
				}
				channel->addUser(avatar);
				gChatMessageLib->sendChatQueryRoomResults(asyncContainer->mClient, channel, 0);
				gChatMessageLib->sendChatOnEnteredRoom(asyncContainer->mClient, avatar, channel, 0);
			}
			mDatabase->DestroyDataBinding(binding);
		}
		break;


		case ChatQuery_QueryFirstName:
		{
			string	name;
			//Player*	mReceiver = asyncContainer->mReceiver;

			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,64);
			result->GetNextRow(binding,&name);

			//_processPersistentMessageToServer(asyncContainer->mMail,asyncContainer->mClient);
			_PersistentMessagebySystem(asyncContainer->mMail,asyncContainer->mClient,name);

			mDatabase->DestroyDataBinding(binding);
		}
		break;

		case ChatQuery_Channels:
		{
			_loadChannels(result);
		}
		break;

		case ChatQuery_Moderators:
			{
				uint64 count = result->getRowCount();

				DataBinding* binding = mDatabase->CreateDataBinding(1);
				binding->addField(DFT_bstring, 0, 33);

				for (uint i = 0; i < count; i++)
				{
					string* name = new string();
					result->GetNextRow(binding, name);
					asyncContainer->mChannel->addModerator(name);
					delete name;
				}

				mDatabase->DestroyDataBinding(binding);
			}
			break;

		case ChatQuery_Banned:
			{
				uint64 count = result->getRowCount();

				DataBinding* binding = mDatabase->CreateDataBinding(1);
				binding->addField(DFT_bstring, 0, 33);

				for (uint i = 0; i < count; i++)
				{
					string* name = new string();
					result->GetNextRow(binding, name);
					asyncContainer->mChannel->banUser(name);
					delete name;
				}
				mDatabase->DestroyDataBinding(binding);
			}

		case ChatQuery_Invited:
			{
				uint64 count = result->getRowCount();

				DataBinding* binding = mDatabase->CreateDataBinding(1);
				binding->addField(DFT_bstring, 0, 33);

				for (uint i = 0; i < count; i++)
				{
					string* name = new string();
					result->GetNextRow(binding, name);
					asyncContainer->mChannel->addInvitedUser(name);
					delete name;
				}

				mDatabase->DestroyDataBinding(binding);
			}
			break;

		case ChatQuery_AddChannel:
			{
				uint64 count = result->getRowCount();
				assert(count == 1); //There should only be one Id

				/* Player* player = */getPlayerByAccId(asyncContainer->mClient->getAccountId());
				DataBinding* binding = mDatabase->CreateDataBinding(1);
				binding->addField(DFT_uint32, 0, 4, 0);

				uint32 id;
				result->GetNextRow(binding, &id);
				mDatabase->DestroyDataBinding(binding);
				asyncContainer->mChannel->setId(id); //Set the id

				uint32 crc = asyncContainer->mChannel->getName().getCrc();

				gChatMessageLib->sendChatOnCreateRoom(asyncContainer->mClient, asyncContainer->mChannel, asyncContainer->mRequestId);

				mChannelMap.insert(std::make_pair(id, asyncContainer->mChannel));
				mChannelNameMap.insert(std::make_pair(crc, asyncContainer->mChannel));
				mvChannels.push_back(asyncContainer->mChannel);
			}
			break;

		case ChatQuery_PlanetNames:
		{
			string			tmp;
			DataBinding*	nameBinding = mDatabase->CreateDataBinding(1);
			nameBinding->addField(DFT_bstring,0,255,0);

			uint64 rowCount = result->getRowCount();

			for(uint64 i = 0;i < rowCount;i++)
			{
				result->GetNextRow(nameBinding,&tmp);
				mvPlanetNames.push_back(BString(tmp.getAnsi()));

			}

			mDatabase->DestroyDataBinding(nameBinding);
		}
		break;

		default:break;
	}

	SAFE_DELETE(asyncContainer);
}
//======================================================================================================================

void ChatManager::_processClusterClientConnect(Message* message,DispatchClient* client)
{
	uint32 accountId = message->getAccountId();
	uint64 charId = message->getUint64();
	uint32 planetId = message->getUint32();

	Player* player = new Player(charId,client,planetId);

	gLogger->logMsgF("Connecting account %u with player id %"PRIu64"", MSG_NORMAL, client->getAccountId(), charId);

	mPlayerAccountMap.insert(std::make_pair(accountId,player));
	mPlayerList.push_back(player);

	/*
	// Query friendslist
	ChatAsyncContainer* asContainer = new ChatAsyncContainer(ChatQuery_PlayerFriends);
	asyncContainer->mClient = client;
	asContainer->mReceiver	= asyncContainer->mReceiver;

	mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT characters.firstname FROM chat_friendlist "
												"INNER JOIN characters ON (chat_friendlist.friend_id = characters.id) "
												"WHERE (chat_friendlist.character_id = %"PRIu64")",asContainer->mReceiver->getCharId());


	*/
	ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_Player);
	asyncContainer->mClient = client;

	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT firstname,lastname FROM characters WHERE id=%"PRIu64"",charId);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opChatOnConnectAvatar);
	Message* newMessage = gMessageFactory->EndMessage();

	client->SendChannelA(newMessage, client->getAccountId(), CR_Client, 1);
}

//======================================================================================================================

void ChatManager::_processClusterClientDisconnect(Message* message,DispatchClient* client)
{
	Player* player;
	PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

	if (accIt != mPlayerAccountMap.end())
	{
		player = (*accIt).second;
		updateFriendsOnline(player,false);
	}
	else
	{
		gLogger->logMsgF("Error finding player in player account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	// Question:
	// is it safe to send a message in the player network client here??
	// Answer: It will not get there... but we can try other clients.

	// remove us from groups
	if (player->getGroupId() != 0)
	{
		GroupObject* group = gGroupManager->getGroupById(player->getGroupId());
		if(group != NULL)
		{
			if(player->getGroupMemberIndex() == 0x0FFFF) // means it was invited but didnt join.
			{
				group->removeTempMember(player);
			}
			else
			{
				group->removeMember(player);
			}
		}
	}

	// Remove us from planet channel
	Channel* channel = getChannelById(player->getPlanetId() + 23);
	if (channel != NULL)
	{
		// Test in debug purpose, sometimes the player are removed and we don't wanna blow the server..
		ChatAvatarId* ava = channel->findUser(player); // findUser() converts toLower.
		if (ava != NULL)
		{
			gLogger->logMsgF("See you soon %s ",MSG_NORMAL, player->getName().getAnsi());
			gChatMessageLib->sendChatOnLeaveRoom(client, channel->findUser(player->getName().getAnsi()), channel, 0);
			channel->removeUser(player);
		}
		else
		{
			gLogger->logMsgF("ChatManager:: Can't find player %s in planet channel",MSG_NORMAL, player->getName().getAnsi());
		}
	}
	else
	{
		// This is the normal path for the Tutorial, it has no planet channel.
		gLogger->logMsgF("Can't find channel for planet %u",MSG_NORMAL, player->getPlanetId());

		// We cant just return here, we need to continue and remove the player from the account map etc below.
	}
	// channel->removeUser(player);

	// Remove user from user created channels.
	ChannelList::iterator channelIt = mvChannels.begin();
	// gLogger->logMsgF("Removing user %s from channel(s): ",MSG_NORMAL, player->getName().getAnsi());

	while (channelIt != mvChannels.end())
	{
		// User in channel?
		ChatAvatarId* av = (*channelIt)->findUser(player);
		if (av)
		{
			// No idea to try and talk to own client here...
			// TODO: fix talk to self...
			// gLogger->logMsgF("%s",MSG_NORMAL, (*channelIt)->getName().getAnsi());

			// Try to talk with all other clients in same channel.
			gChatMessageLib->sendChatOnLeaveRoom(client, av, (*channelIt), 0);
			(*channelIt)->removeUser(player);
		}
		channelIt++;
	}

	PlayerList::iterator listIt = mPlayerList.begin();
	while(listIt != mPlayerList.end())
	{
		if((*listIt) == player)
		{
			mPlayerList.erase(listIt);
			break;
		}
		++listIt;
	}

	// Continue the clean up here.
	mPlayerAccountMap.erase(accIt);


	// Remove user from server.
	PlayerNameMap::iterator	nameIt = mPlayerNameMap.find(player->getKey());
	if (nameIt != mPlayerNameMap.end())
	{
		mPlayerNameMap.erase(nameIt);
		SAFE_DELETE(player);
	}
	else
	{
		gLogger->logMsgF("Error removing player %s from name map",MSG_NORMAL,player->getName().getAnsi());
	}

	// For test
	// gLogger->logMsgF("Player removed from name map",MSG_NORMAL);
}

//======================================================================================================================

// The client is loaded, after a connect or zone transfer.
void ChatManager::_processWhenLoaded(Message* message,DispatchClient* client)
{
	gLogger->logMsg("ChatManager::_processWhenLoaded",MSG_NORMAL);

	// enter planet channel
	Player* player = getPlayerByAccId(client->getAccountId());

	if (player)
	{
		if (player->getAddPending())
		{
			player->setAddPending(false);

			Channel* channel = getChannelById(player->getPlanetId() + 23);
			if (channel == NULL)
			{
				gLogger->logMsgF("No channel for planet %u\n", MSG_NORMAL, player->getPlanetId());
			}
			else
			{
				assert(channel != NULL);
				ChatAvatarId* avatar = new ChatAvatarId();
				avatar->setGalaxy(mGalaxyName);
				avatar->setPlayer(player);

				channel->addUser(avatar);
				gChatMessageLib->sendChatOnEnteredRoom(client, avatar, channel, 0);
			}

			PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());
			if (accIt != mPlayerAccountMap.end())
			{
				// query mail headers
				ChatAsyncContainer* asContainer = new ChatAsyncContainer(ChatQuery_MailHeaders);
				asContainer->mClient = client;
				asContainer->mReceiver = (*accIt).second;

				// gLogger->logMsg("Doing database request for mailheaders",MSG_NORMAL);

				// Update friends list
				updateFriendsOnline(asContainer->mReceiver,true);

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT chat_mail.id,chat_mail.from,chat_mail.subject,chat_mail.status,chat_mail.time"
															" FROM chat_mail"
															" WHERE	(chat_mail.to = %"PRIu64")",asContainer->mReceiver->getCharId());
			}
		}
		GroupObject* group = gGroupManager->getGroupById(player->getGroupId());
		if (group)
		{
			group->sendUpdate(player);
		}
	}
}
//======================================================================================================================


void ChatManager::_processZoneTransfer(Message* message,DispatchClient* client)
{
	gLogger->logMsg("_processZoneTransfer");

	uint32 planetId = message->getUint32();

	Player* player;
	//Channel* channel;
	PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(message->getAccountId());

	if (accIt != mPlayerAccountMap.end())
	{
		player = (*accIt).second;
		Channel* channel = getChannelById(player->getPlanetId() + 23);
		if (channel == NULL)
		{
			gLogger->logMsgF("No channel for depature planet %u", MSG_NORMAL, player->getPlanetId());
		}
		else
		{
			assert(channel != NULL);
			ChatAvatarId* avatar = channel->findUser(player->getName());
			if(!avatar)
			{
				gLogger->logMsgF("ChatManager::_processZoneTransfer No avatar \n", MSG_NORMAL);

			}
			else
				gChatMessageLib->sendChatOnLeaveRoom(client, avatar, channel, 0);

			channel->removeUser(player);
		}
		player->setPlanetId(planetId);

		channel = getChannelById(planetId + 23);
		if (channel == NULL)
		{
			gLogger->logMsgF("No channel for destination planet %u\n", MSG_NORMAL, planetId);
		}
		else
		{
			assert(channel != NULL);
			ChatAvatarId* avatar = new ChatAvatarId();
			avatar->setPlayer(player);
			avatar->setGalaxy(mGalaxyName);

			channel->addUser(avatar);
			gChatMessageLib->sendChatOnEnteredRoom(client, avatar, channel, 0);
		}
	}
	else
	{
		gLogger->logMsgF("Error getting player %u",MSG_NORMAL,client->getAccountId());
		return;
	}
}

//======================================================================================================================

void ChatManager::_processRoomlistRequest(Message* message,DispatchClient* client)
{
	// gLogger->logMsg("RoomlistRequest");

	gChatMessageLib->sendChatRoomList(client, &mvChannels);

	Player* player = getPlayerByAccId(client->getAccountId());
	if(player == NULL)
	{
		gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}
}

//======================================================================================================================

void ChatManager::_processCreateRoom(Message* message,DispatchClient* client)
{
	// gLogger->logMsg("CreateRoom");
	string path;
	string title;

	Player* player = getPlayerByAccId(client->getAccountId());
	string* playername = new BString(BSTRType_ANSI, player->getName().getLength());
	strcpy(playername->getAnsi(), player->getName().getAnsi());

	uint8 publicFlag = message->getUint8();
	uint8 moderatedFlag = message->getUint8();
	message->getUint16(); //null ascii string
	message->getStringAnsi(path);
	message->getStringAnsi(title);
	uint32 requestId = message->getUint32();

	uint32 index = 5 + mGalaxyName.getLength();
	string modpath;
	path.substring(modpath, static_cast<uint16>(index), path.getLength());

	// modpath.toLower();

	gLogger->logMsgF("Attempting to create channel %s at %s\n", MSG_NORMAL, title.getAnsi(), modpath.getAnsi());

	ChannelList::iterator iter = mvChannels.begin();
	while (iter != mvChannels.end())
	{
		if (Anh_Utils::cmpistr((*iter)->getName().getAnsi(), modpath.getAnsi()) == 0)
		{
			gLogger->logMsgF("Channel %s already exist\n", MSG_NORMAL, modpath.getAnsi());
			delete playername;
			return;
		}
		iter++;
	}

	// Lowercase...
	playername->toLower();

	Channel* channel = new Channel();
	channel->setName(modpath);
	channel->setTitle(title);
	channel->setPrivate((publicFlag == 0) ? 1 : 0);
	channel->setModerated(moderatedFlag);

	ChatAvatarId* avatar = new ChatAvatarId();
	avatar->setPlayer(player);
	avatar->setGalaxy(mGalaxyName);
	channel->setCreator(avatar);

	// Don't let two object point to the same data...
	avatar = new ChatAvatarId();
	avatar->setPlayer(player);
	avatar->setGalaxy(mGalaxyName);
	channel->setOwner(avatar);

	channel->setGalaxy(mGalaxyName);
	channel->addModerator(playername);
	if (channel->isPrivate())
		channel->addInvitedUser(playername);

	ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_AddChannel);
	asyncContainer->mClient = client;
	asyncContainer->mRequestId = requestId;
	asyncContainer->mChannel = channel;
	// Lowercase mDatabase->ExecuteSqlAsync(this, asyncContainer, "SELECT sf_CreateChannel('%s', %u, %u, '%s', '%s');", modpath.getAnsi(), channel->isPrivate(), moderatedFlag, player->getName().getAnsi(), title.getAnsi());
	int8 sql[128];
	mDatabase->Escape_String(sql, playername->getAnsi(), playername->getLength());

	int8 sql_2[128];
	mDatabase->Escape_String(sql_2, title.getAnsi(), title.getLength());
	mDatabase->ExecuteSqlAsync(this, asyncContainer, "SELECT sf_CreateChannel('%s', %u, %u, '%s', '%s');", modpath.getAnsi(), channel->isPrivate(), moderatedFlag, sql /* playername->getAnsi() */, sql_2 /* title.getAnsi()*/ );

	// TEST
	gLogger->logMsgF("Channel %s created at %s\n", MSG_NORMAL, title.getAnsi(), modpath.getAnsi());

	delete playername;
}

//======================================================================================================================

void ChatManager::_processDestroyRoom(Message* message,DispatchClient* client)
{
	gLogger->logMsg("DestroyRoom");
	uint32 roomId = message->getUint32();
	/* uint32 requestId = */message->getUint32();

	Channel* channel = getChannelById(roomId);
	if (channel == NULL)
	{
		gLogger->logMsgF("No channel for room %u", MSG_NORMAL, roomId);
		return;
	}
	assert(channel != NULL);

	// This is me destroying the channel
	string playername = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());

	// Lowercase...
	playername.toLower();

	// Request by Moderator or Owner?
	if ((!channel->isModerator(playername)) && (!channel->isOwner(playername)))
	{
		gChatMessageLib->sendChatOnDestroyRoom(client, channel, 16);
		return;
	}

	gLogger->logMsgF("Player %s destroying channel %s\n", MSG_NORMAL, playername.getAnsi(), channel->getName().getAnsi());
	ChatAvatarId* avatar = channel->findUser(playername);
	if (avatar == NULL)
	{
	}
	else
	{
		// Remove user from channel...before destruction.
		assert(avatar != NULL);
		gChatMessageLib->sendChatOnLeaveRoom(client, avatar, channel, 0);
	}

	// Update all users that this channel is gone. And I mean ALL...
	PlayerList::iterator listIt = mPlayerList.begin();
	while(listIt != mPlayerList.end())
	{
		gLogger->logMsgF("Channel %s gone for %s\n", MSG_NORMAL, channel->getName().getAnsi(), (*listIt)->getName().getAnsi());
		gChatMessageLib->sendChatOnDestroyRoom((*listIt)->getClient(), channel, 0);
		++listIt;
	}

	// Clear the channel... Why don't ~channel() fix this ????????
	channel->clearChannel();

	// Clean up the channel data used.
	ChannelList::iterator iter = mvChannels.begin();
	while (iter != mvChannels.end())
	{
		if (*iter == channel)
		{
			mvChannels.erase(iter);
			break;
		}
		++iter;
	}
	ChatManager::unregisterChannel(channel);

	ChannelNameMap::iterator nameIter = mChannelNameMap.find(channel->getName().getCrc());
	{
		mChannelNameMap.erase(nameIter);
	}

	// ChatMessageLib->sendChatQueryRoomResults(client, channel, 0);
	delete(channel);

	// If we delete the channel, we need to delete all related objects too.
	mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels WHERE id = %u", roomId);

	// Moderators, invited, banned and characters.
	mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_moderators WHERE channel_id = %u", roomId);
	mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_invited WHERE channel_id = %u;", roomId);
	mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_banned WHERE channel_id = %u;", roomId);
	mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_char_channels WHERE channel_id = %u;", roomId);
}

//======================================================================================================================

void ChatManager::_processRoomQuery(Message* message,DispatchClient* client)
{
	// gLogger->logMsg("RoomQuery");
	string roomname;
	string path;

	uint32 requestId = message->getUint32();
	message->getStringAnsi(path);

	uint32 index = 5 + mGalaxyName.getLength();
	path.substring(roomname, static_cast<uint16>(index), path.getLength());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->logMsgF("No channel for room %s", MSG_NORMAL, roomname.getAnsi());
		return;
	}
	assert(channel != NULL);
	gChatMessageLib->sendChatQueryRoomResults(client, channel, requestId);
}

//======================================================================================================================

void ChatManager::_processRoomMessage(Message* message,DispatchClient* client)
{
	gLogger->logMsg("_processRoomMessage");
}

//======================================================================================================================
//
// Incoming tell
//
void ChatManager::_processInstantMessageToCharacter(Message* message,DispatchClient* client)
{
	// gLogger->logMsg("Instant message");

	string mainCategory; // "SWG"
	string serverName;   // (galaxy name)
	string targetName;   // recipient
	targetName.setLength(256);
	string msgText;
	msgText.setType(BSTRType_Unicode16);
	msgText.setLength(512);
	uint32 tellId = 0;
	uint32 targetStatus = 0;
	Player* sender = NULL;
	Player* receiver = NULL;

	Message* newMessage; // our replies

	// get our sender
	sender = getPlayerByAccId(client->getAccountId());

	if(sender == NULL)
	{
		gLogger->logMsgF("Error finding sender %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	message->getStringAnsi(mainCategory);
	message->getStringAnsi(serverName);
	message->getStringAnsi(targetName);
	targetName.toLower();
	message->getStringUnicode16(msgText);
	message->getUint32();
	tellId = message->getUint32();

	receiver = getPlayerByName(targetName);

	if(receiver == NULL)
	{
		//4 is error code for player not online
		targetStatus = 4; //TODO: Magic Number - Someone please fix?
	}

	if(receiver != NULL)
	{
		gMessageFactory->StartMessage();
		gMessageFactory->addUint32(opChatInstantMessageToClient);
		gMessageFactory->addString(mainCategory);
		gMessageFactory->addString(serverName);
		gMessageFactory->addString(sender->getName());
		gMessageFactory->addString(msgText);
		gMessageFactory->addUint32(0);
		newMessage = gMessageFactory->EndMessage();

		(receiver->getClient())->SendChannelA(newMessage,(receiver->getClient())->getAccountId(),CR_Client,4);
	}

	// send status to sender
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opChatOnSendInstantMessage);
	gMessageFactory->addUint32(targetStatus);
	gMessageFactory->addUint32(tellId);
	newMessage = gMessageFactory->EndMessage();

	client->SendChannelA(newMessage,client->getAccountId(),CR_Client,3);
}

//======================================================================================================================

void ChatManager::_processEnterRoomById(Message* message,DispatchClient* client)
{
	// gLogger->logMsg("EnterRoomById");

	uint32 requestId = message->getUint32();
	uint32 roomId = message->getUint32();

	Channel* channel = getChannelById(roomId);
	if (channel == NULL)
	{
		gLogger->logErrorF("chat","No channel for room %u", MSG_NORMAL, roomId);
		return;
	}
	Player* player = getPlayerByAccId(client->getAccountId());
	if (player == NULL)
	{
		gLogger->logErrorF("chat","No player for account %u", MSG_NORMAL, client->getAccountId());
		return;
	}

	ChatAvatarId* avatar = new ChatAvatarId();
	avatar->setPlayer(player);
	avatar->setGalaxy(mGalaxyName);

	// If player already in channel, abort.
	ChatAvatarIdList::iterator iter = channel->getUserList()->begin();

	while (iter != channel->getUserList()->end())
	{
		if ((*iter)->getLoweredName().getCrc() == player->getKey())
		{
			gChatMessageLib->sendChatOnEnteredRoom(client, avatar, channel, requestId);
			gLogger->logMsgF("Player %s already in room %s\n", MSG_NORMAL, player->getName().getAnsi(),channel->getName().getAnsi());
			return;
		}
		++iter;
	}

	if (channel->isBanned(avatar->getLoweredName()))
	{
		// You cannot join '%TU (room name)' because you are not invted to the room
		gChatMessageLib->sendChatFailedToEnterRoom(client, avatar, 16, channel, requestId);
		gLogger->logMsg("Player was banned\n");
		return;
	}

	if (channel->isPrivate())
	{
		if (channel->isInvited(avatar->getLoweredName()))
		{
			gLogger->logMsg("Channel was private\n");
		}
		else
		{
			// You cannot join '%TU (room name)' because you are not invted to the room
			gChatMessageLib->sendChatFailedToEnterRoom(client, avatar, 16, channel, requestId);
			gLogger->logMsg("Player was not invited\n");
			return;
		}
	}
	// Test Eruptor
	// If we have not been fully loaded, the channel list is not complete.
	// Removed again, since we screw up client sync and we end up see two duplicates of"player" in the same channel.

	/*
	if (!player->getAddPending())
	{
		channel->addUser(avatar);
		mDatabase->ExecuteSqlAsync(NULL, NULL, "INSERT INTO chat_char_channels VALUES (%"PRIu64", %u);", player->getCharId(), channel->getId());
		gChatMessageLib->sendChatOnEnteredRoom(client, avatar, channel, requestId);
		gLogger->logMsg("Player added to channel");
	}
	else
	{
		gLogger->logMsg("Player NOT added to channel");
	}
	*/
	channel->addUser(avatar);
	gChatMessageLib->sendChatOnEnteredRoom(client, avatar, channel, requestId);
	mDatabase->ExecuteSqlAsync(NULL, NULL, "INSERT INTO chat_char_channels VALUES (%"PRIu64", %u);", player->getCharId(), channel->getId());
	// gLogger->logMsg("Player added to channel");
}

//======================================================================================================================

void ChatManager::_processSendToRoom(Message* message,DispatchClient* client)
{
	gLogger->logMsg("_processSendToRoom");

	Player*	player = getPlayerByAccId(client->getAccountId());

	if (player == NULL)
	{
		gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	string msg;
	msg.setType(BSTRType_Unicode16);
	msg.setLength(512);

	message->getStringUnicode16(msg);
	uint32 requestId = message->getUint32(); // unknown
	uint32 channelId = message->getUint32();

	Channel* channel = getChannelById(channelId);
	if (channel == NULL)
	{
		gLogger->logMsgF("No channel with id %u", MSG_NORMAL, channelId);
		gChatMessageLib->sendChatOnSendRoomMessage(client, 1, requestId);	// Error code 1 will give the default error message.
		return;
	}
	assert(channel != NULL);

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	string sender = BString(player->getName().getAnsi());
	string realSenderName = sender;
	sender.toLower();

//	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
#else
	// Lowercase
	realSenderName.toLower();
#endif

	if (channel->isModerated())
	{
		if ((!channel->isModerator(sender)) && (!channel->isOwner(sender)))
		{
			// gLogger->logMsgF("Not a moderator", MSG_NORMAL);

			gChatMessageLib->sendChatOnSendRoomMessage(client, 9, requestId);
			return;
		}
	}
	gChatMessageLib->sendChatOnSendRoomMessage(client, 0, requestId);
	gChatMessageLib->sendChatRoomMessage(channel, mGalaxyName, realSenderName, msg);
}

//======================================================================================================================

void ChatManager::_processAddModeratorToRoom(Message* message,DispatchClient* client)
{
	gLogger->logMsg("Add moderator to room");
	string playerName;
	string roompath;
	string roomname;
	string game;
	string server;

	message->getStringAnsi(game);
	message->getStringAnsi(server);
	message->getStringAnsi(playerName);
	message->getStringAnsi(roompath);
	uint32 requestId = message->getUint32();

	uint32 index = 5 + mGalaxyName.getLength();
	roompath.substring(roomname, static_cast<uint16>(index), roompath.getLength());

	gLogger->logMsgF("Channel is %s", MSG_NORMAL, roomname.getAnsi());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->logMsgF("No channel for room %s", MSG_NORMAL, roomname.getAnsi());
		return;
	}
	assert(channel != NULL);


	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	string sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	string realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	// Get real first name.
	string* newName = getFirstName(playerName);
	string realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->logMsgF("No player with name %s found\n", MSG_NORMAL, playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	string realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->logMsgF("No player with name %s found", MSG_NORMAL, playerName.getAnsi());
	}
#endif
	// We check in logical order, even if we know that playername is not valid.
	if (!channel->isModerated())
	{
		// Channel is not moderated.
		gLogger->logMsg("Channel is not moderated", MSG_NORMAL);
		errorCode = 9;
	}
	else if ((!channel->isModerator(sender)) && (!channel->isOwner(sender)))
	{
		errorCode = 16;
		gLogger->logMsgF("%s is not owner or moderated in channel %s.", MSG_NORMAL,realSenderName.getAnsi(), roomname.getAnsi());
		// gChatMessageLib->sendChatFailedToAddMod(client, mGalaxyName, sender, playername, channel, 16, requestId);
		// return;
	}
	// If we have invalid player name, we don't need the following checks.
	else if ((errorCode == 0) && (channel->isModerator(playerName)))
	{
		gLogger->logMsgF("%s is already a moderator in channel %s.", MSG_NORMAL, realPlayerName.getAnsi(), roomname.getAnsi());
		// gChatMessageLib->sendChatFailedToAddMod(client, mGalaxyName, sender, playername, channel, 1, requestId);
		errorCode = 1;
		// return;
	}
	if (errorCode != 0)
	{
		gChatMessageLib->sendChatFailedToAddMod(client, mGalaxyName, realSenderName, realPlayerName, channel, errorCode, requestId);
	}
	else
	{
		channel->addModerator(&playerName);
		int8 sql[128];
		mDatabase->Escape_String(sql, realPlayerName.getAnsi(), realPlayerName.getLength());

		mDatabase->ExecuteSqlAsync(NULL, NULL, "INSERT INTO chat_channels_moderators VALUES (%u, '%s');", channel->getId(), sql /* realPlayerName.getAnsi() */);
		gChatMessageLib->sendChatOnAddModeratorToRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, requestId);
	}
#ifdef DISP_REAL_FIRST_NAME
	// delete realPlayerName;
#endif
}

//======================================================================================================================

void ChatManager::_processInviteAvatarToRoom(Message* message,DispatchClient* client)
{
	gLogger->logMsg("Invite avatar to room");
	string nameOfInvitedPlayer;
	string roompath;
	string roomname;
	string game;
	string server;
	string playerName;

	message->getStringAnsi(game);
	message->getStringAnsi(server);
	message->getStringAnsi(playerName);
	message->getStringAnsi(roompath);
	uint32 requestId = message->getUint32();

	uint32 index = 5 + mGalaxyName.getLength();
	roompath.substring(roomname, static_cast<uint16>(index), roompath.getLength());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->logMsgF("No channel for room %s", MSG_NORMAL, roomname.getAnsi());
		return;
	}
	assert(channel != NULL);

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	string sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	string realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	string* newName = getFirstName(playerName);
	string realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->logMsgF("No player with name %s found\n", MSG_NORMAL, playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	string realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->logMsgF("No player with name %s found", MSG_NORMAL, playerName.getAnsi());
	}
#endif
	// We check in logical order, even if we know that playername is not valid.
	// Private channel?
	if (!channel->isPrivate())
	{
		errorCode = 9;
	}
	// Request by Moderator or Owner?
	else if ((!channel->isModerator(sender)) && (!channel->isOwner(sender)))
	{
		errorCode = 16;
	}
	else if (errorCode == 0) // then playername is valid...
	{
		// Player banned?
		if (channel->isBanned(playerName))
		{
			// General failure
			errorCode = 1;
		}
		else if (channel->isInvited(playerName))
		{
			// General failure
			errorCode = 1;
		}
	}
	if (errorCode != 0)
	{
		gChatMessageLib->sendChatFailedToInvite(client, mGalaxyName, realSenderName, realPlayerName, channel, errorCode, requestId);
	}
	else
	{
		gLogger->logMsgF("%s has invited %s to join channel %s",MSG_NORMAL,realSenderName.getAnsi(), realPlayerName.getAnsi(), roomname.getAnsi());
		channel->addInvitedUser(&playerName);
		int8 sql[128];
		mDatabase->Escape_String(sql, playerName.getAnsi(), playerName.getLength());

		mDatabase->ExecuteSqlAsync(NULL, NULL, "INSERT INTO chat_channels_invited VALUES (%u, '%s');", channel->getId(), sql /* playerName.getAnsi() */);
		gLogger->logMsgF("Player %s is added to database for invited",MSG_NORMAL, playerName.getAnsi());

		gChatMessageLib->sendChatOnInviteToRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, requestId);
		gChatMessageLib->sendChatQueryRoomResults(client, channel, 0);
	}
#ifdef DISP_REAL_FIRST_NAME
	// delete realPlayerName;
#endif
}

//======================================================================================================================

void ChatManager::_processUninviteAvatarFromRoom(Message* message, DispatchClient* client)
{
	gLogger->logMsg("Uninvite avatar from room");
	string nameOfInvitedPlayer;
	string roompath;
	string roomname;
	string game;
	string server;
	string playerName;

	message->getStringAnsi(game);
	message->getStringAnsi(server);
	message->getStringAnsi(playerName);
	message->getStringAnsi(roompath);

	uint32 requestId = message->getUint32();

	uint32 index = 5 + mGalaxyName.getLength();
	roompath.substring(roomname, static_cast<uint16>(index), roompath.getLength());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->logMsgF("No channel for room %s", MSG_NORMAL, roomname.getAnsi());
		return;
	}
	assert(channel != NULL);

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	string sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	string realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	// Get real first name.
	string* newName = getFirstName(playerName);
	string realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->logMsgF("No player with name %s found\n", MSG_NORMAL, playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	string realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->logMsgF("No player with name %s found", MSG_NORMAL, playerName.getAnsi());
	}
#endif
	// We check in logical order, even if we know that playername is not valid.
	// Private channel?
	if (!channel->isPrivate())
	{
		errorCode = 9;
	}
	// Request by Moderator?
	else if ((!channel->isModerator(sender)) && (!channel->isOwner(sender)))
	{
		errorCode = 16;
	}
	// We are not allowed to uninvite the owner.
	else if (channel->isOwner(playerName))
	{
		// General failure
		errorCode = 1;
	}
	else if ((errorCode == 0) && (!channel->isInvited(playerName)))
	{
		// Allready invited
		errorCode = 13;
	}

	if (errorCode != 0)
	{
		gChatMessageLib->sendChatFailedToUninviteFromRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, errorCode, requestId);
	}
	else
	{
		gLogger->logMsgF("PLayer %s becomes un-invited", MSG_NORMAL, realPlayerName.getAnsi());
		(void)channel->removeInvitedUser(playerName);

		int8 sql[128];
		mDatabase->Escape_String(sql, playerName.getAnsi(), playerName.getLength());

		mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_invited WHERE char_name = '%s' AND channel_id = %u;", sql /* playerName.getAnsi() */, channel->getId());
		gChatMessageLib->sendChatOnUninviteFromRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, requestId);
		gChatMessageLib->sendChatQueryRoomResults(client, channel, 0);
	}
#ifdef DISP_REAL_FIRST_NAME
	// delete realPlayerName;
#endif
}


//======================================================================================================================

void ChatManager::_processRemoveModFromRoom(Message* message,DispatchClient* client)
{
	gLogger->logMsg("Remove Moderator from room");
	string playerName;
	string roompath;
	string roomname;
	string game;
	string server;

	message->getStringAnsi(game);
	message->getStringAnsi(server);
	message->getStringAnsi(playerName);
	message->getStringAnsi(roompath);
	uint32 requestId = message->getUint32();

	uint32 index = 5 + mGalaxyName.getLength();
	roompath.substring(roomname, static_cast<uint16>(index), roompath.getLength());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->logMsgF("No channel for room %s", MSG_NORMAL, roomname.getAnsi());
		return;
	}
	assert(channel != NULL);

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	string sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	string realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	// Get real first name.
	string* newName = getFirstName(playerName);
	string realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->logMsgF("No player with name %s found\n", MSG_NORMAL, playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	string realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->logMsgF("No player with name %s found", MSG_NORMAL, playerName.getAnsi());
	}
#endif
	// We check in logical order, even if we know that playername is not valid.
	if (!channel->isModerated())
	{
		// Channel is not moderated.
		errorCode = 9;
		gLogger->logMsg("Channel is not moderated", MSG_NORMAL);
		// gChatMessageLib->sendChatFailedToRemoveMod(client, mGalaxyName, sender, playername, channel, 9, requestId);
		// return;
	}
	else if ((!channel->isModerator(sender)) && (!channel->isOwner(sender)))
	{
		// Must be owner or moderator to operate the channel.
		errorCode = 16;
		// gChatMessageLib->sendChatFailedToRemoveMod(client, mGalaxyName, sender, playername, channel, 16, requestId);
		// return;
	}
	else if (errorCode == 0) // Target player has a valid name...
	{
		// We are not allowed to remove the owner.
		if (channel->isOwner(playerName))
		{
			// General failure.
			errorCode = 1;
			// gChatMessageLib->sendChatFailedToRemoveMod(client, mGalaxyName, sender, playername, channel, 1, requestId);
			// return;
		}
		else if (!channel->isModerator(playerName)) // Player is not a moderator...
		{
			// General failure.
			errorCode = 1;
			// gChatMessageLib->sendChatFailedToRemoveMod(client, mGalaxyName, sender, playername, channel, 1, requestId);
			// return;
		}
	}
	if (errorCode != 0)
	{
		gChatMessageLib->sendChatFailedToRemoveMod(client, mGalaxyName, realSenderName, realPlayerName, channel, errorCode, requestId);
	}
	else
	{
		(void)channel->removeModerator(playerName);
		int8 sql[128];
		mDatabase->Escape_String(sql, playerName.getAnsi(), playerName.getLength());

		mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_moderators WHERE char_name = '%s' AND channel_id = %u;", sql /* playerName.getAnsi() */, channel->getId());
		gChatMessageLib->sendChatOnRemoveModeratorFromRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, requestId);
	}
#ifdef DISP_REAL_FIRST_NAME
	// delete realPlayerName;
#endif
}

//======================================================================================================================

void ChatManager::_processRemoveAvatarFromRoom(Message* message,DispatchClient* client)
{
	// gLogger->logMsg("Remove avatar from room");
	string playerName;
	string roompath;
	string roomname;
	string game;
	string server;

	message->getStringAnsi(game);
	message->getStringAnsi(server);
	message->getStringAnsi(playerName);
	message->getStringAnsi(roompath);

	uint32 index = 5 + mGalaxyName.getLength();
	roompath.substring(roomname, static_cast<uint16>(index), roompath.getLength());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->logMsgF("No channel for room %s", MSG_NORMAL, roomname.getAnsi());
		return;
	}
	assert(channel != NULL);

	playerName.toLower();
	uint32 errorCode = 0;
	ChatAvatarId* avatar = channel->findUser(playerName);
	if (avatar == NULL)
	{
		// User not in room.
		// errorCode = 4; We can't use it since the Avatar is not defined...
		gLogger->logMsgF("Can't find user %s in channel", MSG_NORMAL, playerName.getAnsi());
	}
	else
	{
		assert(avatar != NULL);
		mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_char_channels WHERE channel_id = %u AND character_id = %"PRIu64";", channel->getId(), avatar->getPlayer()->getCharId());
		gChatMessageLib->sendChatOnLeaveRoom(client, avatar, channel, 0, errorCode);
	}
	channel->removeUser(playerName);
}

//======================================================================================================================

void ChatManager::_processBanAvatarFromRoom(Message* message,DispatchClient* client)
{
	gLogger->logMsg("Ban Avatar from room");
	string playerName;
	string roompath;
	string roomname;
	string game;
	string server;

	message->getStringAnsi(game);
	message->getStringAnsi(server);
	message->getStringAnsi(playerName);
	message->getStringAnsi(roompath);
	uint32 requestId = message->getUint32();

	uint32 index = 5 + mGalaxyName.getLength();
	roompath.substring(roomname, static_cast<uint16>(index), roompath.getLength());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->logMsgF("No channel for room %s", MSG_NORMAL, roomname.getAnsi());
		return;
	}
	assert(channel != NULL);

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	string sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	string realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	// Get real first name.
	string* newName = getFirstName(playerName);
	string realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->logMsgF("No player with name %s found\n", MSG_NORMAL, playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	string realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->logMsgF("No player with name %s found", MSG_NORMAL, playerName.getAnsi());
	}
#endif
	// We check in logical order, even if we know that playername is not valid.

	if ((!channel->isModerator(sender)) && (!channel->isOwner(sender)))
	{
		errorCode = 16;
	}
	else if (errorCode == 0)
	{
		// Don't ban the owner.
		if (channel->isOwner(playerName))
		{
			// General failure.
			errorCode = 1;
		}
		// Player banned?
		else if (channel->isBanned(playerName))
		{
			// General failure
			errorCode = 1;
		}
	}

	if (errorCode != 0)
	{
		gChatMessageLib->sendChatFailedToBan(client, mGalaxyName, realSenderName, realPlayerName, channel, errorCode, requestId);
	}
	else
	{
		gLogger->logMsgF("PLayer %s becomes kicked, banned and un-invited", MSG_NORMAL, playerName.getAnsi());

		// Kick the player if present in channel.
		ChatAvatarId* avatar = channel->findUser(playerName);
		if (avatar)
		{
			mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_char_channels WHERE channel_id = %u AND character_id = %"PRIu64";", channel->getId(), avatar->getPlayer()->getCharId());
			gChatMessageLib->sendChatOnLeaveRoom(client, avatar, channel, 0, errorCode);
			// gChatMessageLib->sendChatQueryRoomResults(client, channel, 0);	// Update clients before we remove the poor banned one.
			channel->removeUser(playerName);
		}

		int8 sql[128];
		// Un-invite player if invited.
		if (channel->isPrivate() && channel->isInvited(playerName))
		{
			(void)channel->removeInvitedUser(playerName);
			mDatabase->Escape_String(sql, playerName.getAnsi(), playerName.getLength());

			mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_invited WHERE char_name = '%s' AND channel_id = %u;", sql /* playerName.getAnsi() */, channel->getId());
			// Removed since it gives un-wanted spam back to client.
			// gChatMessageLib->sendChatOnUninviteFromRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, 0);

			// gChatMessageLib->sendChatQueryRoomResults(client, channel, 0);
		}

		// Get the ban-stick in ready position
		channel->banUser(&playerName);
		// int8 sql[128];
		mDatabase->Escape_String(sql, playerName.getAnsi(), playerName.getLength());
		mDatabase->ExecuteSqlAsync(NULL, NULL, "INSERT INTO chat_channels_banned VALUES (%u, '%s');", channel->getId(), sql /* playerName.getAnsi()*/);
		gChatMessageLib->sendChatOnBanAvatarFromRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, requestId);
		gChatMessageLib->sendChatQueryRoomResults(client, channel, 0);
	}

#ifdef DISP_REAL_FIRST_NAME
	// delete realPlayerName;
#endif
}

//======================================================================================================================

void ChatManager::_processUnbanAvatarFromRoom(Message* message,DispatchClient* client)
{
	// gLogger->logMsg("Unban avatar from room");
	string playerName;
	string roompath;
	string roomname;
	string game;
	string server;

	message->getStringAnsi(game);
	message->getStringAnsi(server);
	message->getStringAnsi(playerName);
	message->getStringAnsi(roompath);
	uint32 requestId = message->getUint32();

	uint32 index = 5 + mGalaxyName.getLength();
	roompath.substring(roomname, static_cast<uint16>(index), roompath.getLength());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->logMsgF("No channel for room %s", MSG_NORMAL, roomname.getAnsi());
		return;
	}
	assert(channel != NULL);

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	string sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	string realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	// Get real first name.
	string* newName = getFirstName(playerName);
	string realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->logMsgF("No player with name %s found\n", MSG_NORMAL, playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	string realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->logMsgF("No player with name %s found", MSG_NORMAL, playerName.getAnsi());
	}
#endif
	// We check in logical order, even if we know that playername is not valid.
	if ((!channel->isModerator(sender)) && (!channel->isOwner(sender)))
	{
		errorCode = 16;
	}
	else if (errorCode == 0)
	{
		// Player not banned?
		if (!channel->isBanned(playerName))
		{
			// General failure
			errorCode = 1;
		}
	}
	if (errorCode != 0)
	{
		gLogger->logMsgF("Can't un-ban player %s", MSG_NORMAL, realPlayerName.getAnsi());
		gChatMessageLib->sendChatFailedToUnban(client, mGalaxyName, realSenderName, realPlayerName, channel, errorCode, requestId);
	}
	else
	{
		(void)channel->unBanUser(playerName);
		int8 sql[128];
		mDatabase->Escape_String(sql, playerName.getAnsi(), playerName.getLength());

		mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_banned WHERE char_name = '%s' AND channel_id = %u;", sql /* playerName.getAnsi() */, channel->getId());
		gChatMessageLib->sendChatOnUnBanAvatarFromRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, requestId);
	}
#ifdef DISP_REAL_FIRST_NAME
	// delete realPlayerName;
#endif
}

//======================================================================================================================

void ChatManager::_processAvatarId(Message* message,DispatchClient* client)
{
	gLogger->logMsg("Avatar Id");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//allows the trademanager to send EMails even if neither sender nor recipient is logged in
//which is necessary for auctions
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void ChatManager::sendSystemMailMessage(Mail* mail,uint64 recipient)
{
	ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_QueryFirstName);
	asyncContainer->mMail = mail;
	asyncContainer->mMailCounter = 1;
	asyncContainer->mReceiverId = recipient;
	asyncContainer->mClient = NULL;

	int8 sql[100];
	sprintf(sql,"SELECT firstname FROM characters WHERE id LIKE %"PRIu64"",recipient);

	mDatabase->ExecuteSqlAsyncNoArguments(this,asyncContainer,sql);
}


//======================================================================================================================
//
// mail sent, checks if target exists, if target is online, sends only subject, contents are stored to db
// and retrieved with RequestPersistentMessage
// TODO: attachments
//


void ChatManager::_processSystemMailMessage(Message* message,DispatchClient* client)
{

	/* uint64 PlayerID = */message->getUint64();
	uint64 ReceiverID = message->getUint64();
	string Sender;
	Sender.setLength(128);
	message->getStringAnsi(Sender);

	string msgSubject;
	msgSubject.setLength(128);
	message->getStringAnsi(msgSubject);

	string msgText(BSTRType_Unicode16,2048);
	message->getStringUnicode16(msgText);

	string attachmentData(BSTRType_Unicode16,2048);
	message->getStringUnicode16(attachmentData);

	Mail* mail = new Mail();
	mail->setSender(Sender);
	mail->setSubject(msgSubject);
	mail->setText(msgText);
	mail->setStatus(MailStatus_New);
	mail->setTime(static_cast<uint32>(time(NULL)));
	mail->setAttachments(attachmentData);


	//ok we will enter now a db call in order to get the recipients name

	ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_QueryFirstName);
	asyncContainer->mMail = mail;
	asyncContainer->mMailCounter = 1;
	asyncContainer->mReceiverId = ReceiverID;
	asyncContainer->mClient = client;

	int8 sql[100];
	sprintf(sql,"SELECT firstname FROM characters WHERE id LIKE %"PRIu64"",ReceiverID);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);


}
void ChatManager::_PersistentMessagebySystem(Mail* mail,DispatchClient* client, string receiverStr)
{
	Player* sender		= NULL;
	Player* receiver	= NULL;
	uint32 mailId		= 1;       // mail count of sender, sent in this session

	// get our receiver

	receiver = getPlayerByName(receiverStr);

	// Doing a change here, if not pleased with that, please fix the mail related addIgnore implementation.
	// Reason for change:
	// We must have the AddIgnore list and we can't be sure that a client that is online when we do out first test
	// also is online when we do the next async sql, so better treat ALL mail receivers as OFFLINE, then we get a persistent behaviour.

	// Update: If receiver IS online we could take the charId and skip one database access.
	// That have to be a TODO for optimizations when mail is stable.

	// Update: Let's go for that optimization. It's also more in line with the original implementation.

	if (receiver != NULL)
	{

		ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_CreateMail);
		asyncContainer->mMail = mail;
		asyncContainer->mSender = sender;
		asyncContainer->mMailCounter = mailId;
		asyncContainer->mReceiverId = receiver->getCharId();

		int8 sql[20000],*sqlPointer;
		int8 footer[64];
		int8 receiverStr[64];
		sprintf(receiverStr,"',%"PRIu64",'",receiver->getCharId());
		sprintf(footer,",%u,%"PRIu32")",(mail->mAttachments.getLength() << 1),mail->mTime);
		sprintf(sql,"SELECT sf_MailCreate('");
		sqlPointer = sql + strlen(sql);
		sqlPointer += mDatabase->Escape_String(sqlPointer,mail->getSender().getAnsi(),mail->getSender().getLength());
		strcat(sql,receiverStr);
		sqlPointer = sql + strlen(sql);
		sqlPointer += mDatabase->Escape_String(sqlPointer,mail->mSubject.getAnsi(),mail->mSubject.getLength());
		*sqlPointer++ = '\'';
		*sqlPointer++ = ',';
		*sqlPointer++ = '\'';
		sqlPointer += mDatabase->Escape_String(sqlPointer,mail->mText.getAnsi(),mail->mText.getLength());
		*sqlPointer++ = '\'';
		*sqlPointer++ = ',';
		*sqlPointer++ = '\'';
		sqlPointer += mDatabase->Escape_String(sqlPointer,mail->mAttachments.getRawData(),(mail->mAttachments.getLength() << 1));
		*sqlPointer++ = '\'';
		*sqlPointer++ = '\0';
		strcat(sql,footer);

		mDatabase->ExecuteSqlAsyncNoArguments(this,asyncContainer,sql);

	}
	else
	{

		ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_CheckCharacter);
		asyncContainer->mMail = mail;
		asyncContainer->mSender = sender;
		asyncContainer->mMailCounter = mailId;

		int8 sql[256],*sqlPointer;
		sprintf(sql,"SELECT id FROM characters WHERE LOWER(firstname) LIKE '");
		sqlPointer = sql + strlen(sql);
		sqlPointer += mDatabase->Escape_String(sqlPointer,receiverStr.getAnsi(),receiverStr.getLength());
		*sqlPointer++ = '\'';
		*sqlPointer++ = '\0';

		mDatabase->ExecuteSqlAsyncNoArguments(this,asyncContainer,sql);
	}
}



void ChatManager::_processPersistentMessageToServer(Message* message,DispatchClient* client)
{
	string msgText;      // mail text
	msgText.setType(BSTRType_Unicode16);
	msgText.setLength(8192);
	string msgSubject;   // mail subject
	msgSubject.setType(BSTRType_Unicode16);
	msgSubject.setLength(512);
	string targetName;   // recipient
	targetName.setLength(256);
	uint32 mailId;       // mail count of sender, sent in this session
	//uint32 targetStatus = 0; // 0 = exists, 4 = doesn't exist
	Player* sender = NULL;
	Player* receiver = NULL;
	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opHeartBeat);
	newMessage = gMessageFactory->EndMessage();

	client->SendChannelA(newMessage,client->getAccountId(),CR_Client,1);

	// get our sender
	sender = getPlayerByAccId(client->getAccountId());

	if(!sender)
	{
		gLogger->logErrorF("chat","ChatManager::_processPersistentMessageToServer Error finding sender %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	message->getStringUnicode16(msgText);

	string attachmentData;
	message->getStringUnicode16(attachmentData);

	mailId = message->getUint32();
	message->getStringUnicode16(msgSubject);
	message->getUint32();
	message->getStringAnsi(targetName);
	targetName.toLower();


	receiver = getPlayerByName(targetName);

	Mail* mail = new Mail();
	mail->mSender = sender->getName().getAnsi();
	mail->setSubject(msgSubject);
	mail->mSubject.convert(BSTRType_ANSI);
	mail->setText(msgText);
	mail->mText.convert(BSTRType_ANSI);
	mail->mStatus = MailStatus_New;
	mail->mTime = static_cast<uint32>(time(NULL));
	mail->setAttachments(attachmentData);

	// See comment at _PersistentMessagebySystem for this part of the code.
	if (receiver != NULL)
	{
		ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_CreateMail);
		asyncContainer->mMail = mail;
		asyncContainer->mSender = sender;
		asyncContainer->mMailCounter = mailId;
		asyncContainer->mReceiverId = receiver->getCharId();

		int8 sql[20000],*sqlPointer;
		int8 footer[64];
		int8 receiverStr[64];
		sprintf(receiverStr,"',%"PRIu64",'",receiver->getCharId());
		sprintf(footer,",%u,%"PRIu32")",(mail->mAttachments.getLength() << 1),mail->mTime);
		sprintf(sql,"SELECT sf_MailCreate('%s",sender->getName().getAnsi());
		sqlPointer = sql + strlen(sql);
		sqlPointer += mDatabase->Escape_String(sqlPointer,sender->getName().getAnsi(),sender->getName().getLength());
		strcat(sql,receiverStr);
		sqlPointer = sql + strlen(sql);
		sqlPointer += mDatabase->Escape_String(sqlPointer,mail->mSubject.getAnsi(),mail->mSubject.getLength());
		*sqlPointer++ = '\'';
		*sqlPointer++ = ',';
		*sqlPointer++ = '\'';
		sqlPointer += mDatabase->Escape_String(sqlPointer,mail->mText.getAnsi(),mail->mText.getLength());
		*sqlPointer++ = '\'';
		*sqlPointer++ = ',';
		*sqlPointer++ = '\'';
		sqlPointer += mDatabase->Escape_String(sqlPointer,mail->mAttachments.getRawData(),(mail->mAttachments.getLength() << 1));
		*sqlPointer++ = '\'';
		*sqlPointer++ = '\0';
		strcat(sql,footer);

		mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	}
	else
	{
		ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_CheckCharacter);
		asyncContainer->mMail = mail;
		asyncContainer->mSender = sender;
		asyncContainer->mMailCounter = mailId;

		int8 sql[256],*sqlPointer;
		sprintf(sql,"SELECT id FROM characters WHERE LOWER(firstname) LIKE '");
		sqlPointer = sql + strlen(sql);
		sqlPointer += mDatabase->Escape_String(sqlPointer,targetName.getAnsi(),targetName.getLength());
		*sqlPointer++ = '\'';
		*sqlPointer++ = '\0';

		mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	}
}

//======================================================================================================================
//
// mail request, retrieves and returns requested mail contents from db
//

void ChatManager::_processRequestPersistentMessage(Message* message,DispatchClient* client)
{
	uint32 dbMailId;
	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opHeartBeat);
	newMessage = gMessageFactory->EndMessage();

	client->SendChannelA(newMessage,client->getAccountId(),CR_Client,1);


	message->getUint32();            // unknown
	dbMailId = message->getUint32();
	message->getUint8();             // unknown, attachments ?

	ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_MailById);
	asyncContainer->mClient = client;

	int8 sql[256];
	sprintf(sql,"SELECT chat_mail.id,chat_mail.from,chat_mail.subject,chat_mail.body,chat_mail.time,chat_mail.attachments,chat_mail.attachmentSize"
				" FROM chat_mail"
				" WHERE	(chat_mail.id = %"PRIu32")",dbMailId);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
}

//======================================================================================================================
//
// mail delete, deletes mail from the db
//

void ChatManager::_processDeletePersistentMessage(Message* message,DispatchClient* client)
{
	uint32 dbMailId = message->getUint32();
	Message* newMessage;

	message->getUint8();             // unknown, attachments ?

	mDatabase->ExecuteSqlAsync(NULL,NULL,"DELETE FROM chat_mail WHERE id=%u",dbMailId);

	// acknowledge
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opHeartBeat);
	newMessage = gMessageFactory->EndMessage();

	client->SendChannelA(newMessage, client->getAccountId(), CR_Client, 1);

}

//======================================================================================================================

void ChatManager::_processFriendlistUpdate(Message* message,DispatchClient* client)
{
	gLogger->logMsg("Friendlist update");
}

//======================================================================================================================

void ChatManager::_processAddFriend(Message* message,DispatchClient* client)
{
	gLogger->logMsg("Add friend");
}

//======================================================================================================================

Channel* ChatManager::getChannelById(uint32 id)
{
	ChannelMap::iterator it = mChannelMap.find(id);

	if(it != mChannelMap.end())
		return((*it).second);
	else
		gLogger->logErrorF("chat","ChatManager::getChannelById Could not find channel %u",MSG_NORMAL,id);

	return(NULL);
}

//======================================================================================================================

Channel* ChatManager::getChannelByName(string name)
{
	ChannelNameMap::iterator iter = mChannelNameMap.find(name.getCrc());

	if (iter != mChannelNameMap.end())
		return ((*iter).second);

	return NULL;
}

//======================================================================================================================

void ChatManager::_processNotifyChatAddFriend(Message* message,DispatchClient* client)
{
	string name;

	message->getStringUnicode16(name);
	name.convert(BSTRType_ANSI);

	Player* player = getPlayerByAccId(message->getAccountId());

	if (player)
	{
		player->addFriend(name.getAnsi());

		// check if he's online
		Player* target = getPlayerByName(name.getAnsi());

		if(target)
		{
			gChatMessageLib->sendFriendOnlineStatus(player,target,FriendStatus_Online,mMainCategory,mGalaxyName);
		}

		sendFriendList(player);
	}
	else
	{
		gLogger->logErrorF("chat","ChatManager::_processNotifyChatAddFriend Can't find user with account id %u", MSG_NORMAL, message->getAccountId());
	}
}

//======================================================================================================================

void ChatManager::_processNotifyChatRemoveFriend(Message* message,DispatchClient* client)
{
	string name;

	message->getStringUnicode16(name);
	name.convert(BSTRType_ANSI);

	Player* player = getPlayerByAccId(message->getAccountId());

	if(player)
	{
		player->removeFriend(name.getCrc());

		sendFriendList(player);
	}
	else
	{
		gLogger->logErrorF("chat","ChatManager::_processNotifyChatRemoveFriend Can't find user with account id %u", MSG_NORMAL, message->getAccountId());
	}
}

//======================================================================================================================

Player* ChatManager::getPlayerByAccId(uint32 accId)
{
	PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(accId);

	if(accIt != mPlayerAccountMap.end())
	{
		return((*accIt).second);
	}

	return(NULL);
}

//======================================================================================================================

Player* ChatManager::getPlayerByName(string name)
{
	name.toLower();
	PlayerNameMap::iterator nameIt = mPlayerNameMap.find(name.getCrc());

	if(nameIt != mPlayerNameMap.end())
	{
		return((*nameIt).second);
	}

	return(NULL);
}

//======================================================================================================================


void ChatManager::sendFriendList(Player* player)
{
	// gLogger->logMsg("ChatManager::sendFriendList(");

	ContactMap* friendList	= player->getFriendsList();
	ContactMap::iterator it	= friendList->begin();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opChatFriendList);
	gMessageFactory->addUint64(player->getCharId());
	gMessageFactory->addUint32(friendList->size());

	while(it != friendList->end())
	{
		gMessageFactory->addString(mMainCategory);
		gMessageFactory->addString(mGalaxyName);
		gMessageFactory->addString((*it).second);
		++it;
	}

	Message* newMessage = gMessageFactory->EndMessage();

	player->getClient()->SendChannelA(newMessage,player->getClient()->getAccountId(),CR_Client,3);
}

//======================================================================================================================

void ChatManager::sendIgnoreList(Player* player)
{
	// gLogger->logMsg("ChatManager::sendIgnoreList");
/* Since this command is not verified, we put it on hold for now.
	ContactMap* ignoreList	= player->getIgnoreList();
	ContactMap::iterator it	= ignoreList->begin();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opChatIgnoreList);
	gMessageFactory->addUint64(player->getCharId());
	gMessageFactory->addUint32(ignoreList->size());

	while(it != ignoreList->end())
	{
		gMessageFactory->addString(mMainCategory);
		gMessageFactory->addString(mGalaxyName);
		gMessageFactory->addString((*it).second);
		++it;
	}

	Message* newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getClient()->getAccountId(),CR_Client,3,false);
*/
}

//======================================================================================================================

void ChatManager::checkOnlineFriends(Player* player)
{
	// gLogger->logMsg("ChatManager::checkOnlineFriends(");

/*
	ContactMap* friendList	= player->getFriendsList();
	ContactMap::iterator it	= friendList->begin();

	while(it != friendList->end())
	{
		Player* theFriend = getPlayerByName((*it).second);

		if(theFriend)
		{
			gChatMessageLib->sendFriendOnlineStatus(player,theFriend,FriendStatus_Online,mMainCategory,mGalaxyName);
		}
		else
		{
			gLogger->logMsg("Can't find friend", MSG_NORMAL);
		}

		++it;
	}
*/
}

//======================================================================================================================

void ChatManager::updateFriendsOnline(Player* player,bool action)
{
	PlayerList::iterator playerIt = mPlayerList.begin();
	string loweredName = player->getName().getAnsi();
	loweredName.toLower();
	uint32 loweredNameCrc = loweredName.getCrc();
	// gLogger->logMsg("ChatManager::updateFriendsOnline", MSG_NORMAL);
	while(playerIt != mPlayerList.end())
	{
		if((*playerIt) != player && (*playerIt)->checkFriend(loweredNameCrc))
		{
			// Get UNIX-style time and display as number and string.
			time_t ltime;
			time( &ltime );
			/*
			int8 timebuf[64];
			uint32 err;


			err = ctime_s(timebuf, 32, &ltime);
			if (err)
			{
			   sprintf(timebuf,"%s","");
			}
			*/
			string time = ctime(&ltime);

			string name = player->getName();
			name.convert(BSTRType_Unicode16);
			time.convert(BSTRType_Unicode16);
			// gLogger->logMsgF("ChatManager::updateFriendsOnline: Is calling sendFriendOnlineStatus", MSG_NORMAL);
			if(action)
			{
				gChatMessageLib->sendFriendOnlineStatus((*playerIt),player,FriendStatus_Online,mMainCategory,mGalaxyName);
			}
			else
			{
				gChatMessageLib->sendFriendOnlineStatus((*playerIt),player,FriendStatus_Offline,mMainCategory,mGalaxyName);
			}
		}

		++playerIt;
	}
}

//======================================================================================================================

void ChatManager::_processNotifyChatAddIgnore(Message* message,DispatchClient* client)
{
	string name;

	message->getStringUnicode16(name);
	name.convert(BSTRType_ANSI);

	Player* player = getPlayerByAccId(message->getAccountId());

	if(player)
	{
		// gLogger->logMsgF("Player %s add %s to Ignore", MSG_NORMAL, player->getName().getAnsi(), name.getAnsi());
		player->addIgnore(name.getAnsi());

		// Update client with Ignore list.
		sendIgnoreList(player);
	}
	else
	{
		gLogger->logErrorF("chat","ChatManager::_processNotifyChatAddIgnore Can't find player for add ignore with account id %u", MSG_NORMAL, message->getAccountId());
	}
}

//======================================================================================================================

void ChatManager::_processNotifyChatRemoveIgnore(Message* message,DispatchClient* client)
{
	string name;

	message->getStringUnicode16(name);
	name.convert(BSTRType_ANSI);

	Player* player = getPlayerByAccId(message->getAccountId());

	if(player)
	{
		player->removeIgnore(name.getCrc());

		// Update client with Ignore list.
		sendIgnoreList(player);
	}
	else
	{
		gLogger->logErrorF("chat","ChatManager::_processNotifyChatRemoveIgnore Can't find player for remove ignore with account id %u", MSG_NORMAL, message->getAccountId());
	}
}


//////////////////////////////////////////////////////////////////////////////////
//got the position of the zone now report to the searcher
//////////////////////////////////////////////////////////////////////////////////
void ChatManager::_processFindFriendGotPosition(Message* message,DispatchClient* client)
{
	uint64 friendPlayer,player;
	float x,z;
	friendPlayer = message->getUint64();
	player = message->getUint64();
	x = message->getFloat();
	z = message->getFloat();

	Player* friendObject = getPlayerbyId(friendPlayer);
	Player* playerObject = getPlayerbyId(player);

	string unicodeName = friendObject->getName();
	unicodeName.convert(BSTRType_Unicode16);
	friendObject->setPositionX(x);
	friendObject->setPositionZ(z);

	//now create a waypoint for the querying player
	gChatMessageLib->sendSystemMessageProper(playerObject,false, "","ui_cmnty","friend_location","","","",0,"","","",0,0,0,"","",unicodeName.getUnicode16());
	gChatMessageLib->sendFindFriendCreateWaypoint(playerObject->getClient(),playerObject, friendObject);


}

//////////////////////////////////////////////////////////////////////////////////
//Process Findfriend see whether the friend is online and get a valid position
//////////////////////////////////////////////////////////////////////////////////
void ChatManager::_processFindFriendMessage(Message* message,DispatchClient* client)
{
	string friendName(BSTRType_Unicode16,128);
	message->getStringUnicode16(friendName);

	int8			sql[1024],end[16],*sqlPointer;

	Player* playerObject = getPlayerByAccId(message->getAccountId());
	string unicodeName = friendName;
	friendName.convert(BSTRType_ANSI);

	ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_FindFriend);
	asyncContainer->mName = friendName.getAnsi();
	asyncContainer->mSender = playerObject;

	sprintf(sql,"SELECT id from swganh.characters where firstname like '");
	sprintf(end,"'");
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,friendName.getAnsi(),friendName.getLength());
	strcat(sql,end);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);


}

Player* ChatManager::getPlayerbyId(uint64 id)
{

	PlayerList::iterator playerIt = mPlayerList.begin();

	while(playerIt != mPlayerList.end())
	{
		if((*playerIt)->getCharId() == id)
		{
			return (*playerIt);
		}
		playerIt++;
	}
	return NULL;
}

void ChatManager::_handleFindFriendDBReply(Player* player,uint64 retCode,string friendName)
{

	string loweredName = friendName.getAnsi();
	loweredName.toLower();
	uint32 loweredNameCrc = loweredName.getCrc();

	friendName.convert(BSTRType_Unicode16);

	//name not existent
	if(retCode == 0)
	{
		gChatMessageLib->sendSystemMessageProper(player,true,"","ui_cmnty","friend_location_failed_noname","","","",0,"","","",0,0,0,"","",friendName.getUnicode16());
		return;
	}

	// check if he's our friend
	if(!player->checkFriend(loweredNameCrc))
	{
		gChatMessageLib->sendSystemMessageProper(player,false,"","ui_cmnty","friend_location_failed","","","",0,"","","",0,0,0,"","",friendName.getUnicode16());
		return;
	}

	Player*	searchObject	= getPlayerbyId(retCode);
    //now that s/he exists AND is on our friendlist
	//check whether he/she is online
	if(!searchObject)
	{
		gChatMessageLib->sendSystemMessageProper(player,true,"","ui_cmnty","friend_location_failed","","","",0,"","","",0,0,0,"","",friendName.getUnicode16());
		return;
	}

	loweredName = player->getName().getAnsi();
	loweredName.toLower();

	loweredNameCrc = loweredName.getCrc();

	//are we on our targets friendlist???
	if(!searchObject->checkFriend(loweredNameCrc))
	{
		gChatMessageLib->sendSystemMessageProper(player,true,"","ui_cmnty","friend_location_failed","","","",0,"","","",0,0,0,"","",friendName.getUnicode16());
		return;
	}


	//query friend zone for the position
	//handle the rest in the callback
	gChatMessageLib->sendFindFriendRequestPosition(searchObject->getClient(),searchObject,player);

	return;
	//inform the zone in order to create the waypoint
	//will be handled in the trademanagger

	//gChatMessageLib->sendFindFriendCreateWaypoint(player->getClient(), ,searchObject);

	//Datapad* thePad = searchObject->getDatapad();


	//if(thePad->getCapacity())
	//{
	//	gObjectFactory->requestNewWaypoint(thePad,searchObject->getFirstName().getAnsi(),searchObject->mPosition,gWorldManager->getZoneId(),player->getId(),Waypoint_blue);
	//}

}

//======================================================================================================================

bool ChatManager::isValidName(string name)
{
	// DatabaseResult* result = mDatabase->ExecuteSql("SELECT id FROM characters WHERE LCASE(firstname) = '%s';", name.getAnsi());
	int8 sql[128];
	mDatabase->Escape_String(sql, name.getAnsi(), name.getLength());

	DatabaseResult* result = mDatabase->ExecuteSynchSql("SELECT id FROM characters WHERE LCASE(firstname) = '%s';", sql);

	uint64 count = result->getRowCount();
	assert(count <= 1);

	bool valid = (count == 1);
	mDatabase->DestroyResult(result);

	return valid;
}


//======================================================================================================================

bool ChatManager::isValidExactName(string name)
{
	int8 sql[128];
	mDatabase->Escape_String(sql, name.getAnsi(), name.getLength());
	DatabaseResult* result = mDatabase->ExecuteSynchSql("SELECT id FROM characters WHERE BINARY firstname = '%s';", sql);
	uint64 count = result->getRowCount();
	assert(count <= 1);

	bool valid = (count == 1);
	mDatabase->DestroyResult(result);

	return valid;
}

//======================================================================================================================

string* ChatManager::getFirstName(string& name)
{
	string* myName = NULL;
	// string realPlayerName;

	// Assume player is online.
	Player* realPlayer = getPlayerByName(name);
	if (realPlayer)
	{
		myName = new string(realPlayer->getName().getAnsi());
		// myName(name);
		// myName->initRawBSTR(realPlayer->getName().getAnsi(), BSTRType_ANSI);
	}
	else
	{
		// Get first name the hard way...
		myName = new string();
		DataBinding* binding = mDatabase->CreateDataBinding(1);
		binding->addField(DFT_bstring,0,64);
		int8 sql[128];
		mDatabase->Escape_String(sql, name.getAnsi(), name.getLength());
		DatabaseResult* result = mDatabase->ExecuteSynchSql("SELECT firstname FROM characters WHERE LCASE(firstname)= '%s';", sql);

		uint64 count = result->getRowCount();
		assert(count <= 1);
		if (count == 1)
		{
			// We found a username that matched the input.
			// string theName;
			result->GetNextRow(binding,myName);
			// result->GetNextRow(binding,&theName);
			// myName->initRawBSTR(theName.getAnsi(),BSTRType_ANSI);
		}
		mDatabase->DestroyResult(result);
	}
	return myName;
}

//======================================================================================================================
void ChatManager::_processGroupSaySend(Message* message,DispatchClient* client)
{
	//gLogger->logMsg("_processGroupSaySend");

	// gLogger->hexDump(message->getData(), message->getSize());
	message->ResetIndex();

	Player*		player = getPlayerByAccId(client->getAccountId());

	if(player == NULL)
	{
		gLogger->logErrorF("chat","ChatManager::_processGroupSaySend Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	string msg;
	msg.setType(BSTRType_Unicode16);
	msg.setLength(512);

	/* uint32 channelId = */message->getUint32();	// op-code for this command.
	uint32 requestId = message->getUint32();
	message->getStringUnicode16(msg);

	if (player->getGroupId() == 0)
	{
		//gLogger->logMsg("Player not in group\n");
		return;
	}
	else
	{
		//gLogger->logMsg("Player in group\n");
	}

	string sender = BString(player->getName());

	// Get the group and corresponding channel.
	GroupObject* group;
	group = gGroupManager->getGroupById(player->getGroupId());
	if (group == NULL)
	{
		gLogger->logErrorF("chat","ChatManager::_processGroupSaySend Can't find group with group-id %u",MSG_NORMAL,player->getGroupId());
		return;
	}
	assert(group != NULL);

	Channel* channel = group->getChannel();
	if (channel == NULL)
	{
		gLogger->logErrorF("chat","ChatManager::_processGroupSaySend Can't find channel from group with group-id %u",MSG_NORMAL,player->getGroupId());
		return;
	}
	assert(channel != NULL);

	// uint32 requestId = 0; // What's this??

	gChatMessageLib->sendChatOnSendRoomMessage(client, 0, requestId);
	gChatMessageLib->sendChatRoomMessage(channel, mGalaxyName, sender, msg);

	// Convert since we are going to print it.
	// msg.convert(BSTRType_ANSI);
	// gLogger->logMsgF("Groupchat from player %s with id [%"PRIu64"] -> : %s",MSG_NORMAL, player->getName().getAnsi(), player->mCharId, msg.getAnsi());
}


//======================================================================================================================
void ChatManager::_processBroadcastGalaxy(Message* message,DispatchClient* client)
{
	gLogger->logMsg("_processBroadcastGalaxy");

	// gLogger->hexDump(message->getData(), message->getSize());
	message->ResetIndex();

	Player*	player = getPlayerByAccId(client->getAccountId());

	if (player == NULL)
	{
		gLogger->logErrorF("chat","ChatManager::_processBroadcastGalaxy() Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	string msg;
	msg.setType(BSTRType_Unicode16);
	msg.setLength(512);

	/*uint32 opCode = */ message->getUint32();	// op-code for this command.
	uint32 planetId = message->getUint32();
	message->getStringUnicode16(msg);


	PlayerList::iterator listIt = mPlayerList.begin();
	while(listIt != mPlayerList.end())
	{
		if (/*(planetId == -1) ||*/ // planetId is an unsigned int which means it can't be negative.
				(planetId == player->getPlanetId()))
		{
			gChatMessageLib->sendSystemMessage((*listIt), msg.getUnicode16());
		}
		++listIt;
	}

	// Convert since we are going to print it.
	// msg.convert(BSTRType_ANSI);
	// gLogger->logMsgF("ChatManager::_processBroadcastGalaxy() Message = %s", MSG_NORMAL, msg.getAnsi());

}


//======================================================================================================================
void ChatManager::_processScheduleShutdown(Message* message, DispatchClient* client)
{
	gLogger->logMsg("_processScheduleShutdown");

	message->ResetIndex();

	string msg;
	msg.setType(BSTRType_Unicode16);
	msg.setLength(512);

	/*uint32 opCode = */message->getUint32();	// op-code for this command.
	uint32 scheduledTime = message->getUint32();
	message->getStringUnicode16(msg);

	uint8 planetId;
	for (planetId = CR_Corellia; planetId <= CR_Yavin4; planetId++)
	{
		Message* newMessage;
		gMessageFactory->StartMessage();
		gMessageFactory->addUint32(opIsmScheduleShutdown);
		gMessageFactory->addUint32(scheduledTime);
		gMessageFactory->addString(msg);
		newMessage = gMessageFactory->EndMessage();
		client->SendChannelA(newMessage, AdminAccountId, planetId, 2);
	}
	// gLogger->logMsg("_processScheduleShutdown() Have invoked the 10 standard planets");

	planetId = CR_Corellia + 41; // Huuu

	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmScheduleShutdown);
	gMessageFactory->addUint32(scheduledTime);
	gMessageFactory->addString(msg);
	newMessage = gMessageFactory->EndMessage();
	client->SendChannelA(newMessage, AdminAccountId, planetId, 2);

	// gLogger->logMsg("_processScheduleShutdown() Have also invoked Tutorial");


	// Convert since we are going to print it.
	// msg.convert(BSTRType_ANSI);
	// gLogger->logMsgF("ChatManager::_processBroadcastGalaxy() Message = %s", MSG_NORMAL, msg.getAnsi());

}

//======================================================================================================================
//
//
//
//

void ChatManager::_processCancelScheduledShutdown(Message* message, DispatchClient* client)
{
	gLogger->logMsg("_processCancelScheduledShutdown");

	message->ResetIndex();

	string msg;
	msg.setType(BSTRType_Unicode16);
	msg.setLength(512);

	/* uint32 opCode = */message->getUint32();
	/* uint32 option = */message->getUint32();
	message->getStringUnicode16(msg);

	uint8 planetId;
	for (planetId = CR_Corellia; planetId <= CR_Yavin4; planetId++)
	{
		Message* newMessage;
		gMessageFactory->StartMessage();
		gMessageFactory->addUint32(opIsmCancelShutdown);
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(msg);
		newMessage = gMessageFactory->EndMessage();
		client->SendChannelA(newMessage, AdminAccountId, planetId, 2);
	}

	planetId = CR_Corellia + 41; // Huuu

	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmCancelShutdown);
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(msg);
	newMessage = gMessageFactory->EndMessage();
	client->SendChannelA(newMessage, AdminAccountId, planetId, 2);
}
