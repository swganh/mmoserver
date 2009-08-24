/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_COMMON_DISPATCHCLIENT_H
#define ANH_COMMON_DISPATCHCLIENT_H

#include "NetworkManager/NetworkClient.h"
#include "Utils/typedefs.h"


//======================================================================================================================

class DispatchClient : public NetworkClient
{
	public:
  
		virtual void	SendChannelA(Message* message, uint32 accountId, uint8 serverId, uint8 priority);
		virtual void	SendChannelAUnreliable(Message* message, uint32 accountId, uint8 serverId, uint8 priority);
		void			setAccountId(uint32 id){ mAccountId = id; };

		uint32			getAccountId(void){ return mAccountId; };

	private:

		uint32	mAccountId;
};

//======================================================================================================================

#endif //MMOSERVER_COMMON_DISPATCHCLIENT_H


