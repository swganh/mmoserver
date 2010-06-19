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
#include "ChatOpcodes.h"
#include "ChatManager.h"
#include "ChatMessageLib.h"
#include "GroupManager.h"
#include "GroupObject.h"
#include "Player.h"

#include "ZoneServer/TangibleEnums.h"

#include "LogManager/LogManager.h"

#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"

#include "Utils/utils.h"

#include <cassert>

bool            GroupManager::mInsFlag    = false;
GroupManager*   GroupManager::mSingleton  = NULL;


//======================================================================================================================

GroupManager::GroupManager(MessageDispatch* dispatch)
{

#if defined(_MSC_VER)
	mNextGroupId				= 0x0000000F00000000; //groups range
#else
	mNextGroupId				= 0x0000000F00000000LLU; //groups range
#endif
	mLootModeMessages.push_back(L"@group:selected_free4all");
	mLootModeMessages.push_back(L"@group:selected_master");
	mLootModeMessages.push_back(L"@group:selected_lotto");
	mLootModeMessages.push_back(L"@group:selected_random");

	mLootModeNotAllowedMessages.push_back(L"@group:leader_only_free4all");
	mLootModeNotAllowedMessages.push_back(L"@group:leader_only_master");
	mLootModeNotAllowedMessages.push_back(L"@group:leader_only_lottery");
	mLootModeNotAllowedMessages.push_back(L"Only the Group Leader can set the looting options.  Your group is currently set to Random.");

	mMessageDispatch = dispatch;

	mMessageDispatch->RegisterMessageCallback(opIsmGroupInviteRequest,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupInviteResponse,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupUnInvite,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupDisband,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupLeave,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupDismissGroupMember,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupMakeLeader,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupPositionNotification,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupBaselineRequest,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupInviteRequest,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupLootModeRequest,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupLootModeResponse,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupLootMasterRequest,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupLootMasterResponse,this);
	mMessageDispatch->RegisterMessageCallback(opIsmGroupInviteInRangeResponse, this);
	mMessageDispatch->RegisterMessageCallback(opIsmIsGroupLeaderRequest, this);
}


//======================================================================================================================

GroupManager::~GroupManager()
{
	
}

//======================================================================================================================

GroupManager*	GroupManager::Init(MessageDispatch* dispatch)
{
	if(!mSingleton)
	{
		mSingleton = new GroupManager(dispatch);
	}
	
	return mSingleton;
}

//======================================================================================================================

void GroupManager::Shutdown()
{
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupInviteRequest); 
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupInviteResponse); 
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupUnInvite);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupDisband);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupLeave);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupDismissGroupMember);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupMakeLeader);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupPositionNotification);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupBaselineRequest);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupInviteRequest);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupLootModeRequest);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupLootModeResponse);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupLootMasterRequest);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupLootMasterResponse);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupInviteInRangeResponse);
}

//======================================================================================================================

GroupObject* GroupManager::getGroupById(uint64 groupId)
{
	if(groupId == 0) 
	{
		return(NULL);
	}

	GroupMap::iterator it = mGroups.find(groupId);

	if(it !=  mGroups.end())
		return((*it).second);
	else
		gLogger->log(LogManager::DEBUG, "GroupManager::getGroupById: Could not find group %"PRIu64"",groupId);

	return(NULL);
}


//======================================================================================================================

void GroupManager::removeGroup(uint64 groupId)
{
	GroupMap::iterator it = mGroups.find(groupId);

	if(it !=  mGroups.end())
	{
		delete it->second;
		mGroups.erase(it);
	}
	else
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::removeGroup: Could not find group for removing %"PRIu64"",groupId);
	}

}

//======================================================================================================================

