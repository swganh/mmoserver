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

#ifndef ANH_CHATSERVER_CHATMANAGER_H
#define ANH_CHATSERVER_CHATMANAGER_H

#include <map>
#include <vector>

#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/typedefs.h"
#include "Utils/bstring.h"

//======================================================================================================================

class Channel;
class ChatManager;
class Database;
class DataBinding;
class DispatchClient;
class Player;
class Mail;
class Message;
class MessageDispatch;

//======================================================================================================================

typedef std::map<uint32,Player*> 	PlayerAccountMap;
typedef	std::map<uint32,Player*> 	PlayerNameMap;
typedef	std::map<uint64,Player*> 	PlayerIdMap;
typedef std::vector<Player*>		PlayerList;
typedef std::map<uint32,Channel*>	ChannelMap;
typedef std::map<uint32,Channel*>	ChannelNameMap;
typedef std::vector<Channel*>			ChannelList;

#define	gChatManager	ChatManager::getSingletonPtr()

//======================================================================================================================

enum ChatQuery
{
    ChatQuery_Player			= 0,
    ChatQuery_GalaxyName		= 1,
    ChatQuery_CreateMail		= 2,
    ChatQuery_MailById			= 3,
    ChatQuery_MailHeaders		= 4,
    ChatQuery_CheckCharacter	= 5,
    ChatQuery_Channels			= 6,
    ChatQuery_PlayerChannels	= 7,
    ChatQuery_PlayerFriends		= 8,
    ChatQuery_PlayerIgnores		= 9,
    ChatQuery_QueryFirstName	= 10,
    ChatQuery_PlanetNames		= 11,
    ChatQuery_FindFriend		= 12,
    ChatQuery_AddChannel		= 13,
    ChatQuery_Moderators		= 14,
    ChatQuery_Banned			= 15,
    ChatQuery_Invited			= 16,
    ChatQuery_CharChannels		= 17,
    ChatMailQuery_PlayerIgnores = 18
};

//======================================================================================================================

class ChatAsyncContainer
{
public:
    ChatAsyncContainer(ChatQuery qt)
        : mQueryType(qt)
        , mMail(nullptr)
        , mSender(nullptr)
        , mReceiver(nullptr)
        , mMailCounter(0)
    {}

    ~ChatAsyncContainer() {}

    ChatQuery		mQueryType;
    DispatchClient*	mClient;
    Channel*		mChannel;
    Mail*			mMail;
    Player*			mSender;
    Player*			mReceiver;
    uint64			mReceiverId;
    uint32			mRequestId;
    uint32			mMailCounter;
    BString			mName;
};

//======================================================================================================================

class ChatManager: public DatabaseCallback
{
public:

    friend class ChatMessageLib;

    ~ChatManager();
    static ChatManager*	Init(Database* database,MessageDispatch* dispatch);
    static ChatManager*	getSingletonPtr() {
        return mSingleton;
    }

    virtual void		handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    void				registerChannel(Channel* channel);
    void				unregisterChannel(Channel* channel);

    Channel*			getChannelById(uint32 id);
    Channel*			getChannelByName(BString name);

    BString				getMainCategory() {
        return mMainCategory;
    }
    BString				getGalaxyName() {
        return mGalaxyName;
    }

    Player*				getPlayerByAccId(uint32 accId);
    Player*				getPlayerbyId(uint64 id);
    Player*				getPlayerByName(BString name);
    const int8* getPlanetNameById(uint32 planetId) const {
        return mvPlanetNames[planetId].getAnsi();
    }

    void				sendFriendList(Player* player);

    void				sendIgnoreList(Player* player);

    void				checkOnlineFriends(Player* player);
    void				updateFriendsOnline(Player* player,bool action);

    //send a system EMail without the need of a logged in player
    void				sendSystemMailMessage(Mail* mail,uint64 recipient);


    PlayerAccountMap	getPlayerAccountMap() {
        return mPlayerAccountMap;
    }

private:

    ChatManager(Database* database,MessageDispatch* dispatch);

    void			_loadDatabindings();
    void			_destroyDatabindings();
    void			_loadChannels(DatabaseResult* result);
    void			_registerCallbacks();
    void			_unregisterCallbacks();
    BString			_queryServerName();

