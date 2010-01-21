/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_LOGINSERVER_LOGINMANAGER_H
#define ANH_LOGINSERVER_LOGINMANAGER_H

#include "NetworkManager/NetworkCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/typedefs.h"

#include <boost/pool/pool.hpp>

#include <list>


//======================================================================================================================

class Database;
class LoginClient;
class ServerData;
class Message;

enum AuthResult
{
  AUTHRESULT_AuthFailed = 0,
  AUTHRESULT_AuthSucceded,
  AUTHRESULT_AuthBanned
};

typedef std::list<LoginClient*>      LoginClientList;
typedef std::list<ServerData*>       ServerDataList;

//======================================================================================================================

class CharacterInfo
{
	public:

	  uint64        mCharacterId;
	  int8          mFirstName[64];
	  int8          mLastName[64];
	  int8          mBaseModel[64];
	  uint32        mServerId;
};

//======================================================================================================================

class LoginManager : public NetworkCallback, public DatabaseCallback
{
	public:

                            LoginManager(void);
                            ~LoginManager(void);

		void                    Startup(Database* database);
		void                    Shutdown(void);
		void                    Process(void);

  // From NetworkCallback inheritance
		virtual NetworkClient*	handleSessionConnect(Session* session, Service* service);
		virtual void          	handleSessionDisconnect(NetworkClient* client);
		virtual void            handleSessionMessage(NetworkClient* client, Message* message);

  // Inherited from DatabaseCallback
		virtual void            handleDatabaseJobComplete(void* ref, DatabaseResult* result);

	private:
	
		void                    _processDeleteCharacter(Message* message,LoginClient* client);
		void                    _sendDeleteCharacterReply(uint32 result,LoginClient* client);


		void                    _handleLoginClientId(LoginClient* client, Message* message);
		void                    _authenticateClient(LoginClient* client, DatabaseResult* result);
		void                    _sendAuthSucceeded(LoginClient* client);
		void                    _sendCharacterList(LoginClient* client, DatabaseResult* result);
		void                    _sendServerList(LoginClient* client, DatabaseResult* result);
		void                    _sendServerStatus(LoginClient* client);
		void                    _updateServerStatus(DatabaseResult* result);


		Database*	            mDatabase;
		// Anh_Utils::Clock*       mClock;

		LoginClientList         mLoginClientList;
		ServerDataList          mServerDataList;
		bool                    mSendServerList;

		uint32                  mLastStatusQuery;

		boost::pool<boost::default_user_allocator_malloc_free>	mLoginClientPool;
};



#endif //MMOSERVER_LOGINSERVER_LOGINMANAGER_H



