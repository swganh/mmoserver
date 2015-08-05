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

#ifndef ANH_ZONESEVER_ZONEOPCODES_H
#define ANH_ZONESEVER_ZONEOPCODES_H


enum zone_opcodes
{
    //Harvester / House placement
    opStructurePlacementMode			=	0xe8a54dc1,
    opSendPermissionList				=	0x52F364B8,
    opOperateHarvester					=	0xBD18C679,

    opClusterZoneRegisterName			=	0xb88f5166,
    opNewbieTutorialResponse			=	0xca88fbad,
    opNewbieTutorialRequest				=   0x90DD61AF,
    opOpenHolocronToPageMessage			=	0x7CB65021,
    opChatServerStatus					=	0x7102B15F,
    opParametersMessage					=	0x487652DA,
    opCmdStartScene						=	0x3AE6DFAE,
    opCmdSceneReady						=	0x43FD1C22,
    opCmdSceneReady2					=	0x48f493c5,
    opServerTimeMessage					=	0x2EBC3BD9,
    opServerWeatherMessage				=	0x486356ea,
    opObjectMenuSelection				=	0x7ca18726,
    opEnterTicketPurchaseModeMessage	=	0x904dae1a,
    opPlanetTravelPointListRequest		=	0x96405d4d,
    opPlanetTravelPointListResponse		=	0x4d32541f,
    opChatSystemMessage					=	0x6d2a6413,
    opAttributeListMessage				=	0xf3f12f2a,
    opPlayClientEffectObjectMessage		=	0x8855434a,
    opPlayClientEffectLocMessage		=	0x02949e74,
    opPlayMusicMessage					=	0x04270d8a,
    opResourceListForSurveyMessage		=	0x8a64b1d5,
    opSurveyMessage						=	0x877f79ac,
    opBadgesResponseMessage				=	0x6d89d25b,
    opFactionRequestMessage				=	0xc1b03b81,
    opFactionResponseMessage			=	0x5dd53957,
    opStomachRequestMessage				=	0xB75DD5D7,
    opStomachResponseMessage			=	0x4a093f02,
    opPlayerMoneyRequest				=	0x9D105AA1,
    opPlayerMoneyResponse				=	0x367E737E,
    opGuildRequestMessage				=	0x81EB4EF7,
    opGuildResponseMessage				=	0x32263F20,
    opCharacterSheetResponseMessage		=	0x9B3A17C4,

    //Trade
    opBankTipDustOff					=	0x4516EDA7,
    opGiveMoneyMessage					=	0xD1527EE8,
    opUnacceptTransactionMessage		=	0xE81E4382,
    opBeginVerificationMessage			=   0xE7481DF5,
    opAcceptTransactionMessage			=   0xB131CA17,
    opRemoveItemMessage					=	0x4417AF8B,
    opAddItemMessage					=   0x1E8D1356,
    opTradeCompleteMessage				=   0xC542038B,
    opAbortTradeMessage					=	0x9CA80F98,
    opBeginTradeMessage					=	0x325932D8,
    opVerifyTradeMessage				=	0x9AE247EE,
    opIsVendorMessage                   =   0x21B55A3B,
    opIsVendorOwnerResponseMessage      =   0xCE04173E,
    opAuctionQueryHeadersMessage		=	0x679E0D00,
    opAuctionQueryHeadersResponseMessage=	0xFA500E52,
    opCreateAuctionMessage				=   0xAD47021D,
    opCreateImmediateAuctionMessage		=   0x1E9CE308,
    opProcessSendCreateItem				=   0x4E5468E8,
    opDeductMoneyMessage				=	0x10000000,
    opCreateAuctionMessageResponseMessage=  0xE61CC92,
    opGetAuctionDetails					=	0xD36EFAE4,
    opGetAuctionDetailsResponse			=	0xFE0E644B,
    opCanceLiveAuctionMessage			=	0x3687A4D2,
    opCanceLiveAuctionResponseMessage	=	0x7DA2246C,
    opRetrieveAuctionItemMessage		=	0x12B0D449,
    opRetrieveAuctionItemResponseMessage=	0x9499EF8C,
    opBidAuctionResponseMessage			=	0x8FCBEF4A,
    opProcessCreateAuction				=	0xAC2FD41D,

    opLogoutMessage						=	0x42FD19DD,

    opObjControllerMessage				=	0x80ce5e46,

