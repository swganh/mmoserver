/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"

#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "LogManager/LogManager.h"

#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"


//======================================================================================================================
//
// request zone transfer through travel ticket
// 
bool MessageLib::sendClusterZoneTransferRequestByTicket(PlayerObject* playerObject,uint64 ticketId,uint32 destinationPlanet)
{
	if(!playerObject || !playerObject->isConnected())
	{
		return(false);
	}

	mMessageFactory->StartMessage();          
	mMessageFactory->addUint32(opClusterZoneTransferRequestByTicket);  
	mMessageFactory->addUint32(destinationPlanet);
	mMessageFactory->addUint64(ticketId);

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Connection, 0);

	return(true);
}

//======================================================================================================================
//
// request zone transfer through admin command
// 
bool MessageLib::sendClusterZoneTransferRequestByPosition(PlayerObject* playerObject, const glm::vec3& position, uint32 destinationPlanet)
{
	if(!playerObject || !playerObject->isConnected())
	{
		return(false);
	}

	mMessageFactory->StartMessage();          
	mMessageFactory->addUint32(opClusterZoneTransferRequestByPosition);  
	mMessageFactory->addUint32(destinationPlanet);
	mMessageFactory->addFloat(position.x);
	mMessageFactory->addFloat(position.z);

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Connection,0);

	return(true);
}

//======================================================================================================================
//
// zone transfer through travel ticket
// 
bool MessageLib::sendClusterZoneTransferCharacter(PlayerObject* playerObject, uint32 destinationPlanet)
{
	if(!playerObject || !playerObject->isConnected())
	{
		return(false);
	}

	mMessageFactory->StartMessage();          
	mMessageFactory->addUint32(opClusterZoneTransferCharacter);  
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(destinationPlanet);

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Connection, 0);

	return(true);
}

//======================================================================================================================
//
// group baselines
// 
bool MessageLib::sendIsmGroupBaselineRequest(PlayerObject* targetPlayer)
{
	if (!_checkPlayer(targetPlayer))
	{
		return(false);
	}

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opIsmGroupBaselineRequest);
	mMessageFactory->addFloat(targetPlayer->mPosition.x);
	mMessageFactory->addFloat(targetPlayer->mPosition.z);

	targetPlayer->getClient()->SendChannelA(mMessageFactory->EndMessage(), targetPlayer->getAccountId(), CR_Chat, 2);

	return(true);
}

//======================================================================================================================
//
// group leave
// 
bool MessageLib::sendIsmGroupLeave(PlayerObject* player)
{
	if (!_checkPlayer(player))
	{
		return(false);
	}

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opIsmGroupLeave);  

	player->getClient()->SendChannelA(mMessageFactory->EndMessage(),player->getAccountId(),CR_Chat,2);

	return(true);
}

//======================================================================================================================
//
// group position
//
bool MessageLib::sendIsmGroupPositionNotification(PlayerObject* targetPlayer)
{
	if (!_checkPlayer(targetPlayer))
	{
		return(false);
	}

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opIsmGroupPositionNotification);
	mMessageFactory->addFloat(targetPlayer->mPosition.x);
	mMessageFactory->addFloat(targetPlayer->mPosition.z);

	targetPlayer->getClient()->SendChannelA(mMessageFactory->EndMessage(),targetPlayer->getAccountId(), CR_Chat, 2);

	return(true);
}

//======================================================================================================================
//
// group invite inrange response
//
bool MessageLib::sendIsmGroupInviteInRangeResponse(PlayerObject *sender, PlayerObject *target, bool inRange)
{
	if(!_checkPlayer(sender) || !_checkPlayer(target))
	{
		return(false);
	}

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opIsmGroupInviteInRangeResponse);
	mMessageFactory->addUint32(sender->getAccountId());
	mMessageFactory->addUint32(target->getAccountId());
	mMessageFactory->addUint8(inRange);

	sender->getClient()->SendChannelA(mMessageFactory->EndMessage(), sender->getAccountId(), CR_Chat, 2);

	return(true);
}

//======================================================================================================================
//
// report location to chatserver
//
bool MessageLib::sendFindFriendLocation(PlayerObject* friendPlayer, uint64 friendId,uint64 player, float X, float Z)
{
	if(!friendPlayer || !friendPlayer->isConnected())
	{
		return(false);
	}

	mMessageFactory->StartMessage();      
	mMessageFactory->addUint32(opFindFriendSendPosition);  
	mMessageFactory->addUint64(friendId);  
	mMessageFactory->addUint64(player);  
	mMessageFactory->addFloat(X);  
	mMessageFactory->addFloat(Z);  

	(friendPlayer->getClient())->SendChannelA(mMessageFactory->EndMessage(), friendPlayer->getAccountId(), CR_Chat, 4);

	return(true);
}

//======================================================================================================================
//
// Dust off for Banktips
//
bool MessageLib::sendBankTipDustOff(PlayerObject* playerObject, uint64 tipRecipient, uint32 amount, string recipientName)
{
	if(!playerObject || !playerObject->isConnected())
	{
		return(false);
	}

	mMessageFactory->StartMessage();      
	mMessageFactory->addUint32(opBankTipDustOff);  
	mMessageFactory->addUint64(tipRecipient);  
	mMessageFactory->addUint64(playerObject->getId());  
	mMessageFactory->addUint32(amount);  
	mMessageFactory->addString(recipientName);  
	
	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Chat, 4);

	return(true);
}

//======================================================================================================================
//
// group loot mode update
//
bool MessageLib::sendGroupLootModeResponse(PlayerObject* playerObject,uint32 selection)
{
	if(!playerObject || !playerObject->isConnected())
	{
		return(false);
	}

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opIsmGroupLootModeResponse);
	mMessageFactory->addUint32(selection);

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(),CR_Chat,2);

	return(true);
}

//======================================================================================================================

bool MessageLib::sendGroupLootMasterResponse(PlayerObject* masterLooter, PlayerObject* playerObject)
{
	if(!masterLooter || !playerObject || !playerObject->isConnected())
	{
		return(false);
	}

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opIsmGroupLootMasterResponse);
	mMessageFactory->addUint32(masterLooter->getAccountId());

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Chat,2);

	return(true);
}

//======================================================================================================================

