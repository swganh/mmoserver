/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"
#include "ZoneServer/PlayerObject.h"
#include "LogManager/LogManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ObjectFactory.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"


//======================================================================================================================
//
// request zone transfer through travel ticket
// 

bool MessageLib::sendClusterZoneTransferRequestByTicket(PlayerObject* playerObject,uint64 ticketId,uint32 destinationPlanet)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opClusterZoneTransferRequestByTicket);  
	gMessageFactory->addUint32(destinationPlanet);
	gMessageFactory->addUint64(ticketId);
	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Connection, 0);

	return(true);
}

//======================================================================================================================
//
// request zone transfer through admin command
// 

bool MessageLib::sendClusterZoneTransferRequestByPosition(PlayerObject* playerObject,Anh_Math::Vector3 position,uint32 destinationPlanet)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opClusterZoneTransferRequestByPosition);  
	gMessageFactory->addUint32(destinationPlanet);
	gMessageFactory->addFloat(position.mX);
	gMessageFactory->addFloat(position.mZ);
	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage,playerObject->getAccountId(),CR_Connection,0);

	return(true);
}

//======================================================================================================================
//
// zone transfer through travel ticket
// 

bool MessageLib::sendClusterZoneTransferCharacter(PlayerObject* playerObject, uint32 destinationPlanet)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opClusterZoneTransferCharacter);  
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(destinationPlanet);
	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Connection, 0);

	return(true);
}

//======================================================================================================================
//
// group baselines
// 

void MessageLib::sendIsmGroupBaselineRequest(PlayerObject* targetPlayer)
{
	if (!_checkPlayer(targetPlayer))
		return;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupBaselineRequest);
	gMessageFactory->addFloat(targetPlayer->mPosition.mX);
	gMessageFactory->addFloat(targetPlayer->mPosition.mZ);
	Message* newMessage = gMessageFactory->EndMessage();
	targetPlayer->getClient()->SendChannelA(newMessage,targetPlayer->getAccountId(), CR_Chat, 2);
}

//======================================================================================================================
//
// group leave
// 

void MessageLib::sendIsmGroupLeave(PlayerObject* player)
{
	if (!_checkPlayer(player))
		return;

	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupLeave);  
	newMessage = gMessageFactory->EndMessage();
	player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
}

//======================================================================================================================
//
// group position
//

void MessageLib::sendIsmGroupPositionNotification(PlayerObject* targetPlayer)
{
	if (!_checkPlayer(targetPlayer))
		return;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupPositionNotification);
	gMessageFactory->addFloat(targetPlayer->mPosition.mX);
	gMessageFactory->addFloat(targetPlayer->mPosition.mZ);
	Message* newMessage = gMessageFactory->EndMessage();
	targetPlayer->getClient()->SendChannelA(newMessage,targetPlayer->getAccountId(), CR_Chat, 2);
}

//======================================================================================================================
//
// report location to chatserver
//

void MessageLib::sendFindFriendLocation(PlayerObject* friendPlayer, uint64 friendId,uint64 player, float X, float Z)
{
	if(!(friendPlayer->isConnected()))
		return;

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opFindFriendSendPosition);  
	gMessageFactory->addUint64(friendId);  
	gMessageFactory->addUint64(player);  
	gMessageFactory->addFloat(X);  
	gMessageFactory->addFloat(Z);  
	

	(friendPlayer->getClient())->SendChannelA(gMessageFactory->EndMessage(), friendPlayer->getAccountId(), CR_Chat, 4);

	return;
}

//======================================================================================================================
//
// Dust off for Banktips
//

void MessageLib::sendBankTipDustOff(PlayerObject* playerObject, uint64 tipRecipient, uint32 amount, string recipientName)
{
	if(!(playerObject->isConnected()))
		return;

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opBankTipDustOff);  
	gMessageFactory->addUint64(tipRecipient);  
	gMessageFactory->addUint64(playerObject->getId());  
	gMessageFactory->addUint32(amount);  
	gMessageFactory->addString(recipientName);  
	
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Chat, 4);

	return;
}

//======================================================================================================================
//
// group loot mode update
//

bool MessageLib::sendGroupLootModeResponse(PlayerObject* playerObject,uint32 selection)
{
	if(!(playerObject->isConnected()))
	{
		return(false);
	}

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmGroupLootModeResponse);
	gMessageFactory->addUint32(selection);

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(),CR_Chat,2);

	return(true);
}

//======================================================================================================================

