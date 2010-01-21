/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CONNECTIONSERVER_CONNECTIONDISPATCHCLIENT_H
#define ANH_CONNECTIONSERVER_CONNECTIONDISPATCHCLIENT_H


//======================================================================================================================

class ConnectionDispatchClient
{
	public:
		
		ConnectionDispatchClient() : mAccountId(0) {};
		~ConnectionDispatchClient();

		void	setAccountId(uint32 id){ mAccountId = id; };

		uint32	getAccountId(void){ return mAccountId; };

	private:

		uint32	mAccountId;
};

//======================================================================================================================

#endif // ANH_CONNECTIONSERVER_CONNECTIONDISPATCHCLIENT_H



