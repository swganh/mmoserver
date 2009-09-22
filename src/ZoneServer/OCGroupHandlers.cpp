/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "TravelMapHandler.h"
#include "BankTerminal.h"
#include "CurrentResource.h"
#include "WorldConfig.h"
#include "UIManager.h"
#include "Item.h"
#include "SurveyTool.h"
#include "Wearable.h"
#include "NPCObject.h"
#include "CraftingTool.h"


//======================================================================================================================
//
// invite
//

void ObjectController::_handleInvite(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	// lets get the target player
	message->setIndex(32);
	PlayerObject* target_player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(message->getInt64()));

	// if  target is valid
	if(target_player == NULL || target_player == player)
	{
		gMessageLib->sendSystemMessage(player,L"","group","invite_no_target_self");
		return;
	}

	gMessageFactory->StartMessage();
	Message* newMessage;
	gMessageFactory->addUint32(opIsmGroupInviteRequest);
	gMessageFactory->addUint32(target_player->getAccountId());
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);	
}

//======================================================================================================================
//
// uninvite
//

void ObjectController::_handleUninvite(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	// lets get the target player
	message->setIndex(32);
	PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(message->getInt64()));


	// if  target is valid
	if(targetPlayer == NULL || targetPlayer == player)
	{
		gMessageLib->sendSystemMessage(player,L"","group","uninvite_no_target_self");
		return;
	}

	// we advise the chat server
	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupUnInvite);  
	gMessageFactory->addUint32(targetPlayer->getAccountId());
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
}

//======================================================================================================================
//
// join
//

void ObjectController::_handleJoin(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	// resetting the sender's id 
	gMessageLib->sendInviteSenderUpdateDeltasCreo6(0,player);

	// we advise the chat server that he accepted
	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupInviteResponse);  
	gMessageFactory->addUint8(1);
	gMessageFactory->addFloat(player->mPosition.mX);
	gMessageFactory->addFloat(player->mPosition.mZ);
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
}

//======================================================================================================================
//
// decline
//

void ObjectController::_handleDecline(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	// resetting the sender's id 
	gMessageLib->sendInviteSenderUpdateDeltasCreo6(0,player);

	gMessageLib->sendSystemMessage(player, L"","group","decline_self");

	// we advise the chat server that he refused
	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupInviteResponse);  
	gMessageFactory->addUint8(0);
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
	player->setGroupId(0);
}

//======================================================================================================================
//
// disband
//

void ObjectController::_handleDisband(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	if(player->getGroupId() == 0)
	{
		return;
	}	

	// we advise the chat server about the disband
	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupDisband);  
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
}

//======================================================================================================================
//
// leave group
//

void ObjectController::_handleLeaveGroup(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	if(player->getGroupId() == 0)
	{
		return;
	}	

	// we advise the chat server about it
	gMessageLib->sendIsmGroupLeave(player);
}

//======================================================================================================================
//
// make leader
//

void ObjectController::_handleMakeLeader(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	if(player->getGroupId() == 0)
	{
		return;
	}	

	// lets get the target player
	message->setIndex(32);
	PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(message->getInt64()));


	// if  target is valid
	if(targetPlayer == NULL || targetPlayer->getGroupId() != player->getGroupId())
	{
		gMessageLib->sendSystemMessage(player,L"Invalid Target.");
		return;
	}

	// we advise the chat server about it
	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupMakeLeader);
	gMessageFactory->addUint32(targetPlayer->getAccountId());
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
}

//======================================================================================================================
//
// dismiss member
//

void ObjectController::_handleDismissGroupMember(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	// make sure its a fully grouped player
	if(player->getGroupId() == 0)
	{
		return;
	}	

	// lets get the target player
	message->setIndex(32);
	PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(message->getInt64()));


	// if  target is valid
	if(targetPlayer == NULL || targetPlayer->getGroupId() != player->getGroupId())
	{
		gMessageLib->sendSystemMessage(player,L"Invalid Target.");
		return;
	}

	// we advise the chat server about it
	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupDismissGroupMember);
	gMessageFactory->addUint32(targetPlayer->getAccountId());
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
}

//=============================================================================================================================
//
// group chat
//


void ObjectController::_handleGroupChat(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	string msgText;
 
	msgText.setType(BSTRType_Unicode16);
	msgText.setLength(512);

	//Honey bunnies!
	//this is like all handled by the Objectcontroller???
	//we just have to look at the Message here !!!!!!!!

	//gLogger->hexDump always shows the complete Message!!!!! the Objectcontroller however sets the Index to the first data
	//byte AFTER the Objectcontroller header has been dealt with !

	/*
	data32 = message->getUint32();	// object controller opcode
	data32 = message->getUint32();	// Unknown
	data32 = message->getUint32();	// command enqueue
	playerId = message->getUint64();// player id
	data32 = message->getUint32();	// Unknown
	uint32 requestId = message->getUint32();	// RequestID
	// data32 = message->getUint32();	// Unknown RequestID?????
	data32 = message->getUint32();	// command crc (crc of "groupchat")
	data64 = message->getUint64();	// empty id field
	*/
	message->getStringUnicode16(msgText);	// unicode string

	// make sure its a fully grouped player
	if (!player)
	{
		gLogger->logMsg("ObjectController::_handleGroupChat NO PLAYER\n");
	}

	if(!player->getGroupId())
	{

		gLogger->logMsg("ObjectController::_handleGroupChat NO GROUP");
	}

	// let the chatserver handle this.
	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupSay);
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(msgText);
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2); 
	//this should be fastpath as not being Mission critical and we want to prevent the communication protocol overhead with Acks and resends

	// Convert since we are going to print it.
	// msgText.convert(BSTRType_ANSI);
}


//=============================================================================================================================
//
// loot mode
//

void ObjectController::_handleGroupLootMode(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	// disabled for now
	//return;

	gLogger->logMsg("_handleGroupLootMode\n");
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	// make sure its a fully grouped player
	if(player->getGroupId() == 0)
	{
		return;
	}	

	// we advise the chat server about it
	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupLootModeRequest);
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
}

//=============================================================================================================================
//
// make master looter
//

void ObjectController::_handleMakeMasterLooter(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{	
	// disabled for now
	//return;

	gLogger->logMsg("_handleMakeMasterLooter");

	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	// make sure its a fully grouped player
	if(player->getGroupId() == 0)
	{
		return;
	}	

	// we advise the chat server about it
	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupLootMasterRequest);
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
}

//=============================================================================================================================