    opSceneCreateObjectByCrc			=	0xFE89DDEA,
    opUpdateContainmentMessage			=	0x56CBDE9E,
    opUpdatePostureMessage				=	0x0BDE6B41,
    opUpdateCellPermissionMessage		=	0xf612499c,
    opBaselinesMessage					=	0x68A75F0C,
    opDeltasMessage						=	0x12862153,
    opSceneEndBaselines					=	0x2C436037,
    opSceneDestroyObject				=	0x4D45D504,
    opUpdateTransformMessage			=	0x1B24F808,
    opUpdateTransformMessageWithParent	=	0xC867AB5A,
    opUpdatePvpStatusMessage			=	0x08a1c126,
    opChange_Posture					=	0xab290245,
    opOpenedContainer					=	0x2E11E4AB,
    opClosedContainer					=	0x32B79B7E,	  //not functional serverpacket to client

    //social
    opFindFriendCreateWaypoint			=	0xDDA2B297,
    opFindFriendRequestPosition			=	0x35D536D9,
    opFindFriendSendPosition			=	0x7347C6BF,
    opNotifyChatAddFriend				=	0xb581f90d,
    opNotifyChatRemoveFriend			=	0x336015c,
    opNotifyChatFindFriend				=	0xc447e379,
    opNotifyChatAddIgnore				=	0xd387da5,
    opNotifyChatRemoveIgnore			=	0xbb8f85f4,

    opGRUP								=	0x47525550,
    opCREO								=	0x4352454F,
    opPLAY								=	0x504c4159,
    opTANO								=	0x54414e4f,
    opFCYT								=	0x46435954,

    opBUIO								=	0x4255494f,
    opHINO								=	0x48494E4F,
    opINSO								=	0x494E534F,
    opSCLT								=	0x53434c54,
    opRCNO								=	0x52434e4f,
    opMSCO								=	0x4d53434f,
    opMISO								=   0x4d49534f,
    opITNO								=   0x49544e4f,
    opSecureTrade						=   0x00000115,
    opSTAO								=	0x5354414F,
    //ImageDesigner
    opStatMigrationStart				=	0xefac38c4,

    // group ISM (inter-server-messages)

    opIsmSendSystemMailMessage		= 0x7B08578E,
    opIsmGroupInviteRequest			= 0x944F2822,	//[Both]	<uint32 target>
    opIsmGroupInviteResponse		= 0x3FDF93DF,	//[Both]	<bool success>
    opIsmGroupUnInvite				= 0x384CBE2C,	//[Both]	<uint32 target>
    opIsmGroupCREO6deltaGroupId		= 0xF923A570,	//[CH->ZO]	<uint32 target><uint64 groupId>
    opIsmGroupDisband				= 0xEFF47552,	//[ZO->CH]
    opIsmGroupLeave					= 0x2629856B,	//[ZO->CH]
    opIsmGroupLootModeRequest		= 0xAD02C6DA,	//[ZO->CH]	<>
    opIsmGroupLootModeResponse		= 0xF0D065EE,	//[Both]
    opIsmGroupLootMasterRequest		= 0xE91B2BBD,	//[ZO->CH]	<>
    opIsmGroupLootMasterResponse	= 0xCEBF0445,	//[Both]
    opIsmGroupDismissGroupMember	= 0x49BA6C4C,	//[ZO->CH]	<uint32 target>
    opIsmGroupMakeLeader			= 0x9338ADA4,	//[ZO->CH]	<uint32 target>
    opIsmGroupPositionNotification	= 0x2FB45F7B,	//[ZO->CH]	<float x><float z>
    opIsmGroupBaselineRequest		= 0xB0CE545A,	//[ZO->CH]	<float x><float z>
    opIsmGroupAlterMissions			= 0x2F46E3A3,	//[ZO->CH]
    opIsmGroupInviteInRangeRequest	= 0x19F89B8E,	//[CH->ZO]  <uint32 sender><uint32 target>
    opIsmGroupInviteInRangeResponse	= 0xA4B7CA4C,   //[ZO->CH]  <uint32 sender><uint32 target><bool inRange>
    opIsmIsGroupLeaderRequest		= 0xC85225C9,
    opIsmIsGroupLeaderResponse		= 0x1B53DB12,

    // Fake code, crc-based an the word "opIsmGroupSay".
    opIsmGroupSay					= 0x39360616,	//[ZO->CH]  // Fake code, crc-based on the word "opIsmGroupSay".
    opChatNotifySceneReady			= 0x75C4DD84,	//[ZO ->CH] // Fake code, crc-based on the word "chatcmdsceneready".
    opBankTipDeduct					= 0x723BF836,

    // admin inter server messages
    opIsmBroadcastPlanet			= 0x3F9D6D6E,	//[ZO->CH]
    opIsmBroadcastGalaxy			= 0x8E41B5CB,	//[ZO->CH]
    opIsmScheduleShutdown			= 0xF2477D2C,	//[ZO->CH] [CH->ZO]
    opIsmCancelShutdown				= 0x5E43AC09,	//[ZO->CH]

    // structure inter server messages
    opIsmHarvesterUpdate			= 0x8F603896	//[ZO->CH]


};

#endif

