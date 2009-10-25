/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "BankTerminal.h"
#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Datapad.h"
#include "Item.h"
#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "SurveyTool.h"
#include "TravelMapHandler.h"
#include "UIManager.h"
#include "WaypointObject.h"
#include "WorldManager.h"
#include "WorldConfig.h"
#include "Wearable.h"
#include "ZoneOpcodes.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"

//======================================================================================================================
//
// add friend
//

void ObjectController::_handleAddFriend(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player		= dynamic_cast<PlayerObject*>(mObject);

	if(player->getContactListUpdatePending())
		return;
	else
		player->setContactListUpdatePending(true);

	string	friendName;
	int8	sql[1024],end[16],*sqlPointer;

	message->getStringUnicode16(friendName);
	friendName.convert(BSTRType_ANSI);

	if(!friendName.getLength())
	{
		player->setContactListUpdatePending(false);
		return;
	}

	if(player->isConnected())
		gMessageLib->sendHeartBeat(player->getClient());

	friendName.toLower();

	// check if he's already our friend
	if(player->checkFriendList(friendName.getCrc()))
	{
		friendName.convert(BSTRType_Unicode16);
		gMessageLib->sendSystemMessage(player,L"","cmnty","friend_duplicate","","",L"",0,"","",friendName.getUnicode16());
		player->setContactListUpdatePending(false);
		return;
	}

	// or ignored
	
	if(player->checkIgnoreList(friendName.getCrc()))
	{
		friendName.convert(BSTRType_Unicode16);
		gMessageLib->sendSystemMessage(player,L"","cmnty","friend_fail_is_ignored","","",L"",0,"","",friendName.getUnicode16());
		player->setContactListUpdatePending(false);
		return;
	}

	// check our own name
	string firstName = player->getFirstName().getAnsi();
	firstName.toLower();

	if(strcmp(firstName.getAnsi(),friendName.getAnsi()) == 0)
	{
		player->setContactListUpdatePending(false);
		return;
	}

	// pull the db query
	ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_AddFriend);
	asyncContainer->mString = friendName.getAnsi();

	sprintf(sql,"SELECT sf_addFriend(%"PRId64",'",player->getId());
	sprintf(end,"')");
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,friendName.getAnsi(),friendName.getLength());
	strcat(sql,end);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

}

//======================================================================================================================
//
// remove friend
//

void ObjectController::_handleRemoveFriend(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(player->getContactListUpdatePending())
		return;
	else
		player->setContactListUpdatePending(true);

	string	friendName;
	int8	sql[1024],end[16],*sqlPointer;

	message->getStringUnicode16(friendName);
	friendName.convert(BSTRType_ANSI);

	if(!friendName.getLength())
	{
		player->setContactListUpdatePending(false);
		return;
	}


	if(player->isConnected())
		gMessageLib->sendHeartBeat(player->getClient());

	friendName.toLower();

	if(!player->checkFriendList(friendName.getCrc()))
	{
		player->setContactListUpdatePending(false);	
		return;
	}
	ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_RemoveFriend);
	asyncContainer->mString = friendName.getAnsi();

	sprintf(sql,"SELECT sf_removeFriend(%"PRId64",'",player->getId());
	sprintf(end,"')");
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,friendName.getAnsi(),friendName.getLength());
	strcat(sql,end);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

		
}

//======================================================================================================================
//
// add ignore
//

void ObjectController::_handleAddIgnore(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(player->getContactListUpdatePending())
	{
		return;
	}
	else
	{
		player->setContactListUpdatePending(true);
	}

	string	ignoreName;
	int8	sql[2048],end[16],*sqlPointer;

	message->getStringUnicode16(ignoreName);
	ignoreName.convert(BSTRType_ANSI);

	if(!ignoreName.getLength())
	{
		player->setContactListUpdatePending(false);
		return;
	}

	if(player->isConnected())
		gMessageLib->sendHeartBeat(player->getClient());

	ignoreName.toLower();

	// check our ignorelist
	if(player->checkIgnoreList(ignoreName.getCrc()))
	{
		ignoreName.convert(BSTRType_Unicode16);
		gMessageLib->sendSystemMessage(player,L"","cmnty","ignore_duplicate","","",L"",0,"","",ignoreName.getUnicode16());
		player->setContactListUpdatePending(false);
		return;
	}

	// friends CAN be ignored!!!
	//if(player->checkFriendList(ignoreName.getCrc()))
	//{
	//	ignoreName.convert(BSTRType_Unicode16);
	//	gMessageLib->sendSystemMessage(player,L"","cmnty","friend_fail_is_ignored","","",L"",0,"","",ignoreName);
	//	player->setContactListUpdatePending(false);
	//	return;
	//}

	// check our own name
	string firstName = player->getFirstName().getAnsi();
	firstName.toLower();

	if(strcmp(firstName.getAnsi(),ignoreName.getAnsi()) == 0)
	{
		player->setContactListUpdatePending(false);
		return;
	}

	// pull the db query
	ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_AddIgnore);
	asyncContainer->mString = ignoreName.getAnsi();

	sprintf(sql,"SELECT sf_addIgnore(%"PRId64",'",player->getId());
	sprintf(end,"')");
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,ignoreName.getAnsi(),ignoreName.getLength());
	strcat(sql,end);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	
}