void GroupManager::handleDispatchMessage(uint32 opcode, Message* message, DispatchClient* client)
{
	switch(opcode)
	{

		case opIsmGroupInviteRequest:
		{
			_processGroupInviteRequest(message, client);
		}
		break;


		case opIsmGroupInviteResponse:
		{
			_processGroupInviteResponse(message, client);	
		}
		break;

		case opIsmGroupUnInvite:
		{
			_processGroupUnInvite(message, client);	
		}
		break;

		case opIsmGroupDisband:
		{
			_processGroupDisband(message, client);	
		}
		break;

		case opIsmGroupLeave:
		{
			_processGroupLeave(message, client);	
		}
		break;

		case opIsmGroupDismissGroupMember:
		{
			_processGroupDismissGroupMember(message, client);	
		}
		break;

		case opIsmGroupMakeLeader:
		{
			_processGroupMakeLeader(message, client);	
		}
		break;

		case opIsmGroupPositionNotification:
		{
			_processGroupPositionNotification(message, client);	
		}
		break;

		case opIsmGroupBaselineRequest:
		{
			_processGroupBaselineRequest(message, client);	
		}
		break;

		case opIsmGroupLootModeRequest:
		{
			_processGroupLootModeRequest(message, client);
		}
		break;

		case opIsmGroupLootModeResponse:
		{
			_processGroupLootModeResponse(message, client);
		}
		break;

		case opIsmGroupLootMasterRequest:
		{
			_processGroupLootMasterRequest(message, client);
		}
		break;

		case opIsmGroupLootMasterResponse:
		{
			_processGroupLootMasterResponse(message, client);
		}
		break;

		case opIsmGroupInviteInRangeResponse:
		{
			_processIsmInviteInRangeResponse(message, client);
		}
		break;

		case opIsmIsGroupLeaderRequest:
		{
			_processIsmIsGroupLeaderRequest(message, client);
		}

		default:
		gLogger->log(LogManager::DEBUG,"GroupManager::handleDispatchMessage: Unhandled opcode %u",opcode);
		break;
	} 
}

void GroupManager::_processIsmIsGroupLeaderRequest(Message* message, DispatchClient* client)
{
	uint64 requestId = message->getUint64();
	uint64 playerId = message->getUint64();
	uint64 groupId	= message->getUint64();

	GroupObject* group = this->getGroupById(groupId);

	if(!group)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processIsmIsGroupLeaderRequest: Couldnt find group with id %I64u; player %I64u",groupId,playerId);
	}

	gChatMessageLib->sendIsmIsGroupLeaderResponse(group->getLeader(), requestId, (group->getLeader()->getCharId() == playerId));
}

//======================================================================================================================

void GroupManager::_processGroupInviteRequest(Message* message, DispatchClient* client)
{

	Player* player			= gChatManager->getPlayerByAccId(client->getAccountId());
	Player* targetPlayer	= gChatManager->getPlayerByAccId(message->getUint32());

	if(targetPlayer == NULL || player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupInviteRequest player not found");
		return;
	}

	// If they are not on the same planet, they are obviously not in-range.
	if(player->getPlanetId() != targetPlayer->getPlanetId())
	{
		gChatMessageLib->sendSystemMessage(player, L"@error_message:error_invite_range");
		return;
	}

	// Ask the zone server if both players are in range.
	gChatMessageLib->sendIsmInviteInRangeRequest(player, targetPlayer);
}

//======================================================================================================================

void GroupManager::_processGroupInviteResponse(Message* message, DispatchClient* client)
{

	Player* player = gChatManager->getPlayerByAccId(client->getAccountId());

	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupInviteResponse: player not found");
		return;
	}

	if(player->getGroupId() == 0 || player->getGroupMemberIndex() != 0xFFFF )
	{
		// player hasnt been invited
		gChatMessageLib->sendSystemMessage(player,L"@group:must_be_invited");
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		// the group has died im the mean time
		// lets tell zone to update groupId of this guy
		// and tell him the group is no more
		gChatMessageLib->sendIsmGroupCREO6deltaGroupId(0,player);
		gChatMessageLib->sendSystemMessage(player, L"@group:disbanded");
		return;
	}


	// if player refuses to join the group
	if(message->getUint8() == 0)
	{
		gChatMessageLib->sendGroupSystemMessage(player->getName(), BString("decline_leader"), group->getLeader(), NULL);
		group->removeTempMember(player);	
		return;
	}


	// if accept to join the group
	gChatMessageLib->sendSystemMessage(player, L"@group:joined_self");

	player->setPositionX(message->getFloat());		
	player->setPositionZ(message->getFloat());	

	// if this is the first member, we have to
	// create objects in leader's client too
	Player *groupLeader = group->getLeader();
	DispatchClient* clientLeader = groupLeader->getClient();

	ChatAvatarId* avatarLeader = NULL;
	if(group->getSize() == 1)
	{	
		// create the channel and create the group on leaders'client
		group->createChannel();

		avatarLeader = new ChatAvatarId();
		avatarLeader->setPlayer(group->getLeader());
		avatarLeader->setGalaxy(gChatManager->getGalaxyName());

		group->getChannel()->addUser(avatarLeader);
		group->sendCreate(group->getLeader());
		gChatMessageLib->sendSystemMessage(group->getLeader(), L"@group:formed_self");
	}

	// set the member index of the player
	player->setGroupMemberIndex(group->getSize());

	// advise existing members
	group->broadcastDeltaAdd(player);

	// add the member to the list
	group->addMember(player);

	ChatAvatarId* avatar = new ChatAvatarId();
	avatar->setPlayer(player);
	avatar->setGalaxy(gChatManager->getGalaxyName());

	group->getChannel()->addUser(avatar);
	
	// create the group on the new client
	group->sendCreate(player);

	// gChatMessageLib->sendChatOnEnteredRoom(client, avatar, group->getChannel(), 0);

	// When we have changed number of members in group, we need to update the client also.
	// right now doing a quick and dirty for two players since not everything is working.

	if (avatarLeader)
	{
		gChatMessageLib->sendChatQueryRoomResults(clientLeader, group->getChannel(), 0);
	}

	gChatMessageLib->sendChatQueryRoomResults(client, group->getChannel(), 0);
	
	if (avatarLeader)
	{
		gChatMessageLib->sendChatOnEnteredRoom(clientLeader, avatarLeader, group->getChannel(), 0);
	}

	gChatMessageLib->sendChatOnEnteredRoom(client, avatar, group->getChannel(), 0);
}

