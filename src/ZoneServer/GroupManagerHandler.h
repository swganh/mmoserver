/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_GROUPMANAGERHANDLER_H
#define ANH_ZONESERVER_GROUPMANAGERHANDLER_H

#include "Common/MessageDispatchCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "ObjectFactory.h"
#include "ZoneOpcodes.h"

#include <vector>

//======================================================================================================================

#define	gGroupManagerHandler	GroupManagerHandler::getSingletonPtr()

//======================================================================================================================

class Message;
class Database;
class MessageDispatch;

//======================================================================================================================

class GroupManagerHandler : public MessageDispatchCallback, public DatabaseCallback
{
	public:

		static GroupManagerHandler*	getSingletonPtr() { return mSingleton; }
		static GroupManagerHandler*	Init(Database* database,MessageDispatch* dispatch);

		GroupManagerHandler(Database* database,MessageDispatch* dispatch);
		~GroupManagerHandler();

		void				Shutdown();

		virtual void		handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client);
		

	private:

		void				_processIsmInviteRequest(Message* message);
		void				_processIsmGroupCREO6deltaGroupId(Message* message);
		void				_processIsmGroupLootModeResponse(Message* message);
		void				_processIsmGroupLootMasterResponse(Message* message);

		static GroupManagerHandler*	mSingleton;
		static bool					mInsFlag;

		Database*					mDatabase;
		MessageDispatch*			mMessageDispatch;

		
};

#endif 