//======================================================================================================================
//
// remove ignore
//

void ObjectController::_handleRemoveIgnore(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	if(player->getContactListUpdatePending())
		return;
	else
		player->setContactListUpdatePending(true);

	string	ignoreName;
	int8	sql[2048],end[16],*sqlPointer;

	message->getStringUnicode16(ignoreName);
	ignoreName.convert(BSTRType_ANSI);

	if(!ignoreName.getLength())
	{
		player->setContactListUpdatePending(false);
		return;
	}

	if(player->isConnected())
		gMessageLib->sendHeartBeat(player->getClient());

	ignoreName.toLower();

	if(!player->checkIgnoreList(ignoreName.getCrc()))
	{
		player->setContactListUpdatePending(false);
		return;
	}

	ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_RemoveIgnore);
	asyncContainer->mString = ignoreName.getAnsi();

	sprintf(sql,"SELECT sf_removeIgnore(%"PRId64",'",player->getId());
	sprintf(end,"')");
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,ignoreName.getAnsi(),ignoreName.getLength());
	strcat(sql,end);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	
}

//======================================================================================================================
//
// add friend db reply
//

void ObjectController::_handleAddFriendDBReply(uint32 retCode,string friendName)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	switch(retCode)
	{
		// no such name
		case 0:
		default:
		{
			friendName.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(player,L"","cmnty","friend_not_found","","",L"",0,"","",friendName.getUnicode16());
	
		}
		break;

		// add ok
		case 1:
		{
			// update list
			player->addFriend(friendName.getAnsi());
			gMessageLib->sendFriendListPlay9(player);

			// send notification
			friendName.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(player,L"","cmnty","friend_added","","",L"",0,"","",friendName.getUnicode16());

			// notify chat server
			if(player->isConnected())
			{
				gMessageFactory->StartMessage();
				gMessageFactory->addUint32(opNotifyChatAddFriend);
				gMessageFactory->addString(friendName);
				Message* message = gMessageFactory->EndMessage();

				player->getClient()->SendChannelA(message,player->getAccountId(),CR_Chat,2);
			}
		}
		break;
	}

	player->setContactListUpdatePending(false);
}

//======================================================================================================================
//
// add friend db reply
//

void ObjectController::_handleFindFriendDBReply(uint64 retCode,string friendName)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);
	friendName.convert(BSTRType_Unicode16);
	if(retCode == 0)
	{
		gMessageLib->sendSystemMessage(player,L"","cmnty","friend_location_failed_noname","","",L"",0,"","",friendName.getUnicode16());
		return;
	}

	PlayerObject*	searchObject	= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(retCode));

	if(!searchObject)
	{
		gMessageLib->sendSystemMessage(player,L"","cmnty","friend_location_failed","","",L"",0,"","",friendName.getUnicode16());
		return;
	}

	//are we on our targets friendlist???
	if(!searchObject->checkFriendList(player->getFirstName().getCrc()))
	{
		gMessageLib->sendSystemMessage(player,L"","cmnty","friend_location_failed","","",L"",0,"","",friendName.getUnicode16());
		return;
	}

	Datapad* thePad = dynamic_cast<Datapad*>(searchObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));

	if(thePad && thePad->getCapacity())
	{
		thePad->requestNewWaypoint(searchObject->getFirstName().getAnsi(),searchObject->mPosition,static_cast<uint16>(gWorldManager->getZoneId()),Waypoint_blue);
	}
}

//======================================================================================================================
//
// remove friend db reply
//