//======================================================================================================================

void GroupManager::_processGroupUnInvite(Message* message, DispatchClient* client)
{

	Player* player = gChatManager->getPlayerByAccId(client->getAccountId());

	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupUnInvite: player not found");
		return;
	}

	Player* targetPlayer	= gChatManager->getPlayerByAccId(message->getUint32());
	if(targetPlayer== NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupUnInvite: target not found");
		return;
	}

	if(targetPlayer->getGroupMemberIndex() != 0xFFFF)
	{
		// target has already join the group
		return;
	}

	if(player->getGroupId() == 0)
	{
		return;
	}

	if(player->getGroupId() != targetPlayer->getGroupId())
	{
		// not the same group
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		return;
	}

	if(player != group->getLeader())
	{
		gChatMessageLib->sendSystemMessage(player,L"@group:must_be_leader");
		return;
	}

	group->removeTempMember(targetPlayer);

	gChatMessageLib->sendGroupSystemMessage(targetPlayer->getName(), BString("uninvite_self"), player, NULL);
	gChatMessageLib->sendGroupSystemMessage(player->getName(), BString("uninvite_target"),targetPlayer, NULL);

}

//======================================================================================================================

void GroupManager::_processGroupDisband(Message* message, DispatchClient* client)
{

	Player* player = gChatManager->getPlayerByAccId(client->getAccountId());

	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupDisband: player not found\n");
	}

	if(player->getGroupMemberIndex() == 0xFFFF)
	{
		// target hasnt accepted the invite yet
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());
	if(group == NULL)
	{
		return;
	}

	if(player != group->getLeader())
	{
		//sendSystemMessage(player, L"@group:must_be_leader");
		group->removeMember(player);
		return;
	}

	group->disband();

}

//======================================================================================================================

void GroupManager::_processGroupLeave(Message* message, DispatchClient* client)
{
	Player* player = gChatManager->getPlayerByAccId(client->getAccountId());

	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupLeave: player not found\n");
	}

	if(player->getGroupMemberIndex() == 0xFFFF)
	{
		// target hasnt accepted the invite yet
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		return;
	}

	group->removeMember(player);

}

//======================================================================================================================

void GroupManager::_processGroupDismissGroupMember(Message* message, DispatchClient* client)
{

	Player* player			= gChatManager->getPlayerByAccId(client->getAccountId());
	Player* targetPlayer	= gChatManager->getPlayerByAccId(message->getUint32());


	if(targetPlayer == NULL || player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupDismissGroupMember player not found");
		return;
	}

	if(targetPlayer->getGroupMemberIndex() == 0xFFFF)
	{
		// target hasnt accepted the invite yet
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		return;
	}

	if(targetPlayer ==  player)
	{
		return;
	}

	if(player != group->getLeader())
	{
		gChatMessageLib->sendSystemMessage(player,  L"@group:must_be_leader");
	}

	group->removeMember(targetPlayer);

}

//======================================================================================================================

