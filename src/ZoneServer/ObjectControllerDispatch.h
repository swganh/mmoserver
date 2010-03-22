/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_OBJECTCONTROLLERDISPATCH_H
#define ANH_ZONESERVER_OBJECTCONTROLLERDISPATCH_H

#include "Common/MessageDispatchCallback.h"

#define ObjController object->getController()

//======================================================================================================================

class Message;
class Database;
class MessageDispatch;

//======================================================================================================================

class ObjectControllerDispatch : public MessageDispatchCallback
{
	public:

		ObjectControllerDispatch(Database* database,MessageDispatch* dispatch);
		~ObjectControllerDispatch();

		void			Process();

		virtual void	handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client);

	private:

		void			_dispatchMessage(Message* message,DispatchClient* client);
		void			_dispatchObjectMenuSelect(Message* message,DispatchClient* client);

		Database*					mDatabase;
		MessageDispatch*			mMessageDispatch;
};

#endif 



