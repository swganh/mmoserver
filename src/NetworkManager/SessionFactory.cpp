/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "SessionFactory.h"
#include "NetConfig.h"
#include "Session.h"
#include "LogManager/LogManager.h"


//======================================================================================================================

SessionFactory::SessionFactory(void) 
: mSessionIdNext(0)
{

}

//======================================================================================================================

SessionFactory::~SessionFactory(void)
{

}


//======================================================================================================================

void SessionFactory::Startup(GameService* service, PacketFactory* packetFactory, MessageFactory* messageFactory)
{
  mService				= service;
  mPacketFactory		= packetFactory;
  mMessageFactory		= messageFactory;

}

//======================================================================================================================

void SessionFactory::Shutdown(void)
{
	SessionPool::purge_memory();
	
}

//======================================================================================================================

void SessionFactory::Process(void)
{

}

//======================================================================================================================

Session* SessionFactory::CreateSession(void)
{
	Session* session = new(SessionPool::malloc()) Session();
	session->setService(mService);
	session->setPacketFactory(mPacketFactory);
	session->setMessageFactory(mMessageFactory);
	session->setId(mSessionIdNext++);

	uint16 unreliable = gNetConfig->getServerClientUnReliableSize();
	uint16 reliable = gNetConfig->getServerClientReliableSize();

	if(reliable > MAX_CLIENT_PACKET_SIZE)
		reliable = MAX_CLIENT_PACKET_SIZE;
	if(unreliable > MAX_CLIENT_PACKET_SIZE)
		unreliable = MAX_CLIENT_PACKET_SIZE;

	session->setPacketSize(reliable);
	session->setUnreliableSize(unreliable);

	return session;
}

//======================================================================================================================

void SessionFactory::DestroySession(Session* session)
{
	session->~Session();
	SessionPool::free(session);
}

//======================================================================================================================




