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

#ifndef ANH_ZONESERVER_CHATOPCODES_H
#define ANH_ZONESERVER_CHATOPCODES_H


enum chat_opcodes
{

    opIsVendorMessage                   =   0x21B55A3B,
    opIsVendorOwnerResponseMessage      =   0xCE04173E,
    opAuctionQueryHeadersMessage		=	0x679E0D00,
    opAuctionQueryHeadersResponseMessage=	0xFA500E52,
    opCreateAuctionMessageResponseMessage=  0xE61CC92,
    opGetAuctionDetails					=	0xD36EFAE4,
    opGetAuctionDetailsResponse			=	0xFE0E644B,
    opCancelLiveAuctionMessage			=	0x3687A4D2,
    opCanceLiveAuctionResponseMessage	=	0x7DA2246C,
    opBidAuctionMessage					=   0x91125453,
    opBidAuctionResponseMessage			=   0x8FCBEF4A,
    opRetrieveAuctionItemMessage		=	0x12B0D449,
    opRetrieveAuctionItemResponseMessage=	0x9499EF8C,
    opGetCommoditiesTypeList			=   0x48F493C5,
    opCommoditiesTypeListResponse		=   0xD4E937FC,

    //inter Server Kommunikation
    opDeductMoneyMessage				= 0x10000000,
    opBidAuctionAcceptedMessage			= 0x8737A639,
    opSendSystemMailMessage				= 0x7B08578E,
    opProcessSendCreateItem				= 0x4E5468E8,
    opProcessCreateAuction				= 0xAC2FD41D,
    opFindFriendCreateWaypoint			= 0xdda2b297,
    opFindFriendRequestPosition			= 0x35d536d9,
    opFindFriendSendPosition			= 0x7347c6bf,
    opBankTipDustOff					= 0x4516EDA7,
    opBankTipDeduct						= 0x723BF836,

    opIsmHarvesterUpdate				= 0x8F603896,	//[ZO->CH]



    opConnectPlayerMessage			= 0x2e365218,
    opConnectPlayerResponseMessage	= 0x6137556F,
    opClusterClientConnect			= 0x6B9E5323,
    opClusterClientDisconnect		= 0x44e7e4fa,
    opClusterZoneTransferCharacter	= 0x74C4FC34,

    opChatRequestRoomlist			= 0x4c3d2cfa,
    opChatRoomlist					= 0x70deb197,

    opChatCreateRoom				= 0x35366bed,
    opChatOnCreateRoom				= 0x35d7cc9f,

    opChatDestroyRoom				= 0x094b2a77,
    opChatOnDestroyRoom				= 0xe8ec5877,

    opChatEnterRoomById				= 0xbc6bddf2,
    opChatOnEnteredRoom				= 0xe69bdc0a,

    opChatOnLeaveRoom				= 0x60b5098b,

    opChatQueryRoom					= 0x9cf2b192,
    opChatQueryRoomResults			= 0xc4de864e,

    opChatSendToRoom				= 0x20e4dbe3,
    opChatOnSendRoomMessage			= 0xe7b61633,
    opChatRoomMessage				= 0xcd4ce444,

    opChatAddModeratorToRoom		= 0x90bde76f,
    opChatOnModerateRoom			= 0x36a03858,

    opChatInviteAvatarToRoom		= 0x7273ecd3,
    opChatOnInviteRoom				= 0x493fe74a,

    opChatOnReceiveRoomInvite		= 0xc17eb06d,

    opChatRemoveModFromRoom			= 0x8a3f8e04,
    opChatOnRemoveModFromRoom		= 0x1342fc47,

    opChatRemoveAvatarFromRoom		= 0x493e3ffa,

    opChatUninviteFromRoom			= 0xfc8d01f1,
    opChatOnUninviteFromRoom		= 0xbe33c7e8,

    opChatBanAvatarFromRoom			= 0xd9fa0194,
    opChatOnBanAvatarFromRoom		= 0x5a38538d,

    opChatUnbanAvatarFromRoom		= 0x4c8f94a9,
    opChatOnUnbanAvatarFromRoom		= 0xbaf9b815,

    opChatAvatarId					= 0x179a47fe,

    opChatOnConnectAvatar			= 0xD72FE9BE,

    // instant messages
    opChatInstantMessageToCharacter = 0x84bb21f7, // C -> S
    opChatInstantMessageToClient    = 0x3c565ced, // S -> C(target)
    opChatOnSendInstantMessage		= 0x88dbb381, // S -> C(sender)

    // mail
    opChatPersistentMessageToServer	= 0x25a29fa6, // C -> S
    opChatPersistentMessageToClient	= 0x08485e17, // S -> C (target)
    opChatOnSendPersistentMessage	= 0x94e7a7ae, // S -> C (sender)

    opChatRequestPersistentMessage	= 0x07e3559f,

    opChatDeletePersistentMessage	= 0x8f251641,
    opChatOnDeletePersistentMessage	= 0x4f23965a,

    // friendslist

    // ignoreList

    // This one is not verified or explained in wiki (afaik)
    // We get it when we log in after a Ctrl+escape (Disconnect).
    // The ..Play9 handles the update so far, but we don't wanna see all these un-handled messages.
    // TODO: Implement
    opGetIgnorelist					= 0x788BA6A3,