void GroupManager::_processGroupMakeLeader(Message* message, DispatchClient* client)
{

	Player* player			= gChatManager->getPlayerByAccId(client->getAccountId());
	Player* targetPlayer	= gChatManager->getPlayerByAccId(message->getUint32());


	if(targetPlayer == NULL || player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupMakeLeader player not found");
		return;
	}

	if(targetPlayer ==  player)
	{
		return;
	}

	if(targetPlayer->getGroupMemberIndex() == 0xFFFF)
	{
		// target hasnt accepted the invite yet
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		return;
	}

	if(player != group->getLeader())
	{
		gChatMessageLib->sendSystemMessage(player, L"@group:must_be_leader");
		return;
	}

	group->changeLeader(targetPlayer);

}

//======================================================================================================================

void GroupManager::_processGroupPositionNotification(Message* message, DispatchClient* client)
{

	Player* player			= gChatManager->getPlayerByAccId(client->getAccountId());

	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupZoneNotification player not found\n");
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		return;
	}

	player->setPositionX(message->getFloat());
	player->setPositionZ(message->getFloat());


	group->broadcastPlayerPositionNotification(player);
}

//======================================================================================================================
// someone needs some baselines here! 
void GroupManager::_processGroupBaselineRequest(Message* message, DispatchClient* client)
{

	Player* player = gChatManager->getPlayerByAccId(client->getAccountId());

	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupBaselineRequest player not found\n");
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		gChatMessageLib->sendSystemMessage(player, L"@group:disbanded");
		gChatMessageLib->sendIsmGroupCREO6deltaGroupId(0,player);
		return;
	}

	player->setPositionX(message->getFloat());
	player->setPositionZ(message->getFloat());

	group->sendCreate(player);

}

//======================================================================================================================
// someone requests to change the loot mode
void GroupManager::_processGroupLootModeRequest(Message* message, DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"_processGroupLootModeRequest\n");
	Player* player = gChatManager->getPlayerByAccId(client->getAccountId());

	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupBaselineRequest player not found\n");
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		gChatMessageLib->sendSystemMessage(player, L"@group:disbanded");
		gChatMessageLib->sendIsmGroupCREO6deltaGroupId(0,player);
		return;
	}

	if(player != group->getLeader())
	{
		gChatMessageLib->sendSystemMessage(player, mLootModeNotAllowedMessages[group->getLootMode()]);
		return;
	}

	gChatMessageLib->sendIsmGroupLootModeResponse(player);
	// ok to change loot mode

}

//======================================================================================================================
// Leader has chosen a new mode
void GroupManager::_processGroupLootModeResponse(Message* message, DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"_processGroupLootModeResponse\n");

	Player* player = gChatManager->getPlayerByAccId(client->getAccountId());
	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupBaselineRequest player not found\n");
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		gChatMessageLib->sendSystemMessage(player, L"@group:disbanded");
		gChatMessageLib->sendIsmGroupCREO6deltaGroupId(0,player);
		return;
	}

	if(player != group->getLeader())
	{
		gChatMessageLib->sendSystemMessage(player, L"@group:must_be_leader");
		return;
	}

	// set and send
	group->changeLootMode(message->getUint32());
	gChatMessageLib->sendGroupSystemMessage(mLootModeMessages[group->getLootMode()], group, true);

}

//======================================================================================================================
// someone wants to set master looter
void GroupManager::_processGroupLootMasterRequest(Message* message, DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"_processGroupLootMasterRequest\n");

	Player* player = gChatManager->getPlayerByAccId(client->getAccountId());
	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupBaselineRequest player not found\n");
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		gChatMessageLib->sendSystemMessage(player, L"@group:disbanded");
		gChatMessageLib->sendIsmGroupCREO6deltaGroupId(0,player);
		return;
	}

	if(player != group->getLeader())
	{
		gChatMessageLib->sendSystemMessage(player, L"@group:must_be_leader");
		return;
	}

	gChatMessageLib->sendIsmGroupLootMasterResponse(player);

}

