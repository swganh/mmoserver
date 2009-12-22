/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "NetworkClient.h"
#include "Session.h"
#include "LogManager/LogManager.h"
#include "Common/Message.h"



//======================================================================================================================
void NetworkClient::SendChannelA(Message* message, uint8 priority, bool fastpath)
{ 
  message->setPriority(priority);
  message->setFastpath(fastpath);
  message->mSession = mSession;

 
  return mSession->SendChannelA(message);
}

void NetworkClient::SendChannelAUnreliable(Message* message, uint8 priority)
{ 
  message->setPriority(priority);
  message->setFastpath(true);

  return mSession->SendChannelAUnreliable(message);
}


//======================================================================================================================
void NetworkClient::Disconnect(uint8 reason)
{
  mSession->setCommand(SCOM_Disconnect);
}