void ObjectController::_handleRemoveFriendDBReply(uint32 retCode,string friendName)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	switch(retCode)
	{
		// no such name
		case 0:
		default:
		{
			friendName.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(player,L"","cmnty","friend_not_found","","",L"",0,"","",friendName.getUnicode16());
		}
		break;

		// remove ok
		case 1:
		{
			// update list
			player->removeFriend(friendName.getCrc());
			gMessageLib->sendFriendListPlay9(player);

			// send notification
			friendName.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(player,L"","cmnty","friend_removed","","",L"",0,"","",friendName.getUnicode16());

			if(player->isConnected())
			{
				// notify chat server
				gMessageFactory->StartMessage();
				gMessageFactory->addUint32(opNotifyChatRemoveFriend);
				gMessageFactory->addString(friendName);
				Message* message = gMessageFactory->EndMessage();

				player->getClient()->SendChannelA(message,player->getAccountId(),CR_Chat,2);
			}
		}
		break;
	}

	player->setContactListUpdatePending(false);
}


//======================================================================================================================
//
// add ignore db reply
//

void ObjectController::_handleAddIgnoreDBReply(uint32 retCode,string ignoreName)
{
	
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	// gLogger->logMsgF("_handleAddIgnoreDBReply retCode = %u",MSG_NORMAL, retCode);

	switch(retCode)
	{
		// no such name
		case 0:
		default:
		{
			ignoreName.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(player,L"","cmnty","ignore_not_found","","",L"",0,"","",ignoreName.getUnicode16());
		}
		break;

		// add ok
		case 1:
		{
			// update list
			player->addIgnore(ignoreName.getAnsi());
			gMessageLib->sendIgnoreListPlay9(player);

			// send notification
			ignoreName.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(player,L"","cmnty","ignore_added","","",L"",0,"","",ignoreName.getUnicode16());

			// notify chat server
			if(player->isConnected())
			{
				gMessageFactory->StartMessage();
				gMessageFactory->addUint32(opNotifyChatAddIgnore);
				gMessageFactory->addString(ignoreName);
				Message* message = gMessageFactory->EndMessage();

				player->getClient()->SendChannelA(message,player->getAccountId(),CR_Chat,2);
			}
		}
		break;
	}

	player->setContactListUpdatePending(false);
}

//======================================================================================================================
//
// remove ignore db reply
//

void ObjectController::_handleRemoveIgnoreDBReply(uint32 retCode,string ignoreName)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);
	// gLogger->logMsgF("_handleRemoveIgnoreDBReply retCode = %u",MSG_NORMAL, retCode);

	switch(retCode)
	{
		// no such name
		case 0:
		default:
		{
			ignoreName.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(player,L"","cmnty","ignore_not_found","","",L"",0,"","",ignoreName.getUnicode16());
		}
		break;

		// remove ok
		case 1:
		{
			// update list
			player->removeIgnore(ignoreName.getCrc());
			gMessageLib->sendIgnoreListPlay9(player);

			// send notification
			ignoreName.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(player,L"","cmnty","ignore_removed","","",L"",0,"","",ignoreName.getUnicode16());

			// notify chat server
			if(player->isConnected())
			{
				gMessageFactory->StartMessage();
				gMessageFactory->addUint32(opNotifyChatRemoveIgnore);
				gMessageFactory->addString(ignoreName);
				Message* message = gMessageFactory->EndMessage();

				player->getClient()->SendChannelA(message,player->getAccountId(),CR_Chat,2);
			}
		}
		break;
	}

	player->setContactListUpdatePending(false);
}

//======================================================================================================================

//

//======================================================================================================================
//
// find a friend
//

void ObjectController::_handlefindfriend(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	string			friendName;
	int8			sql[1024],end[16],*sqlPointer;

	message->getStringUnicode16(friendName);

	if(!friendName.getLength())
	{
		gMessageLib->sendSystemMessage(playerObject,L"","ui_cmnty","friend_location_failed_usage","","",L"",0,"","",L"");
		return;
	}
	
	if(playerObject->isConnected())
	{
		// query the chat server
		gMessageFactory->StartMessage();
		gMessageFactory->addUint32(opNotifyChatFindFriend);
		gMessageFactory->addString(friendName);
		Message* message = gMessageFactory->EndMessage();

		playerObject->getClient()->SendChannelA(message,playerObject->getAccountId(),CR_Chat,2);
		
	}
	return;


	string unicodeName = friendName;
	friendName.convert(BSTRType_ANSI);


	// check if he's our friend
	if(!playerObject->checkFriendList(friendName.getCrc()))
	{
		gMessageLib->sendSystemMessage(playerObject,L"","cmnty","friend_not_found","","",L"",0,"","",unicodeName.getUnicode16());
		return;
	}

	
	// pull the db query
	ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_FindFriend);
	asyncContainer->mString = friendName.getAnsi();

	sprintf(sql,"SELECT id from swganh.characters where firstname like '");
	sprintf(end,"'");
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,friendName.getAnsi(),friendName.getLength());
	strcat(sql,end);

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

	
}

