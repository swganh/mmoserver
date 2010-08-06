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
	_loadDatabindings();

	ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_GalaxyName);
	// Commented out the filter for now, at a later time this needs to be updated to not be bound to a single galaxy
	// mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT name FROM galaxy;"); // WHERE galaxy_id=3");
	mDatabase->ExecuteProcedureAsync(this,asyncContainer,"CALL swganh.sp_ReturnGalaxyName(2);");
	gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatChannelMod(2);"); // SQL Debug Log

	asyncContainer = new ChatAsyncContainer(ChatQuery_Channels);
	mDatabase->ExecuteProcedureAsync(this,asyncContainer,"CALL swganh.sp_ReturnChatChannels();");
	gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatChannels();"); // SQL Debug Log

	asyncContainer = new ChatAsyncContainer(ChatQuery_PlanetNames);
	mDatabase->ExecuteProcedureAsync(this,asyncContainer,"CALL swganh.sp_ReturnChatPlanetNames();");
	gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatPlanetNames();"); // SQL Debug Log
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

void ChatManager::_loadChannels(DatabaseResult* result)
{
	uint64 count = result->getRowCount();

	for(uint64 i = 0;i < count;i++)
	{
		BString creator;

		BString owner;
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

		mDatabase->ExecuteProcedureAsync(this, modContainer, "CALL swganh.sp_ReturnChatChannelMod(%u);", channel->getId());
		gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatChannelMod(%u);", channel->getId()); // SQL Debug Log
		mDatabase->ExecuteProcedureAsync(this, bannedContainer, "CALL swganh.sp_ReturnChatChannelBan(%u);", channel->getId());
		gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatChannelBan(%u);", channel->getId()); // SQL Debug Log
		mDatabase->ExecuteProcedureAsync(this, inviteContainer, "CALL swganh.sp_ReturnChatChannelInvite(%u);", channel->getId());
		gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatChannelInvite(%u);", channel->getId()); // SQL Debug Log
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
		gLogger->log(LogManager::CRITICAL,"Could not find channel for removing. %u",channel->getId());
	}
}

//======================================================================================================================

