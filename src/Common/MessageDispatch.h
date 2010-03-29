/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_COMMON_MESSAGEDISPATCH_H
#define ANH_COMMON_MESSAGEDISPATCH_H

#include "NetworkManager/NetworkCallback.h"
#include "Utils/typedefs.h"

#include <boost/thread/recursive_mutex.hpp>
#include <map>


//======================================================================================================================

class IService;
class DispatchClient;
class MessageDispatchCallback;
class Message;

typedef std::map<uint32, MessageDispatchCallback*>   MessageCallbackMap;
typedef std::map<uint32, DispatchClient*>            AccountClientMap;


// This should be defined as a pre-set in DB account table.
// Maybe reserv the first 10 or 100 id's for internal usage?
// TODO: Find a better place to store stuff like this...
#define AdminAccountId ((uint32)(3000))

//======================================================================================================================

class MessageDispatch : public NetworkCallback
{
	public:

		MessageDispatch(IService* service);
		~MessageDispatch(void);

		void						Process(void);

		void						RegisterMessageCallback(uint32 opcode, MessageDispatchCallback* callback);
		void						UnregisterMessageCallback(uint32 opcode);
		AccountClientMap*			getClientMap(){return(&mAccountClientMap);}

		// Inherited NetworkCallback
		virtual NetworkClient*		handleSessionConnect(Session* session, IService* service);
		virtual void				handleSessionDisconnect(NetworkClient* client);
		virtual void				handleSessionMessage(NetworkClient* client, Message* message);

		// Sessionless clients
		void						registerSessionlessDispatchClient(uint32 accountId);
		void						unregisterSessionlessDispatchClient(uint32 accountId);
	private:

		IService*					mRouterService;
		MessageCallbackMap			mMessageCallbackMap;
		AccountClientMap			mAccountClientMap;
        boost::recursive_mutex		mSessionMutex;
};

//======================================================================================================================

#endif //MMOSERVER_COMMON_MESSAGEDISPATCH_H




