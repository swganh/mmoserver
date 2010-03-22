/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CHARACTERLOGINHANDLER_H
#define ANH_ZONESERVER_CHARACTERLOGINHANDLER_H

#include "ObjectFactoryCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "MathLib/Vector3.h"
#include "Common/MessageDispatchCallback.h"

#include <boost/thread/recursive_mutex.hpp>

//======================================================================================================================

class Message;
class Database;
class MessageDispatch;
class PlayerObject;

//======================================================================================================================

enum CLHCallBack
{
	CLHCallBack_None				= 0,
	CLHCallBack_Transfer_Ticket		= 1,
	CLHCallBack_Transfer_Position	= 2
};

class CharacterLoadingContainer
{
public:

	DispatchClient*				mClient;
	ObjectFactoryCallback*		ofCallback;
	DatabaseCallback*			dbCallback;
	uint64						mPlayerId;
	Anh_Math::Vector3			destination;
	uint16						planet;
	PlayerObject*				player;
	CLHCallBack					callBack;
};

class CharacterLoginHandler : public MessageDispatchCallback,public ObjectFactoryCallback, public DatabaseCallback
{
	public:

        CharacterLoginHandler(Database* database, MessageDispatch* dispatch);
        ~CharacterLoginHandler(void);

		// DatabaseCallback
		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		  // Inherited from MessageDispatchCallback
		virtual void	handleDispatchMessage(uint32 opcode, Message* message, DispatchClient* client);

		  // ObjectFactoryCallback
		virtual void	handleObjectReady(Object* object,DispatchClient* client);

	private:

		void    _processClusterClientDisconnect(Message* message, DispatchClient* client);
		void    _processClusterZoneTransferApprovedByTicket(Message* message, DispatchClient* client);
		void    _processClusterZoneTransferApprovedByPosition(Message* message, DispatchClient* client);
		void    _processClusterZoneTransferDenied(Message* message, DispatchClient* client);

		Database*					mDatabase;
		MessageDispatch*			mMessageDispatch;

		uint32						mZoneId;
        boost::recursive_mutex		mSessionMutex;
};




#endif // ANH_ZONESERVER_CHARACTERLOGINHANDLER_H