void ChatManager::_registerCallbacks()
{
	mMessageDispatch->RegisterMessageCallback(opClusterClientConnect,std::bind(&ChatManager::_processClusterClientConnect, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opClusterClientDisconnect,std::bind(&ChatManager::_processClusterClientDisconnect, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opClusterZoneTransferCharacter,std::bind(&ChatManager::_processZoneTransfer, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatNotifySceneReady,std::bind(&ChatManager::_processWhenLoaded, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatRequestRoomlist,std::bind(&ChatManager::_processRoomlistRequest, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatCreateRoom,std::bind(&ChatManager::_processCreateRoom, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatDestroyRoom,std::bind(&ChatManager::_processDestroyRoom, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatEnterRoomById,std::bind(&ChatManager::_processEnterRoomById, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatQueryRoom,std::bind(&ChatManager::_processRoomQuery, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatRoomMessage, std::bind(&ChatManager::_processRoomMessage, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatSendToRoom,std::bind(&ChatManager::_processSendToRoom, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatAddModeratorToRoom,std::bind(&ChatManager::_processAddModeratorToRoom, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatInviteAvatarToRoom,std::bind(&ChatManager::_processInviteAvatarToRoom, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatUninviteFromRoom,std::bind(&ChatManager::_processUninviteAvatarFromRoom, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatRemoveModFromRoom,std::bind(&ChatManager::_processRemoveModFromRoom, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatRemoveAvatarFromRoom,std::bind(&ChatManager::_processRemoveAvatarFromRoom, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatBanAvatarFromRoom,std::bind(&ChatManager::_processBanAvatarFromRoom, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatUnbanAvatarFromRoom,std::bind(&ChatManager::_processUnbanAvatarFromRoom, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatAvatarId,std::bind(&ChatManager::_processAvatarId, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatInstantMessageToCharacter,std::bind(&ChatManager::_processInstantMessageToCharacter, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatPersistentMessageToServer,std::bind(&ChatManager::_processPersistentMessageToServer, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatRequestPersistentMessage,std::bind(&ChatManager::_processRequestPersistentMessage, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatDeletePersistentMessage,std::bind(&ChatManager::_processDeletePersistentMessage, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatFriendlistUpdate,std::bind(&ChatManager::_processFriendlistUpdate, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opChatAddFriend,std::bind(&ChatManager::_processAddFriend, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opNotifyChatAddFriend,std::bind(&ChatManager::_processNotifyChatAddFriend, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opNotifyChatRemoveFriend,std::bind(&ChatManager::_processNotifyChatRemoveFriend, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opNotifyChatAddIgnore,std::bind(&ChatManager::_processNotifyChatAddIgnore, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opNotifyChatRemoveIgnore,std::bind(&ChatManager::_processNotifyChatRemoveIgnore, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opNotifyChatFindFriend,std::bind(&ChatManager::_processFindFriendMessage, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opFindFriendSendPosition,std::bind(&ChatManager::_processFindFriendGotPosition, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opSendSystemMailMessage,std::bind(&ChatManager::_processSystemMailMessage, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opIsmGroupSay,std::bind(&ChatManager::_processGroupSaySend, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opIsmBroadcastGalaxy,std::bind(&ChatManager::_processBroadcastGalaxy, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opIsmScheduleShutdown,std::bind(&ChatManager::_processScheduleShutdown, this, std::placeholders::_1, std::placeholders::_2));
	mMessageDispatch->RegisterMessageCallback(opIsmCancelShutdown,std::bind(&ChatManager::_processCancelScheduledShutdown, this, std::placeholders::_1, std::placeholders::_2));
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

void ChatManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	ChatAsyncContainer* asyncContainer = (ChatAsyncContainer*)ref;

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

				mDatabase->ExecuteProcedureAsync(this,asContainer,"CALL swganh.sp_ReturnChatFriendlist(%"PRIu64");",asContainer->mReceiver->getCharId());
				gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatFriendlist(%"PRIu64");", asContainer->mReceiver->getCharId()); // SQL Debug Log
			}
			else
				gLogger->log(LogManager::NOTICE,"Could not find account %u",asyncContainer->mClient->getAccountId());
		}
		break;

		case ChatQuery_GalaxyName:
		{
			uint64 count = result->getRowCount();
			
			if (count == 0) {
				gLogger->log(LogManager::NOTICE,"Could not find Galaxy ");
				return;
			}

			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,64);

			result->GetNextRow(binding,&mGalaxyName);

			gLogger->log(LogManager::DEBUG,"Main: [%s] Galaxy: [%s]",mMainCategory.getAnsi(),mGalaxyName.getAnsi());

			mDatabase->DestroyDataBinding(binding);
		}
		break;


		case ChatQuery_CheckCharacter:
		{
			uint64 receiverId = 0;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,0,8);

			if(result->getRowCount())
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

			result->GetNextRow(binding,&dbMailId);

			// query ignoreslist
			ChatAsyncContainer* asContainer = new ChatAsyncContainer(ChatMailQuery_PlayerIgnores);

			asContainer->mMail = asyncContainer->mMail;
			asContainer->mSender = asyncContainer->mSender;
			asContainer->mRequestId = dbMailId;
			asContainer->mMailCounter = asyncContainer->mMailCounter;
			asContainer->mReceiverId = asyncContainer->mReceiverId;

			mDatabase->ExecuteProcedureAsync(this,asContainer,"CALL swganh.sp_ReturnChatIgnoreList(%"PRIu64");", asyncContainer->mReceiverId);
			gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatIgnoreList(%"PRIu64");", asyncContainer->mReceiverId); // SQL Debug Log
			mDatabase->DestroyDataBinding(binding);
		}
		break;


		case ChatMailQuery_PlayerIgnores:
		{
			Message* newMessage;
			BString	name;
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
					gLogger->log(LogManager::DEBUG," %s ChatMailQuery_PlayerIgnores [%s]", receiver->getName().getAnsi(), name.getAnsi());
				}
				ignoreList.insert(std::make_pair(name.getCrc(),name.getAnsi()));
			}

			bool bIgnore = false;

			// If receiver have sender ignored, auto delete mail (don't send it and remove from db).
			if (asyncContainer->mSender != NULL)
			{
				BString ignoreName = asyncContainer->mSender->getName();
				ignoreName.toLower();

				// check receivers ignorelist.
				ContactMap::iterator it = ignoreList.find(ignoreName.getCrc());
				bIgnore = (it != ignoreList.end());
				if (bIgnore)
				{
					mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_DeleteMail(%u);", asyncContainer->mRequestId);
					gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_MailDelete(%u);", asyncContainer->mRequestId); // SQL Debug Log
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
			if(!result->getRowCount())
			{
				gLogger->log(LogManager::DEBUG," not found mail with id %u",asyncContainer->mRequestId);
				SAFE_DELETE(asyncContainer);
				return;
			}
			Mail mail = Mail();
			mail.mSubject.setLength(512);
			mail.mText.setLength(8192);

			result->GetNextRow(mMailBinding,&mail);

			memcpy(mail.mAttachments.getRawData(), mail.mAttachmentRaw, mail.mAttachmentSize);

			mail.mAttachments.setLength(static_cast<uint16>(mail.mAttachmentSize >> 1));
			mail.mSubject.convert(BSTRType_Unicode16);
			mail.mText.convert(BSTRType_Unicode16);

			gChatMessageLib->sendChatPersistantMessagetoClient(asyncContainer->mClient,&mail);

			//mDatabase->ExecuteSqlAsync(NULL,NULL,"UPDATE chat_mail SET status = 1 WHERE id=%u", mail.mId);
			mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_MailStatusUpdate(%u)", mail.mId);
			gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_MailStatusUpdate(%u);", asyncContainer->mRequestId); // SQL Debug Log

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
			BString	name;
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

			mDatabase->ExecuteProcedureAsync(this,asContainer,"CALL swganh.sp_ReturnChatIgnorelist(%"PRIu64");",asContainer->mReceiver->getCharId());
			gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatIgnorelist(%"PRIu64");",asContainer->mReceiver->getCharId()); // SQL Debug Log
		}
		break;

		case ChatQuery_PlayerIgnores:
		{
			BString	name;
			Player*	player = asyncContainer->mReceiver;

			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,64);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&name);
				name.toLower();
				player->getIgnoreList()->insert(std::make_pair(name.getCrc(),name.getAnsi()));
			}

			mDatabase->DestroyDataBinding(binding);

			// Update channel info.
			ChatAsyncContainer* asContainer = new ChatAsyncContainer(ChatQuery_CharChannels);
			asContainer->mClient = asyncContainer->mClient;

			Player* currentPlayer = getPlayerByAccId(asyncContainer->mClient->getAccountId());

			mDatabase->ExecuteProcedureAsync(this, asContainer,"CALL swganh.sp_ReturnChatCharChannels(%"PRIu64");", currentPlayer->getCharId());
			gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatCharChannels(%"PRIu64");", currentPlayer->getCharId()); // SQL Debug Log
		}
		break;

		case ChatQuery_CharChannels:
		{
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);
			uint32 roomId;

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&roomId);
				Channel* channel = getChannelById(roomId);
				Player* player = getPlayerByAccId(asyncContainer->mClient->getAccountId());
				if(player == NULL)
				{
					gLogger->log(LogManager::DEBUG,"Error getting player from account map %u", asyncContainer->mClient->getAccountId());
					continue;
				}
				ChatAvatarId* avatar = new ChatAvatarId();
				avatar->setPlayer(player);
				avatar->setGalaxy(mGalaxyName);

				//If I'm banned or not invited to a private channel then stay out.
				if (channel->isBanned(avatar->getLoweredName()))
				{
					gLogger->log(LogManager::DEBUG,"Player was banned\n");
					continue;
				}
				if (channel->isPrivate())
				{
					if (!channel->isInvited(avatar->getLoweredName()))
					{
						gLogger->log(LogManager::DEBUG,"Player was not invited to private channel\n");
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
			BString	name;
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
					BString* name = new BString();
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
					BString* name = new BString();
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
					BString* name = new BString();
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
			BString			tmp;
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

	// @NOTE this is a hack that is needed for the time being. Currently the MessageDispatch
	// handles these opClusterClientConnect messages however messages coming from a zone
	// server require additional extra processing. To ensure this is only handled at the
	// appropriate time check to see if a character id has been set as only the zone server sets this.
	if (charId == 0) {
		return;
	}

	Player* player = new Player(charId,client,planetId);

	gLogger->log(LogManager::DEBUG,"Connecting account %u with player id %"PRIu64"", client->getAccountId(), charId);

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

	mDatabase->ExecuteProcedureAsync(this,asyncContainer,"CALL swganh.sp_ReturnCharacterName(%"PRIu64")",charId);
	gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnCharacterName(%"PRIu64")",charId); // SQL Debug Log

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
		gLogger->log(LogManager::DEBUG,"Error finding player in player account map %u",client->getAccountId());
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
			gChatMessageLib->sendChatOnLeaveRoom(client, channel->findUser(player->getName().getAnsi()), channel, 0);
			channel->removeUser(player);
		}
		else
		{
			gLogger->log(LogManager::DEBUG,"ChatManager:: Can't find player %s in planet channel", player->getName().getAnsi());
		}
	}
	else
	{
		// This is the normal path for the Tutorial, it has no planet channel.
		gLogger->log(LogManager::DEBUG,"Can't find channel for planet %u", player->getPlanetId());

		// We cant just return here, we need to continue and remove the player from the account map etc below.
	}
	// channel->removeUser(player);

	// Remove user from user created channels.
	ChannelList::iterator channelIt = mvChannels.begin();

	while (channelIt != mvChannels.end())
	{
		// User in channel?
		ChatAvatarId* av = (*channelIt)->findUser(player);
		if (av)
		{
			// No idea to try and talk to own client here...
			// TODO: fix talk to self...

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
		gLogger->log(LogManager::DEBUG,"Error removing player %s from name map",player->getName().getAnsi());
	}
}

//======================================================================================================================

// The client is loaded, after a connect or zone transfer.
void ChatManager::_processWhenLoaded(Message* message,DispatchClient* client)
{
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
				gLogger->log(LogManager::DEBUG,"No channel for planet %u\n", player->getPlanetId());
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

				// Update friends list
				updateFriendsOnline(asContainer->mReceiver,true);

				mDatabase->ExecuteProcedureAsync(this,asContainer,"CALL swganh.sp_ReturnChatMailHeaders(%"PRIu64");",asContainer->mReceiver->getCharId());
				gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_ReturnChatMailHeaders(%"PRIu64");",asContainer->mReceiver->getCharId()); // SQL Debug Log
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
	gLogger->log(LogManager::DEBUG,"_processZoneTransfer");

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
			gLogger->log(LogManager::DEBUG,"No channel for depature planet %u", player->getPlanetId());
		}
		else
		{
			assert(channel != NULL);
			ChatAvatarId* avatar = channel->findUser(player->getName());
			if(!avatar)
			{
				gLogger->log(LogManager::DEBUG,"ChatManager::_processZoneTransfer No avatar \n");

			}
			else
				gChatMessageLib->sendChatOnLeaveRoom(client, avatar, channel, 0);

			channel->removeUser(player);
		}
		player->setPlanetId(planetId);

		channel = getChannelById(planetId + 23);
		if (channel == NULL)
		{
			gLogger->log(LogManager::DEBUG,"No channel for destination planet %u\n", planetId);
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
		gLogger->log(LogManager::DEBUG,"Error getting player %u",client->getAccountId());
		return;
	}
}

//======================================================================================================================

void ChatManager::_processRoomlistRequest(Message* message,DispatchClient* client)
{
	gChatMessageLib->sendChatRoomList(client, &mvChannels);

	Player* player = getPlayerByAccId(client->getAccountId());
	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"Error getting player from account map %u",client->getAccountId());
		return;
	}
}

//======================================================================================================================

void ChatManager::_processCreateRoom(Message* message,DispatchClient* client)
{
	BString path;
	BString title;

	Player* player = getPlayerByAccId(client->getAccountId());
	BString* playername = new BString(BSTRType_ANSI, player->getName().getLength());
	strcpy(playername->getAnsi(), player->getName().getAnsi());

	uint8 publicFlag = message->getUint8();
	uint8 moderatedFlag = message->getUint8();
	message->getUint16(); //null ascii string
	message->getStringAnsi(path);
	message->getStringAnsi(title);
	uint32 requestId = message->getUint32();

	uint32 index = 5 + mGalaxyName.getLength();
	BString modpath;
	path.substring(modpath, static_cast<uint16>(index), path.getLength());

	// modpath.toLower();

	gLogger->log(LogManager::DEBUG,"Attempting to create channel %s at %s\n", title.getAnsi(), modpath.getAnsi());

	ChannelList::iterator iter = mvChannels.begin();
	while (iter != mvChannels.end())
	{
		if (Anh_Utils::cmpistr((*iter)->getName().getAnsi(), modpath.getAnsi()) == 0)
		{
			gLogger->log(LogManager::DEBUG,"Channel %s already exist\n", modpath.getAnsi());
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
	gLogger->log(LogManager::DEBUG, "SQL :: CALL sf_CreateChannel('%s', %u, %u, '%s', '%s');", modpath.getAnsi(), channel->isPrivate(), moderatedFlag, sql /* playername->getAnsi() */, sql_2 /* title.getAnsi()*/ ); // SQL Debug Log

	// TEST
	gLogger->log(LogManager::DEBUG,"Channel %s created at %s\n", title.getAnsi(), modpath.getAnsi());

	delete playername;
}

//======================================================================================================================

void ChatManager::_processDestroyRoom(Message* message,DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"DestroyRoom");
	uint32 roomId = message->getUint32();
	/* uint32 requestId = */message->getUint32();

	Channel* channel = getChannelById(roomId);
	if (channel == NULL)
	{
		gLogger->log(LogManager::DEBUG,"No channel for room %u", roomId);
		return;
	}

	// This is me destroying the channel
	BString playername = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());

	// Lowercase...
	playername.toLower();

	// Request by Moderator or Owner?
	if ((!channel->isModerator(playername)) && (!channel->isOwner(playername)))
	{
		gChatMessageLib->sendChatOnDestroyRoom(client, channel, 16);
		return;
	}

	gLogger->log(LogManager::DEBUG,"Player %s destroying channel %s\n", playername.getAnsi(), channel->getName().getAnsi());
	ChatAvatarId* avatar = channel->findUser(playername);
	if (avatar == NULL)
	{
	}
	else
	{
		// Remove user from channel...before destruction.
		gChatMessageLib->sendChatOnLeaveRoom(client, avatar, channel, 0);
	}

	// Update all users that this channel is gone. And I mean ALL...
	PlayerList::iterator listIt = mPlayerList.begin();
	while(listIt != mPlayerList.end())
	{
		gLogger->log(LogManager::DEBUG,"Channel %s gone for %s\n", channel->getName().getAnsi(), (*listIt)->getName().getAnsi());
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
	mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomDelete(%u);", roomId);
	gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomDelete(%u);", roomId); // SQL Debug Log
}

//======================================================================================================================

void ChatManager::_processRoomQuery(Message* message,DispatchClient* client)
{
	BString roomname;
	BString path;

	uint32 requestId = message->getUint32();
	message->getStringAnsi(path);

	uint32 index = 5 + mGalaxyName.getLength();
	path.substring(roomname, static_cast<uint16>(index), path.getLength());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->log(LogManager::DEBUG,"No channel for room %s", roomname.getAnsi());
		return;
	}
	gChatMessageLib->sendChatQueryRoomResults(client, channel, requestId);
}

//======================================================================================================================

void ChatManager::_processRoomMessage(Message* message,DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"_processRoomMessage");
}

//======================================================================================================================
//
// Incoming tell
//
void ChatManager::_processInstantMessageToCharacter(Message* message,DispatchClient* client)
{
	BString mainCategory; // "SWG"
	BString serverName;   // (galaxy name)
	BString targetName;   // recipient
	targetName.setLength(256);
	BString msgText;
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
		gLogger->log(LogManager::DEBUG,"Error finding sender %u",client->getAccountId());
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

	// If a galaxy name is provided, automatically flag
	// as the user not existing.
	// This feature was supported in Live SWG ( Cross Galaxy Talk )
	// however, the likelyhood of a community having more then
	// one galaxy is sparce.
	if(strcmp( gChatManager->getGalaxyName().getAnsi(), serverName.getAnsi() ) != 0)
	{
		receiver = NULL;
		targetStatus = 4;
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
	uint32 requestId = message->getUint32();
	uint32 roomId = message->getUint32();

	Channel* channel = getChannelById(roomId);
	if (channel == NULL)
	{
		gLogger->log(LogManager::DEBUG,"No channel for room %u", roomId);
		return;
	}
	Player* player = getPlayerByAccId(client->getAccountId());
	if (player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"No player for account %u", client->getAccountId());
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
			gLogger->log(LogManager::DEBUG,"Player %s already in room %s\n", player->getName().getAnsi(),channel->getName().getAnsi());
			return;
		}
		++iter;
	}

	if (channel->isBanned(avatar->getLoweredName()))
	{
		// You cannot join '%TU (room name)' because you are not invted to the room
		gChatMessageLib->sendChatFailedToEnterRoom(client, avatar, 16, channel, requestId);
		gLogger->log(LogManager::DEBUG,"Player was banned\n");
		return;
	}

	if (channel->isPrivate())
	{
		if (channel->isInvited(avatar->getLoweredName()))
		{
			gLogger->log(LogManager::DEBUG,"Channel was private\n");
		}
		else
		{
			// You cannot join '%TU (room name)' because you are not invted to the room
			gChatMessageLib->sendChatFailedToEnterRoom(client, avatar, 16, channel, requestId);
			gLogger->log(LogManager::DEBUG,"Player was not invited\n");
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
	}
	else
	{
	}
	*/
	channel->addUser(avatar);
	gChatMessageLib->sendChatOnEnteredRoom(client, avatar, channel, requestId);

	mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomUserAdd(%"PRIu64", %u);", player->getCharId(), channel->getId());
	gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomUserAdd(%"PRIu64", %u);", player->getCharId(), channel->getId()); // SQL Debug Log
}

//======================================================================================================================

void ChatManager::_processSendToRoom(Message* message,DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"_processSendToRoom");

	Player*	player = getPlayerByAccId(client->getAccountId());

	if (player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"Error getting player from account map %u",client->getAccountId());
		return;
	}

	BString msg;
	msg.setType(BSTRType_Unicode16);
	msg.setLength(512);

	message->getStringUnicode16(msg);
	uint32 requestId = message->getUint32(); // unknown
	uint32 channelId = message->getUint32();

	Channel* channel = getChannelById(channelId);
	if (channel == NULL)
	{
		gLogger->log(LogManager::DEBUG,"No channel with id %u", channelId);
		gChatMessageLib->sendChatOnSendRoomMessage(client, 1, requestId);	// Error code 1 will give the default error message.
		return;
	}

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	BString sender = BString(player->getName().getAnsi());
	BString realSenderName = sender;
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
	gLogger->log(LogManager::DEBUG,"Add moderator to room");
	BString playerName;
	BString roompath;
	BString roomname;
	BString game;
	BString server;

	message->getStringAnsi(game);
	message->getStringAnsi(server);
	message->getStringAnsi(playerName);
	message->getStringAnsi(roompath);
	uint32 requestId = message->getUint32();

	uint32 index = 5 + mGalaxyName.getLength();
	roompath.substring(roomname, static_cast<uint16>(index), roompath.getLength());

	gLogger->log(LogManager::DEBUG,"Channel is %s", roomname.getAnsi());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->log(LogManager::DEBUG,"No channel for room %s", roomname.getAnsi());
		return;
	}


	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	BString sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	BString realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	// Get real first name.
	BString* newName = getFirstName(playerName);
	BString realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->log(LogManager::DEBUG,"No player with name %s found\n", playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	BString realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->log(LogManager::DEBUG,"No player with name %s found", playerName.getAnsi());
	}
#endif
	// We check in logical order, even if we know that playername is not valid.
	if (!channel->isModerated())
	{
		// Channel is not moderated.
		gLogger->log(LogManager::DEBUG,"Channel is not moderated");
		errorCode = 9;
	}
	else if ((!channel->isModerator(sender)) && (!channel->isOwner(sender)))
	{
		errorCode = 16;
		gLogger->log(LogManager::DEBUG,"%s is not owner or moderated in channel %s.",realSenderName.getAnsi(), roomname.getAnsi());
		// gChatMessageLib->sendChatFailedToAddMod(client, mGalaxyName, sender, playername, channel, 16, requestId);
		// return;
	}
	// If we have invalid player name, we don't need the following checks.
	else if ((errorCode == 0) && (channel->isModerator(playerName)))
	{
		gLogger->log(LogManager::DEBUG,"%s is already a moderator in channel %s.", realPlayerName.getAnsi(), roomname.getAnsi());
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

		// mDatabase->ExecuteSqlAsync(NULL, NULL, "INSERT INTO chat_channels_moderators VALUES (%u, '%s');", channel->getId(), sql /* realPlayerName.getAnsi() */);
		
		mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomModeratorAdd(%u, '%s');", channel->getId(), sql);
		gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomModeratorAdd(%u, '%s');", channel->getId(), sql); // SQL Debug Log

		gChatMessageLib->sendChatOnAddModeratorToRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, requestId);
	}
#ifdef DISP_REAL_FIRST_NAME
	// delete realPlayerName;
#endif
}

//======================================================================================================================

void ChatManager::_processInviteAvatarToRoom(Message* message,DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"Invite avatar to room");
	BString nameOfInvitedPlayer;
	BString roompath;
	BString roomname;
	BString game;
	BString server;
	BString playerName;

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
		gLogger->log(LogManager::DEBUG,"No channel for room %s", roomname.getAnsi());
		return;
	}

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	BString sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	BString realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	BString* newName = getFirstName(playerName);
	BString realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->log(LogManager::DEBUG,"No player with name %s found\n", playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	BString realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->log(LogManager::DEBUG,"No player with name %s found", playerName.getAnsi());
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
		gLogger->log(LogManager::DEBUG,"%s has invited %s to join channel %s",realSenderName.getAnsi(), realPlayerName.getAnsi(), roomname.getAnsi());
		channel->addInvitedUser(&playerName);
		int8 sql[128];
		mDatabase->Escape_String(sql, playerName.getAnsi(), playerName.getLength());

		//mDatabase->ExecuteSqlAsync(NULL, NULL, "INSERT INTO chat_channels_invited VALUES (%u, '%s');", channel->getId(), sql /* playerName.getAnsi() */);
		mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomUserInvite(%u, '%s');", channel->getId(), sql);
		gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomUserInvite(%u, '%s');", channel->getId(), sql);
		gLogger->log(LogManager::DEBUG, "Player %s is added to database for invited", playerName.getAnsi());

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
	gLogger->log(LogManager::DEBUG,"Uninvite avatar from room");
	BString nameOfInvitedPlayer;
	BString roompath;
	BString roomname;
	BString game;
	BString server;
	BString playerName;

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
		gLogger->log(LogManager::DEBUG,"No channel for room %s", roomname.getAnsi());
		return;
	}

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	BString sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	BString realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	// Get real first name.
	BString* newName = getFirstName(playerName);
	BString realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->log(LogManager::DEBUG,"No player with name %s found\n", playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	BString realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->log(LogManager::DEBUG,"No player with name %s found", playerName.getAnsi());
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
		gLogger->log(LogManager::DEBUG,"PLayer %s becomes un-invited", realPlayerName.getAnsi());
		(void)channel->removeInvitedUser(playerName);

		int8 sql[128];
		mDatabase->Escape_String(sql, playerName.getAnsi(), playerName.getLength());

		//	mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_invited WHERE char_name = '%s' AND channel_id = %u;", channel->getId(), sql);

		mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomUserUnInvite(%u, '%s');", channel->getId(), sql);
		gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomUserUnInvite(%u, '%s');", channel->getId(), sql);

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
	gLogger->log(LogManager::DEBUG,"Remove Moderator from room");
	BString playerName;
	BString roompath;
	BString roomname;
	BString game;
	BString server;

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
		gLogger->log(LogManager::DEBUG,"No channel for room %s", roomname.getAnsi());
		return;
	}

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	BString sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	BString realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	// Get real first name.
	BString* newName = getFirstName(playerName);
	BString realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->log(LogManager::DEBUG,"No player with name %s found\n", playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	BString realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->log(LogManager::DEBUG,"No player with name %s found", playerName.getAnsi());
	}
#endif
	// We check in logical order, even if we know that playername is not valid.
	if (!channel->isModerated())
	{
		// Channel is not moderated.
		errorCode = 9;
		gLogger->log(LogManager::DEBUG,"Channel is not moderated");
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

		// mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_moderators WHERE char_name = '%s' AND channel_id = %u;", sql /* playerName.getAnsi() */, channel->getId());

		mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomModeratorRemove(%u, '%s');", channel->getId(), sql);
		gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomModeratorRemove(%u, '%s');", channel->getId(), sql);

		gChatMessageLib->sendChatOnRemoveModeratorFromRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, requestId);
	}
#ifdef DISP_REAL_FIRST_NAME
	// delete realPlayerName;
#endif
}

//======================================================================================================================

void ChatManager::_processRemoveAvatarFromRoom(Message* message,DispatchClient* client)
{
	BString playerName;
	BString roompath;
	BString roomname;
	BString game;
	BString server;

	message->getStringAnsi(game);
	message->getStringAnsi(server);
	message->getStringAnsi(playerName);
	message->getStringAnsi(roompath);

	uint32 index = 5 + mGalaxyName.getLength();
	roompath.substring(roomname, static_cast<uint16>(index), roompath.getLength());

	Channel* channel = getChannelByName(roomname);
	if (channel == NULL)
	{
		gLogger->log(LogManager::DEBUG,"No channel for room %s", roomname.getAnsi());
		return;
	}

	playerName.toLower();
	uint32 errorCode = 0;
	ChatAvatarId* avatar = channel->findUser(playerName);
	if (avatar == NULL)
	{
		// User not in room.
		// errorCode = 4; We can't use it since the Avatar is not defined...
		gLogger->log(LogManager::DEBUG,"Can't find user %s in channel", playerName.getAnsi());
	}
	else
	{
		//mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_char_channels WHERE channel_id = %u AND character_id = %"PRIu64";", channel->getId(), avatar->getPlayer()->getCharId());
		
		mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomUserRemove(%u, %"PRIu64");", avatar->getPlayer()->getCharId(), channel->getId());
		gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomUserRemove(%u, %"PRIu64");", avatar->getPlayer()->getCharId(), channel->getId());
		gChatMessageLib->sendChatOnLeaveRoom(client, avatar, channel, 0, errorCode);
	}
	channel->removeUser(playerName);
}

//======================================================================================================================

void ChatManager::_processBanAvatarFromRoom(Message* message,DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"Ban Avatar from room");
	BString playerName;
	BString roompath;
	BString roomname;
	BString game;
	BString server;

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
		gLogger->log(LogManager::DEBUG,"No channel for room %s", roomname.getAnsi());
		return;
	}

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	BString sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	BString realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	// Get real first name.
	BString* newName = getFirstName(playerName);
	BString realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->log(LogManager::DEBUG,"No player with name %s found\n", playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	BString realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->log(LogManager::DEBUG,"No player with name %s found", playerName.getAnsi());
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
		gLogger->log(LogManager::DEBUG,"PLayer %s becomes kicked, banned and un-invited", playerName.getAnsi());

		// Kick the player if present in channel.
		ChatAvatarId* avatar = channel->findUser(playerName);
		if (avatar)
		{
			// mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_char_channels WHERE channel_id = %u AND character_id = %"PRIu64";", channel->getId(), avatar->getPlayer()->getCharId());
			
			mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomUserRemove(%u, %"PRIu64");", avatar->getPlayer()->getCharId(), channel->getId());
			gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomUserRemove(%u, %"PRIu64");", avatar->getPlayer()->getCharId(), channel->getId());
			
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

			// mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_invited WHERE char_name = '%s' AND channel_id = %u;", sql /* playerName.getAnsi() */, channel->getId());

			mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomUserUnInvite(%u, '%s');", channel->getId(), sql);
			gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomUserUnInvite(%u, '%s');", channel->getId(), sql);

			// Removed since it gives un-wanted spam back to client.
			// gChatMessageLib->sendChatOnUninviteFromRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, 0);

			// gChatMessageLib->sendChatQueryRoomResults(client, channel, 0);
		}

		// Get the ban-stick in ready position
		channel->banUser(&playerName);
		// int8 sql[128];
		mDatabase->Escape_String(sql, playerName.getAnsi(), playerName.getLength());
		// mDatabase->ExecuteSqlAsync(NULL, NULL, "INSERT INTO chat_channels_banned VALUES (%u, '%s');", channel->getId(), sql /* playerName.getAnsi()*/);

		mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomUserBan(%u, '%s');", channel->getId(), sql);
		gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomUserBan(%u, '%s');", channel->getId(), sql);

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
	BString playerName;
	BString roompath;
	BString roomname;
	BString game;
	BString server;

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
		gLogger->log(LogManager::DEBUG,"No channel for room %s", roomname.getAnsi());
		return;
	}

	// We use two versions of names, one with the real spelling and one with pure lowercase.
	playerName.toLower();
	BString sender = BString(getPlayerByAccId(client->getAccountId())->getName().getAnsi());
	BString realSenderName = sender;
	sender.toLower();

	uint32 errorCode = 0;

#ifdef DISP_REAL_FIRST_NAME
	// Get real first name.
	BString* newName = getFirstName(playerName);
	BString realPlayerName;

	if (newName->getLength() == 0)
	{
		errorCode = 4;
		realPlayerName = playerName;	// We have to stick with this name when error reporting.
		gLogger->log(LogManager::DEBUG,"No player with name %s found\n", playerName.getAnsi());
	}
	else
	{
		realPlayerName = *newName;
	}
	delete newName;
#else
	// Lowercase
	BString realPlayerName = playerName;
	realSenderName.toLower();

	// Well, the player don't have to be online.
	if (!isValidName(playerName))
	{
		errorCode = 4;
		gLogger->log(LogManager::DEBUG,"No player with name %s found", playerName.getAnsi());
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
		gLogger->log(LogManager::DEBUG,"Can't un-ban player %s", realPlayerName.getAnsi());
		gChatMessageLib->sendChatFailedToUnban(client, mGalaxyName, realSenderName, realPlayerName, channel, errorCode, requestId);
	}
	else
	{
		(void)channel->unBanUser(playerName);
		int8 sql[128];
		mDatabase->Escape_String(sql, playerName.getAnsi(), playerName.getLength());

		// mDatabase->ExecuteSqlAsync(NULL, NULL, "DELETE FROM chat_channels_banned WHERE char_name = '%s' AND channel_id = %u;", sql /* playerName.getAnsi() */, channel->getId());

		mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_ChatRoomUserUnBan(%u, '%s');", channel->getId(), sql);
		gLogger->log(LogManager::DEBUG, "SQL :: CALL sp_ChatRoomUserUnBan(%u, '%s');", channel->getId(), sql);

		gChatMessageLib->sendChatOnUnBanAvatarFromRoom(client, mGalaxyName, realSenderName, realPlayerName, channel, requestId);
	}
#ifdef DISP_REAL_FIRST_NAME
	// delete realPlayerName;
#endif
}

//======================================================================================================================

void ChatManager::_processAvatarId(Message* message,DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG, "Avatar Id");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//allows the trade manager to send EMails even if neither sender nor recipient is logged in
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
	sprintf(sql, "SELECT firstname FROM characters WHERE id LIKE %"PRIu64"", recipient);

	mDatabase->ExecuteSqlAsyncNoArguments(this, asyncContainer, sql);
	gLogger->log(LogManager::DEBUG, "SQL :: ", sql);
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
	BString Sender;
	Sender.setLength(128);
	message->getStringAnsi(Sender);

	BString msgSubject;
	msgSubject.setLength(128);
	message->getStringAnsi(msgSubject);

	BString msgText(BSTRType_Unicode16,2048);
	message->getStringUnicode16(msgText);

	BString attachmentData(BSTRType_Unicode16,2048);
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
	sprintf(sql, "SELECT firstname FROM characters WHERE id LIKE %"PRIu64"", ReceiverID);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);


}
void ChatManager::_PersistentMessagebySystem(Mail* mail,DispatchClient* client, BString receiverStr)
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

		mDatabase->ExecuteSqlAsyncNoArguments(this, asyncContainer, sql);

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
		sqlPointer += mDatabase->Escape_String(sqlPointer, receiverStr.getAnsi(), receiverStr.getLength());
		*sqlPointer++ = '\'';
		*sqlPointer++ = '\0';

		mDatabase->ExecuteSqlAsyncNoArguments(this, asyncContainer, sql);
	}
}



void ChatManager::_processPersistentMessageToServer(Message* message,DispatchClient* client)
{
	BString msgText;      // mail text
	msgText.setType(BSTRType_Unicode16);
	msgText.setLength(8192);
	BString msgSubject;   // mail subject
	msgSubject.setType(BSTRType_Unicode16);
	msgSubject.setLength(512);
	BString targetName;   // recipient
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
		gLogger->log(LogManager::DEBUG,"ChatManager::_processPersistentMessageToServer Error finding sender %u",client->getAccountId());
		return;
	}

	message->getStringUnicode16(msgText);

	BString attachmentData;
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
	

	message->getUint32();            // unknown
	dbMailId = message->getUint32();
	message->getUint8();             // unknown, attachments ?

	ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_MailById);
	asyncContainer->mClient = client;
	asyncContainer->mRequestId = dbMailId;

	int8 sql[256];
	sprintf(sql,"CALL sp_ReturnChatMailById(%"PRIu32");",dbMailId);

	mDatabase->ExecuteProcedureAsync(this,asyncContainer,sql);
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

	// mDatabase->ExecuteSqlAsync(NULL,NULL,"DELETE FROM chat_mail WHERE id=%u",dbMailId);
	mDatabase->ExecuteProcedureAsync(NULL, NULL, "CALL sp_DeleteMail(%u);", dbMailId);
	gLogger->log(LogManager::DEBUG,"SQL :: CALL sp_MailDelete(%u);", dbMailId); // SQL Debug Log

	// acknowledge
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opHeartBeat);
	newMessage = gMessageFactory->EndMessage();

	client->SendChannelA(newMessage, client->getAccountId(), CR_Client, 1);

}

//======================================================================================================================

void ChatManager::_processFriendlistUpdate(Message* message,DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"Friendlist update");
}

//======================================================================================================================

void ChatManager::_processAddFriend(Message* message,DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"Add friend");
}

//======================================================================================================================

Channel* ChatManager::getChannelById(uint32 id)
{
	ChannelMap::iterator it = mChannelMap.find(id);

	if(it != mChannelMap.end())
		return((*it).second);
	else
		gLogger->log(LogManager::DEBUG,"chat","ChatManager::getChannelById Could not find channel %u",id);

	return(NULL);
}

//======================================================================================================================

Channel* ChatManager::getChannelByName(BString name)
{
	ChannelNameMap::iterator iter = mChannelNameMap.find(name.getCrc());

	if (iter != mChannelNameMap.end())
		return ((*iter).second);

	return NULL;
}

//======================================================================================================================

void ChatManager::_processNotifyChatAddFriend(Message* message,DispatchClient* client)
{
	BString name;

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
		gLogger->log(LogManager::DEBUG,"ChatManager::_processNotifyChatAddFriend Can't find user with account id %u", message->getAccountId());
	}
}

//======================================================================================================================

void ChatManager::_processNotifyChatRemoveFriend(Message* message,DispatchClient* client)
{
	BString name;

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
		gLogger->log(LogManager::DEBUG,"ChatManager::_processNotifyChatRemoveFriend Can't find user with account id %u", message->getAccountId());
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

Player* ChatManager::getPlayerByName(BString name)
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
		}

		++it;
	}
*/
}

//======================================================================================================================

void ChatManager::updateFriendsOnline(Player* player,bool action)
{
	PlayerList::iterator playerIt = mPlayerList.begin();
	BString loweredName = player->getName().getAnsi();
	loweredName.toLower();
	uint32 loweredNameCrc = loweredName.getCrc();
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
			BString time = ctime(&ltime);

			BString name = player->getName();
			name.convert(BSTRType_Unicode16);
			time.convert(BSTRType_Unicode16);
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
	BString name;

	message->getStringUnicode16(name);
	name.convert(BSTRType_ANSI);

	Player* player = getPlayerByAccId(message->getAccountId());

	if(player)
	{
		player->addIgnore(name.getAnsi());

		// Update client with Ignore list.
		sendIgnoreList(player);
	}
	else
	{
		gLogger->log(LogManager::DEBUG,"ChatManager::_processNotifyChatAddIgnore Can't find player for add ignore with account id %u", message->getAccountId());
	}
}

//======================================================================================================================

void ChatManager::_processNotifyChatRemoveIgnore(Message* message,DispatchClient* client)
{
	BString name;

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
		gLogger->log(LogManager::DEBUG,"ChatManager::_processNotifyChatRemoveIgnore Can't find player for remove ignore with account id %u", message->getAccountId());
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

	BString unicodeName = friendObject->getName();
	unicodeName.convert(BSTRType_Unicode16);
	friendObject->setPositionX(x);
	friendObject->setPositionZ(z);

	//now create a waypoint for the querying player
	gChatMessageLib->sendSystemMessageProper(playerObject, false, "", "ui_cmnty","friend_location", "", "", "", 0, "", "", "", 0, 0, 0, "", "", unicodeName.getUnicode16());
	gChatMessageLib->sendFindFriendCreateWaypoint(playerObject->getClient(), playerObject, friendObject);


}

//////////////////////////////////////////////////////////////////////////////////
//Process Findfriend see whether the friend is online and get a valid position
//////////////////////////////////////////////////////////////////////////////////
void ChatManager::_processFindFriendMessage(Message* message,DispatchClient* client)
{
	BString friendName(BSTRType_Unicode16,128);
	message->getStringUnicode16(friendName);

	int8			sql[1024],end[16],*sqlPointer;

	Player* playerObject = getPlayerByAccId(message->getAccountId());
	BString unicodeName = friendName;
	friendName.convert(BSTRType_ANSI);

	ChatAsyncContainer* asyncContainer = new ChatAsyncContainer(ChatQuery_FindFriend);
	asyncContainer->mName = friendName.getAnsi();
	asyncContainer->mSender = playerObject;

	sprintf(sql,"SELECT id FROM swganh.characters WHERE firstname LIKE '");
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

void ChatManager::_handleFindFriendDBReply(Player* player,uint64 retCode,BString friendName)
{

	BString loweredName = friendName.getAnsi();
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

bool ChatManager::isValidName(BString name)
{
	// DatabaseResult* result = mDatabase->ExecuteSql("SELECT id FROM characters WHERE LCASE(firstname) = '%s';", name.getAnsi());
	int8 sql[128];
	mDatabase->Escape_String(sql, name.getAnsi(), name.getLength());

	DatabaseResult* result = mDatabase->ExecuteSynchSql("SELECT id FROM characters WHERE LCASE(firstname) = '%s';", sql);

	bool valid = (result->getRowCount() == 1);
	mDatabase->DestroyResult(result);

	return valid;
}


//======================================================================================================================

bool ChatManager::isValidExactName(BString name)
{
	int8 sql[128];
	mDatabase->Escape_String(sql, name.getAnsi(), name.getLength());
	DatabaseResult* result = mDatabase->ExecuteSynchSql("SELECT id FROM characters WHERE BINARY firstname = '%s';", sql);

	bool valid = (result->getRowCount() == 1);
	mDatabase->DestroyResult(result);

	return valid;
}

//======================================================================================================================

BString* ChatManager::getFirstName(BString& name)
{
	BString* myName = NULL;
	// BString realPlayerName;

	// Assume player is online.
	Player* realPlayer = getPlayerByName(name);
	if (realPlayer)
	{
		myName = new BString(realPlayer->getName().getAnsi());
		// myName(name);
		// myName->initRawBSTR(realPlayer->getName().getAnsi(), BSTRType_ANSI);
	}
	else
	{
		// Get first name the hard way...
		myName = new BString();
		DataBinding* binding = mDatabase->CreateDataBinding(1);
		binding->addField(DFT_bstring,0,64);
		int8 sql[128];
		mDatabase->Escape_String(sql, name.getAnsi(), name.getLength());
		DatabaseResult* result = mDatabase->ExecuteSynchSql("SELECT firstname FROM characters WHERE LCASE(firstname)= '%s';", sql);

		if (result->getRowCount() == 1)
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
	// gLogger->hexDump(message->getData(), message->getSize());
	message->ResetIndex();

	Player*		player = getPlayerByAccId(client->getAccountId());

	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"ChatManager::_processGroupSaySend Error getting player from account map %u",client->getAccountId());
		return;
	}

	BString msg;
	msg.setType(BSTRType_Unicode16);
	msg.setLength(512);

	/* uint32 channelId = */message->getUint32();	// op-code for this command.
	uint32 requestId = message->getUint32();
	message->getStringUnicode16(msg);

	if (player->getGroupId() == 0)
	{
		return;
	}

	BString sender = BString(player->getName());

	// Get the group and corresponding channel.
	GroupObject* group;
	group = gGroupManager->getGroupById(player->getGroupId());
	if (group == NULL)
	{
		gLogger->log(LogManager::DEBUG,"ChatManager::_processGroupSaySend Can't find group with group-id %u",player->getGroupId());
		return;
	}

	Channel* channel = group->getChannel();
	if (channel == NULL)
	{
		gLogger->log(LogManager::DEBUG,"ChatManager::_processGroupSaySend Can't find channel from group with group-id %u",player->getGroupId());
		return;
	}

	// uint32 requestId = 0; // What's this??

	gChatMessageLib->sendChatOnSendRoomMessage(client, 0, requestId);
	gChatMessageLib->sendChatRoomMessage(channel, mGalaxyName, sender, msg);

	// Convert since we are going to print it.
	// msg.convert(BSTRType_ANSI);
}


//======================================================================================================================
void ChatManager::_processBroadcastGalaxy(Message* message,DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"_processBroadcastGalaxy");

	// gLogger->hexDump(message->getData(), message->getSize());
	message->ResetIndex();

	Player*	player = getPlayerByAccId(client->getAccountId());

	if (player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"ChatManager::_processBroadcastGalaxy() Error getting player from account map %u",client->getAccountId());
		return;
	}

	BString msg;
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
}


//======================================================================================================================
void ChatManager::_processScheduleShutdown(Message* message, DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"_processScheduleShutdown");

	message->ResetIndex();

	BString msg;
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

	planetId = CR_Corellia + 41; // Huuu

	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmScheduleShutdown);
	gMessageFactory->addUint32(scheduledTime);
	gMessageFactory->addString(msg);
	newMessage = gMessageFactory->EndMessage();
	client->SendChannelA(newMessage, AdminAccountId, planetId, 2);

	// Convert since we are going to print it.
	// msg.convert(BSTRType_ANSI);
}

//======================================================================================================================
//
//
//
//

void ChatManager::_processCancelScheduledShutdown(Message* message, DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"_processCancelScheduledShutdown");

	message->ResetIndex();

	BString msg;
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
