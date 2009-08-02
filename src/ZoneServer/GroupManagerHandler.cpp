 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "GroupManagerHandler.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "UIManager.h"
#include "LogManager/LogManager.h"
#include "WorldManager.h"
#include "Utils/utils.h"

bool						GroupManagerHandler::mInsFlag    = false;
GroupManagerHandler*		GroupManagerHandler::mSingleton  = NULL;


//======================================================================================================================

GroupManagerHandler::GroupManagerHandler(Database* database, MessageDispatch* dispatch)
{	
	mDatabase = database;
	mMessageDispatch = dispatch;
	
	mMessageDispatch->RegisterMessageCallback(opIsmGroupInviteRequest,this);  
	mMessageDispatch->RegisterMessageCallback(opIsmGroupCREO6deltaGroupId,this);  
	mMessageDispatch->RegisterMessageCallback(opIsmGroupLootModeResponse,this); 
	mMessageDispatch->RegisterMessageCallback(opIsmGroupLootMasterResponse,this); 
}


//======================================================================================================================

GroupManagerHandler::~GroupManagerHandler()
{
	mInsFlag = false;
	delete(mSingleton);
}

//======================================================================================================================

GroupManagerHandler*	GroupManagerHandler::Init(Database* database, MessageDispatch* dispatch)
{
	if(!mInsFlag)
	{
		mSingleton = new GroupManagerHandler(database,dispatch);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
	
}

//======================================================================================================================

void GroupManagerHandler::Shutdown()
{
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupInviteRequest);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupCREO6deltaGroupId);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupLootModeResponse);
	mMessageDispatch->UnregisterMessageCallback(opIsmGroupLootMasterResponse);
}

//======================================================================================================================

void GroupManagerHandler::handleDispatchMessage(uint32 opcode, Message* message, DispatchClient* client)
{
	switch(opcode)
	{
	
		case opIsmGroupInviteRequest:
		{
			_processIsmInviteRequest(message);
		}
		break;

		case opIsmGroupCREO6deltaGroupId:
		{
			_processIsmGroupCREO6deltaGroupId(message);
		}
		break;

		case opIsmGroupLootModeResponse:
		{
			_processIsmGroupLootModeResponse(message);
		}
		break;

		case opIsmGroupLootMasterResponse:
		{
			_processIsmGroupLootMasterResponse(message);
		}
		break;

		default:
			gLogger->logMsgF("GroupManagerHandlerMessage::handleDispatchMessage: Unhandled opcode %u",MSG_NORMAL,opcode);
		break;
	} 
}

//=======================================================================================================================


void GroupManagerHandler::_processIsmInviteRequest(Message* message)
{
	
	PlayerObject* sender = gWorldManager->getPlayerByAccId(message->getUint32()); // the player who sent the invite
	PlayerObject* target = gWorldManager->getPlayerByAccId(message->getUint32());  // the player who will recieve it

	if(sender == NULL || target == NULL)
	{
		gLogger->logMsg("GroupManagerHandler::_processIsmInviteRequest PlayerAccId not found");
		return;	
	}

	//target->setGroupId(message->getUint64()); // the group id provided by the chatserver

	gMessageLib->sendInviteSenderUpdateDeltasCreo6(sender->getId(),target);
	
}

//=======================================================================================================================

void GroupManagerHandler::_processIsmGroupCREO6deltaGroupId(Message* message)
{
	// this packet is sent by the chatserver to update the group_id in the CREO6
	// it concerns the player itself and all the inrange players

	PlayerObject* const player = gWorldManager->getPlayerByAccId(message->getUint32());  // the player whos group_id has changed
	if(player == NULL)
	{
		gLogger->logMsg("GroupManagerHandler::_processIsmGroupCREO6deltaGroupId PlayerAccId not found");
		return;
	}
	
	player->setGroupId(message->getUint64());


	// to in-range folks
	const PlayerObjectSet*	const inRangePlayers	= player->getKnownPlayers();
	PlayerObjectSet::const_iterator	it				= inRangePlayers->begin();
	
	while(it != inRangePlayers->end())
	{
		const PlayerObject* const targetObject = (*it);

		if(targetObject->isConnected())
		{
			gMessageLib->sendGroupIdUpdateDeltasCreo6(player->getGroupId(),player,targetObject);
		}

		++it;
	}

	// to self
	gMessageLib->sendGroupIdUpdateDeltasCreo6(player->getGroupId(), player, player);

}

//=======================================================================================================================

void GroupManagerHandler::_processIsmGroupLootModeResponse(Message* message)
{
	gLogger->logMsg("_processIsmGroupLootModeResponse");
	PlayerObject* playerObject = gWorldManager->getPlayerByAccId(message->getUint32());  // the player whos group_id has changed
	if(playerObject == NULL)
	{
		gLogger->logMsg("GroupManagerHandler::processIsmGroupLootModeResponse PlayerAccId not found");
		return;
	}

	//send the SUI
	gLogger->logMsg("ok");

	BStringVector availableLootModes;
	availableLootModes.push_back("Free for all");
	availableLootModes.push_back("Master Looter");
	availableLootModes.push_back("Lottery");
	availableLootModes.push_back("Random");

	gUIManager->createNewListBox(playerObject,"handleSetLootMode","@group:set_loot_type_title","@group:set_loot_type_text",availableLootModes,playerObject,SUI_Window_SelectGroupLootMode_Listbox);
}

//=======================================================================================================================

void GroupManagerHandler::_processIsmGroupLootMasterResponse(Message* message)
{
	gLogger->logMsg("_processIsmGroupLootMasterResponse");
	PlayerObject* playerObject = gWorldManager->getPlayerByAccId(message->getUint32());  // the player whos group_id has changed
	if(playerObject == NULL)
	{
		gLogger->logMsg("GroupManagerHandler::_processIsmGroupLootMasterResponse PlayerAccId not found");
		return;
	}

	//send the SUI
	gLogger->logMsg("ok");

	PlayerList inRangeMembers	= playerObject->getInRangeGroupMembers(true);
	PlayerList::iterator it		= inRangeMembers.begin();

	BStringVector namesArray;

	while(it != inRangeMembers.end())
	{
		namesArray.push_back((*it)->getFirstName().getAnsi());
		++it;
	}

	gUIManager->createNewPlayerSelectListBox(playerObject,"handleSetLootMaster","@group:master_looter_sui_title","@group:set_loot_type_text",namesArray,inRangeMembers,playerObject);
}

//=======================================================================================================================
