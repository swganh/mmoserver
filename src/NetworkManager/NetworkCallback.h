/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_NETWORKCALLBACK_H
#define ANH_NETWORKMANAGER_NETWORKCALLBACK_H

//======================================================================================================================

class Session;
class Service;
class NetworkClient;
class Message;

//======================================================================================================================

class NetworkCallback
{
	public:

	  virtual NetworkClient*  handleSessionConnect(Session* session, Service* service)            { return (NetworkClient*)-1; };
	  virtual void            handleSessionDisconnect(NetworkClient* client)                      {};
	  virtual void            handleSessionMessage(NetworkClient* client, Message* message)       {};

	private:
};

//======================================================================================================================

#endif //ANH_NETWORKMANAGER_NETWORKCALLBACK_H