    void			_processClusterClientConnect(Message* message,DispatchClient* client);
    void			_processClusterClientDisconnect(Message* message,DispatchClient* client);
    void			_processZoneTransfer(Message* message,DispatchClient* client);

    // global chat
    void			_processRoomlistRequest(Message* message,DispatchClient* client);
    void			_processCreateRoom(Message* message,DispatchClient* client);
    void			_processDestroyRoom(Message* message,DispatchClient* client);
    void			_processRoomQuery(Message* message,DispatchClient* client);
    void			_processRoomMessage(Message* message,DispatchClient* client);
    void			_processSendToRoom(Message* message,DispatchClient* client);
    void			_processEnterRoomById(Message* message,DispatchClient* client);
    void			_processAddModeratorToRoom(Message* message,DispatchClient* client);
    void			_processInviteAvatarToRoom(Message* message,DispatchClient* client);
    void			_processUninviteAvatarFromRoom(Message* message, DispatchClient* client);
    void			_processRemoveModFromRoom(Message* message,DispatchClient* client);
    void			_processRemoveAvatarFromRoom(Message* message,DispatchClient* client);
    void			_processBanAvatarFromRoom(Message* message,DispatchClient* client);
    void			_processUnbanAvatarFromRoom(Message* message,DispatchClient* client);
    void			_processAvatarId(Message* message,DispatchClient* client);
    void			_processLeaveRoom(Message*message, DispatchClient* client);

    void			_processGroupSaySend(Message* message,DispatchClient* client);

    // mail
    void			_processPersistentMessageToServer(Message* message,DispatchClient* client);
    void			_processRequestPersistentMessage(Message* message,DispatchClient* client);
    void			_processDeletePersistentMessage(Message* message,DispatchClient* client);
    void			_PersistentMessagebySystem(Mail* mail,DispatchClient* client, BString sender);
    void			_processSystemMailMessage(Message* message,DispatchClient* client);

    // friendlist
    void			_processFriendlistUpdate(Message* message,DispatchClient* client);
    void			_processAddFriend(Message* message,DispatchClient* client);

    void			_processNotifyChatAddFriend(Message* message,DispatchClient* client);
    void			_processNotifyChatRemoveFriend(Message* message,DispatchClient* client);
    void			_processNotifyChatAddIgnore(Message* message,DispatchClient* client);
    void			_processNotifyChatRemoveIgnore(Message* message,DispatchClient* client);
    void			_processFindFriendMessage(Message* message,DispatchClient* client);
    void			_processFindFriendGotPosition(Message* message,DispatchClient* client);

    void			_handleFindFriendDBReply(Player* player,uint64 retCode,BString friendName);

    void			_processWhenLoaded(Message* message,DispatchClient* client);

    // instant messages
    void			_processInstantMessageToCharacter(Message* message,DispatchClient* client);

    // admin
    void			_processBroadcastGalaxy(Message* message,DispatchClient* client);
    void			_processScheduleShutdown(Message* message,DispatchClient* client);
    void			_processCancelScheduledShutdown(Message* message, DispatchClient* client);

    static bool				mInsFlag;
    static ChatManager*		mSingleton;

    bool					isValidName(BString name);
    bool					isValidExactName(BString name);
    BString*					getFirstName(BString& name);

    Database*				mDatabase;
    MessageDispatch*        mMessageDispatch;
    ChannelList				mvChannels;
    ChannelMap				mChannelMap;
    ChannelNameMap			mChannelNameMap;

    BString					mMainCategory;
    BString					mGalaxyName;
    BStringVector			mvPlanetNames;

    PlayerAccountMap		mPlayerAccountMap;
    PlayerNameMap			mPlayerNameMap;
    PlayerIdMap				mPlayerIdMap;
    PlayerList				mPlayerList;

    DataBinding*			mPlayerBinding;
    DataBinding*			mChannelBinding;
    DataBinding*			mMailBinding;
    DataBinding*			mMailHeaderBinding;
    DataBinding*			mCreatorBinding;
    DataBinding*			mOwnerBinding;

};

#endif