    // note: addfriend, addignore are sent through ObjControllerMessage
    // didnt see those in precu yet...
    opChatFriendlistUpdate			= 0x6cd2fcd8,
    opChatAddFriend					= 0x6c002d13,

    opChatFriendList				= 0xe97ab594,

    opNotifyChatAddFriend			= 0xb581f90d,
    opNotifyChatRemoveFriend		= 0x336015c,
    opNotifyChatAddIgnore			= 0xd387da5,
    opNotifyChatRemoveIgnore		= 0xbb8f85f4,
    opNotifyChatFindFriend			= 0xc447e379,

    // This one is not verified or explained in wiki (afaik)
    opChatIgnoreList				= 0xea566326,

    opHeartBeat						= 0xa16cf9af,

    opChatSystemMessage				= 0x6D2A6413,
    opSceneCreateObjectByCrc		= 0xFE89DDEA,
    opSceneDestroyObject			= 0x4D45D504,
    opBaselinesMessage				= 0x68A75F0C,
    opDeltasMessage					= 0x12862153,
    opSceneEndBaselines				= 0x2C436037,
    opClientMfdStatusUpdateMessage	= 0x2D2D6EE1,

    opCREO							= 0x4352454F,
    opGRUP							= 0x47525550,
    opHINO							= 0x48494E4F,

    // group ISM (inter-server-messages)
    // these are custom opcodes
    //opIsmSendSystemMailMessage		= 0x7B08578E,
    opIsmGroupInviteRequest			= 0x944F2822,	//[Both]	<uint32 target>
    opIsmGroupInviteResponse		= 0x3FDF93DF,	//[Both]	<bool success>
    opIsmGroupUnInvite				= 0x384CBE2C,	//[Both]	<uint32 target>
    opIsmGroupCREO6deltaGroupId		= 0xF923A570,	//[CH->ZO]	<uint32 target><uint64 groupId>
    opIsmGroupDisband				= 0xEFF47552,	//[ZO->CH]
    opIsmGroupLeave					= 0x2629856B,	//[ZO->CH]
    opIsmGroupLootModeRequest		= 0xAD02C6DA,	//[ZO->CH]	<>
    opIsmGroupLootModeResponse		= 0xF0D065EE,	//[CH->ZO]	<uint8 allowed>
    opIsmGroupLootMasterRequest		= 0xE91B2BBD,	//[ZO->CH]	<>
    opIsmGroupLootMasterResponse	= 0xCEBF0445,	//[Both]
    opIsmGroupDismissGroupMember	= 0x49BA6C4C,	//[ZO->CH]	<uint32 target>
    opIsmGroupMakeLeader			= 0x9338ADA4,	//[ZO->CH]	<uint32 target>
    opIsmGroupPositionNotification	= 0x2FB45F7B,	//[ZO->CH]	<float x><float z>
    opIsmGroupBaselineRequest		= 0xB0CE545A,	//[ZO->CH]	<float x><float z>
    opIsmGroupAlterMissions			= 0x2F46E3A3,
    opISMUpdateStructureCondition	= 0xED4643B6,
    opIsmGroupInviteInRangeRequest	= 0x19F89B8E,	//[CH->ZO]  <uint32 sender><uint32 target>
    opIsmGroupInviteInRangeResponse	= 0xA4B7CA4C,   //[ZO->CH]  <uint32 sender><uint32 target><bool inRange>
    opIsmIsGroupLeaderRequest		= 0xC85225C9,
    opIsmIsGroupLeaderResponse		= 0x1B53DB12,

    opIsmGroupSay					= 0x39360616,	//[ZO->CH]
    opChatNotifySceneReady			= 0x75C4DD84,	//[ZO->CH]  // Fake code, crc-based on the word "chatcmdsceneready".

    // character creation messages
    opClientIdMsg				=	0xd5899226,
    opClientPermissionsMessage	=	0xE00730E5,

    opClientCreateCharacter				=	0xB97F3074,
    opClientRandomNameRequest			=	0xD6D1B6D1,
    opClientRandomNameResponse			=	0xE85FB868,
    opClientCreateCharacterSuccess		=	0x1DB575CC,
    opClientCreateCharacterFailed		=	0xdf333c6e,
    opLagRequest						=	0x31805EE0,

    // Map location messages
    opGetMapLocationsMessage			=	0x1a7ab839,
    opGetMapLocationsResponseMessage	=	0x9f80464c,

    // admin inter server messages
    opIsmBroadcastPlanet			= 0x3F9D6D6E,	//[ZO->CH]
    opIsmBroadcastGalaxy			= 0x8E41B5CB,	//[ZO->CH]
    opIsmScheduleShutdown			= 0xF2477D2C,	//[ZO <-> CH]
    opIsmCancelShutdown				= 0x5E43AC09	//[ZO <-> CH]

};

//======================================================================================================================

enum mail_stats
{
    MailStatus_New				= 0x4e,  // 'N'
    MailStatus_Read				= 0x52  // 'R'
};

//======================================================================================================================

enum friend_stats
{
    FriendStatus_Online		= 1,
    FriendStatus_Offline	= 0
};

//======================================================================================================================

#endif

