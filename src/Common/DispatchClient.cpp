/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

//#include <WINSOCK2.h>
#include "DispatchClient.h"
#include "Message.h"
#include "NetworkManager/Session.h"


//======================================================================================================================

void DispatchClient::SendChannelA(Message* message, uint32 accountId, uint8 serverId, uint8 priority)
{
	message->setRouted(true);
	message->setAccountId(accountId);
	message->setDestinationId(serverId);
	NetworkClient::SendChannelA(message, priority, false);
	

}

void DispatchClient::SendChannelAUnreliable(Message* message, uint32 accountId, uint8 serverId, uint8 priority)
{
	message->setRouted(true);
	message->setAccountId(accountId);
	message->setDestinationId(serverId);
	NetworkClient::SendChannelAUnreliable(message, priority);
}

//======================================================================================================================

