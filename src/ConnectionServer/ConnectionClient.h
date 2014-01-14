/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CONNECTIONSERVER_CONNECTIONCLIENT_H
#define ANH_CONNECTIONSERVER_CONNECTIONCLIENT_H

#include "NetworkManager/NetworkClient.h"


//======================================================================================================================

enum ConnectionClientState
{
	CCSTATE_QueryAuth,
	CCState_End
};

//======================================================================================================================

class ConnectionClient : public NetworkClient
{
	public:

		ConnectionClient(void) : mAccountId(0), mServerId(0) {};
		~ConnectionClient(void) {};

		ConnectionClientState         getState(void)                            { return mState; }
		uint32                        getAccountId(void)                        { return mAccountId; }
		uint32                        getServerId(void)                         { return mServerId; }

		void                          setState(ConnectionClientState state)     { mState = state; }
		void                          setAccountId(uint32 id)                   { mAccountId = id; }
		void                          setServerId(uint32 id)                    { mServerId = id; }

	private:

		ConnectionClientState	mState;
		uint32                  mAccountId;
		uint32                  mServerId;
};

//======================================================================================================================

#endif //MMOSERVER_CONNECTIONSERVER_CONNECTIONCLIENT_H

