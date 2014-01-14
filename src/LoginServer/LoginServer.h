/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_LOGINSERVER_ZONESERVER_H
#define ANH_LOGINSERVER_ZONESERVER_H

#include "Utils/typedefs.h"

//======================================================================================================================
class NetworkManager;
class Service;
class LoginManager;
class DatabaseManager;
class Database;


//======================================================================================================================
class LoginServer
{
	public:
		LoginServer(void);
		~LoginServer(void);

		void	Process(void);

private:
	NetworkManager*									mNetworkManager;
  Service*                        mService;
	DatabaseManager*								mDatabaseManager;
	Database*												mDatabase;
  LoginManager*                   mLoginManager;
};



#endif // ANH_LOGINSERVER_ZONESERVER_H