//======================================================================================================================
// Leader choosen a new master looter
void GroupManager::_processGroupLootMasterResponse(Message* message, DispatchClient* client)
{
	gLogger->log(LogManager::DEBUG,"_processGroupLootMasterResponse");

	Player* player			= gChatManager->getPlayerByAccId(client->getAccountId());
	Player* targetPlayer	= gChatManager->getPlayerByAccId(message->getUint32());


	if(targetPlayer == NULL || player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processGroupLootMasterResponse player not found");
		return;
	}

	if(targetPlayer->getGroupId() != player->getGroupId())
	{
		return;
	}

	if(targetPlayer->getGroupMemberIndex() == 0xFFFF)
	{
		// target hasnt accepted the invite yet
		return;
	}

	GroupObject* group = getGroupById(player->getGroupId());

	if(group == NULL)
	{
		gChatMessageLib->sendSystemMessage(player, L"@group:disbanded");
		gChatMessageLib->sendIsmGroupCREO6deltaGroupId(0,player);
		return;
	}

	if(player != group->getLeader())
	{
		gChatMessageLib->sendSystemMessage(player, L"@group:leader_only");
		return;
	}

	if(targetPlayer->getCharId() == group->getMasterLooter())
	{
		// no change needed
		return;
	}

	group->changeMasterLooter(targetPlayer);
	gChatMessageLib->sendGroupSystemMessage(targetPlayer->getName(), BString("set_new_master_looter"), NULL, group);
}

//======================================================================================================================

void GroupManager::_processIsmInviteInRangeResponse(Message* message, DispatchClient* client)
{
	Player* player			= gChatManager->getPlayerByAccId(message->getUint32());
	Player* targetPlayer	= gChatManager->getPlayerByAccId(message->getUint32());
	uint8	inRange			= message->getUint8();

	if(targetPlayer == NULL || player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"GroupManager::_processIsmInviteInRangeResponse player not found");
		return;
	}

	// If we are not in range, inform the player and return.
	if(!inRange)
	{
		gChatMessageLib->sendSystemMessage(player, L"@group:out_of_range_suffix");
		return;
	}

	// I must be the group leader and group not full before we bother checking target...
	GroupObject* group = NULL;
	uint64 groupId = player->getGroupId();

	if (groupId != 0)
	{
		group = getGroupById(player->getGroupId());
		if (group == NULL)
		{
			return;
		}
		// Sender in group.

		// if sender is not leader
		if(group->getLeader() != player)
		{
			gChatMessageLib->sendSystemMessage(player,L"@group:must_be_leader");
			return;
		}

		// is it full?
		if(group->getMemberCount() >= 20)
		{
			gChatMessageLib->sendSystemMessage(group->getLeader(),L"@group:full");
			return;
		}
	}
	
	// If target have me ignored, auto decline my invitation.
	string ignoreName = player->getName();
	ignoreName.toLower();

	// check our ignorelist
	if (targetPlayer->checkIgnore(ignoreName.getCrc()))
	{
		gChatMessageLib->sendGroupSystemMessage(targetPlayer->getName(), BString("decline_leader"), player, NULL);
		return;
	}


	// if target is member of a group already
	if(targetPlayer->getGroupId() != 0 && targetPlayer->getGroupMemberIndex() != 0xFFFF)
	{
		if(targetPlayer->getGroupId() == groupId)
		{
			gChatMessageLib->sendSystemMessage(player,L"This player is already in your group.");
		}
		else
		{
			gChatMessageLib->sendGroupSystemMessage(targetPlayer->getName(), BString("already_grouped"), player, NULL);
		}
		return;
	}


	// if target in group and is considering to join a group
	if ((targetPlayer->getGroupMemberIndex() == 0xFFFF) && (targetPlayer->getGroupId() != 0))
	{
		// considering to join your group
		if(targetPlayer->getGroupId() == player->getGroupId())
		{
			gChatMessageLib->sendGroupSystemMessage(targetPlayer->getName(), BString("considering_your_group"), player, NULL);
		}

		// considering to join another group
		else
		{
			gChatMessageLib->sendGroupSystemMessage(targetPlayer->getName(), BString("considering_other_group"), player, NULL);
		}
		return;
	}


	// the sender is not in a group, lets create a new one
	// and insert it in the group map
	if (groupId == 0)
	{
		groupId = this->getNextGroupId();
		group = new GroupObject(player, groupId); 
		mGroups.insert(std::make_pair(groupId, group));
	}

	// add the target player as temp member
	group->addTempMember(targetPlayer);

	gChatMessageLib->sendGroupSystemMessage(targetPlayer->getName(), BString("invite_leader"), player, NULL);

	// tell the zone to display the invite box
	gChatMessageLib->sendIsmInviteRequest(player, targetPlayer);

}