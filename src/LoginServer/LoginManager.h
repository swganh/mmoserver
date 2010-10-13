/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef ANH_LOGINSERVER_LOGINMANAGER_H
#define ANH_LOGINSERVER_LOGINMANAGER_H

#include <cstdint>

#include "NetworkManager/NetworkCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/typedefs.h"
#include "Utils/bstring.h"

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

struct CharacterInfo
{
    uint64_t        mCharacterId;
    BString          mFirstName;
    BString          mLastName;
    BString          mBaseModel;
    uint32        mServerId;
};

//======================================================================================================================

class LoginManager : public NetworkCallback, public DatabaseCallback
{
public:

    LoginManager(Database* database);
    ~LoginManager(void);

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

    //launcher functions
    void                    _handleLauncherSession(LoginClient* client, Message* message);
    void                    _getLauncherSessionKey(LoginClient* client, DatabaseResult* result);
    void                    _sendLauncherSessionKey(LoginClient* client, DatabaseResult* result);


    Database*	            mDatabase;
    // Anh_Utils::Clock*       mClock;

    LoginClientList         mLoginClientList;
    ServerDataList          mServerDataList;
    bool                    mSendServerList;

    uint64                  mLastStatusQuery;
    uint64                  mLastHeartbeat;
    uint32					mNumClientsProcessed;

    boost::pool<boost::default_user_allocator_malloc_free>	mLoginClientPool;
};



#endif //MMOSERVER_LOGINSERVER_LOGINMANAGER_H



