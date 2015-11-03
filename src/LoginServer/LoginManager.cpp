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

#include "LoginManager.h"


#ifdef ERROR
#undef ERROR
#endif
#include "Utils/logger.h"

#include "AccountData.h"
#include "LoginClient.h"

#include "NetworkManager/Session.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include "Utils/bstring.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>


//======================================================================================================================
LoginManager::LoginManager(Database* database) :
    mDatabase(database),
    // mClock(0),
    mSendServerList(false),
    mLastStatusQuery(0),
    mLastHeartbeat(0),
    mNumClientsProcessed(0),
    mLoginClientPool(sizeof(LoginClient))
{
}

//======================================================================================================================
LoginManager::~LoginManager(void)
{
    LoginClientList::iterator iter = mLoginClientList.begin();

    while(iter != mLoginClientList.end())
    {
        iter = mLoginClientList.erase(iter);
    }
}

//======================================================================================================================
void LoginManager::Process(void)
{
    // Update our galaxy status list once in a while.
    if ((Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastStatusQuery) > 5000)
    {
        mLastStatusQuery = Anh_Utils::Clock::getSingleton()->getLocalTime();
        mDatabase->executeProcedureAsync(this, (void*)1, "CALL %s.sp_ReturnGalaxyStatus;",mDatabase->galaxy());
    }

    // Heartbeat once in awhile
    if ((Anh_Utils::Clock::getSingleton()->getLocalTime() - mLastHeartbeat) > 180000)//main loop every 10mins
    {
        mLastHeartbeat = Anh_Utils::Clock::getSingleton()->getLocalTime();
        LOG(INFO) << "LoginServer Heartbeat. Total clients (non-unique) processed since boot [" << mNumClientsProcessed << "]";
    }
}

//======================================================================================================================
NetworkClient* LoginManager::handleSessionConnect(Session* session, Service* service)
{
    LoginClient* newClient = new(mLoginClientPool.malloc()) LoginClient();

    newClient->setSession(session);
    newClient->setState(LCSTATE_ServerHelloSent);

    mLoginClientList.push_back(newClient);
    mNumClientsProcessed++;

    return newClient;
}

//======================================================================================================================
void LoginManager::handleSessionDisconnect(NetworkClient* client)
{
    LoginClient* loginClient = reinterpret_cast<LoginClient*>(client);

    // Client has disconnected.  Update the db to show they are no longer authenticated.
    mDatabase->executeProcedureAsync(0, 0, "UPDATE %s.account SET account_authenticated = 0 WHERE account_id=%u;",mDatabase->galaxy(), loginClient->getAccountId());

    LoginClientList::iterator iter = mLoginClientList.begin();

    while(iter != mLoginClientList.end())
    {
        if(loginClient == (*iter))
        {
            mLoginClientPool.free(*iter);
            mLoginClientList.erase(iter);
            break;
        }

        ++iter;
    }
}

//======================================================================================================================
void LoginManager::handleSessionMessage(NetworkClient* client, Message* message)
{
    LoginClient* loginClient = reinterpret_cast<LoginClient*>(client);

    // Grab the opcode from the message
    uint32 msgType = message->getUint32();   // Opcode

    switch (msgType)
    {
    case opLoginClientId:  // sent username and password.
    {
        // Start the login process
        DLOG(INFO) << "opLoginClientId";

        _handleLoginClientId(loginClient, message);
        break;
    }
    case opLauncherSessionOpen:  //Launcher has opened a session
    {
        //Handle the auth, make and return a session_key for them to use
        _handleLauncherSession(loginClient, message);
        break;
    }
    case opDeleteCharacterMessage:
    {
        _processDeleteCharacter(message,loginClient);
    }
    break;

    default:
    {
        // Unknown or unhandled message sent from client.
        //   int jack=0;
        break;
    }
    }

    // We need to destroy the incoming message for the session here
    // We want the application to decide whether the message is needed further or not.
    // This is mainly used in the ConnectionServer since routing messages need a longer life than normal
    client->getSession()->DestroyIncomingMessage(message);
}

//======================================================================================================================
void LoginManager::handleDatabaseJobComplete(void* ref, DatabaseResult* result)
{
    // Stupid simple hack since we don't have a client for this query
    if (ref == (void*)1)
    {
        _updateServerStatus(result);
        return;
    }

    // This assumes only authentication calls are async right now.  Will change as needed.
    LoginClient* client = (LoginClient*)ref;

    switch (client->getState())
    {
    case LCSTATE_QueryAuth:
    {
        // Check authentication
        _authenticateClient(client, result);
        break;
    }
    case LCSTATE_QueryServerList:
    {
        // Auth was successful, now send the server list.
        _sendServerList(client, result);

        // Execute our query
        client->setState(LCSTATE_QueryCharacterList);
        mDatabase->executeProcedureAsync(this, ref, "CALL %s.sp_ReturnAccountCharacters(%u);",mDatabase->galaxy(), client->getAccountId());
       
        break;
    }
    case LCSTATE_QueryCharacterList:
    {
        // Query done for the character list, send it out.
        _sendCharacterList(client, result);
        break;
    }
    case LCSTATE_DeleteCharacter:
    {
        // TODO: check returncodes, when using sf
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);

        uint32 queryResult;
        result->getNextRow(binding,&queryResult);

        uint32 deleteFailed = 1;
        if (queryResult == 1)
        {
            deleteFailed = 0;
        }
        mDatabase->destroyDataBinding(binding);

        _sendDeleteCharacterReply(deleteFailed,client);

        // _sendDeleteCharacterReply(0,client);
    }
    case LCSTATE_RetrieveAccountId:
    {
        //check to see if we have an account id
        _getLauncherSessionKey(client, result);
        break;
    }
    case LCSTATE_RetrieveSessionKey:
    {
        //should have a session key now
        _sendLauncherSessionKey(client, result);
        break;
    }
    break;

    default:
        break;
    }
}


//======================================================================================================================
void LoginManager::_handleLoginClientId(LoginClient* client, Message* message)
{
    // Extract our username, password, and id string
    BString username, password, clientId;
    message->getStringAnsi(username);
    message->getStringAnsi(password);
    message->getStringAnsi(clientId);

    if(strcmp("20050408-18:00",clientId.getAnsi()) != 0)
    {
        LOG(WARNING) << "illegal client: " << clientId.getAnsi();
        client->Disconnect(0);
        return;
    }
    
    LOG(INFO) << "Login request for account: [" << username.getAnsi() << "]";

    client->setUsername(username);
    client->setPassword(password);

    int8 sql[512],*sqlPointer;

    if (strlen(username.getAnsi()) == 0) //SessionID Login With ANH Launcher
    {
        sprintf(sql,"SELECT account_id, account_username, account_password, account_station_id, account_banned, account_active,account_characters_allowed,"
                "account_session_key, account_csr FROM account WHERE account_banned=0 AND account_authenticated=0 AND account_loggedin=0 AND account_session_key='");
        // sprintf(sql,"SELECT account_id, account_username, account_password, account_station_id, account_banned, account_active,account_characters_allowed,"
        //"account_session_key, account_csr FROM account WHERE account_banned=0 AND account_authenticated=0 AND account_loggedin=0 AND account_session_key=' AND session_key='");
        sqlPointer = sql + strlen(sql);
        sqlPointer += mDatabase->escapeString(sqlPointer,password.getAnsi(),password.getLength());
        *sqlPointer++ = '\'';
        *sqlPointer++ = '\0';
    }
    else //	regular login the client login screen
    {
        //the problem with the marked authentication is, that if a connection drops without a sessiondisconnect packet
        //the connection to the loginserver cannot be established anymore - need to have the sessiontimeout think of that

        sprintf(sql,"CALL %s.sp_ReturnUserAccount('",mDatabase->galaxy());

        sqlPointer = sql + strlen(sql);
        sqlPointer += mDatabase->escapeString(sqlPointer,username.getAnsi(),username.getLength());
        strcat(sql,"' , '");
        sqlPointer = sql + strlen(sql);
        sqlPointer += mDatabase->escapeString(sqlPointer,password.getAnsi(),password.getLength());
        *sqlPointer++ = '\'';
        *sqlPointer++ = ')';
        *sqlPointer++ = '\0';
    }

// Setup an async query for checking authentication.
    client->setState(LCSTATE_QueryAuth);
    mDatabase->executeProcedureAsync(this,client,sql);
    
}

//======================================================================================================================
void LoginManager::_authenticateClient(LoginClient* client, DatabaseResult* result)
{
    AccountData data;
    
    // This DataBinding code I'm not sure where to put atm.  I've thought about a base class for any objects that want
    // DataBinding, but I don't want to go overboard on abstraction.  Any suggestions would be appreciated.  :)
    DataBinding* binding = mDatabase->createDataBinding(8);
    binding->addField(DFT_int64, offsetof(AccountData, mId), 8);
    binding->addField(DFT_string, offsetof(AccountData, mUsername), 32);
    binding->addField(DFT_string, offsetof(AccountData, mPassword), 32);
    binding->addField(DFT_uint32, offsetof(AccountData, mAccountId), 4);
    binding->addField(DFT_uint8, offsetof(AccountData, mBanned), 1);
    binding->addField(DFT_uint8, offsetof(AccountData, mActive), 1);
    binding->addField(DFT_uint32,offsetof(AccountData, mCharsAllowed), 4);
    binding->addField(DFT_uint8, offsetof(AccountData, mCsr), 1);
        
    if (result->getRowCount())
    {
        result->getNextRow(binding, (void*)&data);
        client->setAccountId(data.mId);
        client->setCharsAllowed(data.mCharsAllowed);
        client->setCsr(data.mCsr);
        
        LOG(INFO) << "Login: AccountId: " << data.mId << " Name: " << data.mUsername;

        _sendAuthSucceeded(client);
    }
    else
    {
        Message* newMessage;

        BString errType, errMsg;
        errType = "@cpt_login_fail";
        errMsg = "@msg_login_fail";

        LOG(WARNING) << " Login failed for username: " <<  client->getUsername().getAnsi() << " password: ********" << client->getPassword().getAnsi();

        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opErrorMessage);
        gMessageFactory->addString(errType);
        gMessageFactory->addString(errMsg);
        gMessageFactory->addUint8(0);

        newMessage = gMessageFactory->EndMessage();

        client->SendChannelA(newMessage, 3,false);
        client->Disconnect(6);
    }

    // Destroy our database object
    mDatabase->destroyDataBinding(binding);
}

//======================================================================================================================
void LoginManager::_sendAuthSucceeded(LoginClient* client)
{
    const uint8 data[56] =
    {
        0x20, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00,
        0x0E, 0xD6, 0x93, 0xDE, 0xD2, 0xEF, 0xBF, 0x8E,
        0xA1, 0xAC, 0xD2, 0xEE, 0x4C, 0x55, 0xBE, 0x30,
        0x5F, 0xBE, 0x23, 0x0D, 0xB4, 0xAB, 0x58, 0xF9,
        0x62, 0x69, 0x79, 0x67, 0xE8, 0x10, 0x6E, 0xD3,
        0x86, 0x9B, 0x3A, 0x4A, 0x1A, 0x72, 0xA1, 0xFA,
        0x8F, 0x96, 0xFF, 0x9F, 0xA5, 0x62, 0x5A, 0x29
    };

    gMessageFactory->StartMessage();                    // opcode group?
    gMessageFactory->addUint32(opLoginClientToken);
    gMessageFactory->addUint32(60);                               // Size of the data
    gMessageFactory->addData((int8*)&data, sizeof(data));
    gMessageFactory->addUint32(client->getAccountId());
    gMessageFactory->addUint32(0);
    gMessageFactory->addString(client->getUsername());

    Message* message = gMessageFactory->EndMessage();
    client->SendChannelA(message, 4,false);

    // Update the account record so we know they authenticated properly.
    mDatabase->executeSqlAsync(0, 0, "UPDATE %s.account SET account_authenticated = 1 WHERE account_id = %u;",mDatabase->galaxy(), client->getAccountId());

    // Execute our query for sending the server list.
    client->setState(LCSTATE_QueryServerList);
    mDatabase->executeProcedureAsync(this, (void*)client, "CALL %s.sp_ReturnServerList;",mDatabase->galaxy());
}

//======================================================================================================================
void LoginManager::_sendServerList(LoginClient* client, DatabaseResult* result)
{
    uint32 serverCount = 1;

    ServerData data;
    memset(&data, 0, sizeof(ServerData));

    // This DataBinding code I'm not sure where to put atm.
    DataBinding* binding = mDatabase->createDataBinding(7);
    binding->addField(DFT_uint32, offsetof(ServerData, mId), 4);
    binding->addField(DFT_bstring, offsetof(ServerData, mName), 50);
    binding->addField(DFT_bstring, offsetof(ServerData, mAddress), 100);
    binding->addField(DFT_uint16, offsetof(ServerData, mConnectionPort), 2);
    binding->addField(DFT_uint16, offsetof(ServerData, mPingPort), 2);
    binding->addField(DFT_uint32, offsetof(ServerData, mPopulation), 4);
    binding->addField(DFT_uint32, offsetof(ServerData, mStatus), 4);

    serverCount = static_cast<uint32>(result->getRowCount());

    // Create our server list message
    gMessageFactory->StartMessage();                           // Group number
    gMessageFactory->addUint32(opLoginEnumCluster);
    gMessageFactory->addUint32(serverCount);                      // Number of servers in the list.
    for (uint32 i = 0; i < serverCount; i++)
    {
        result->getNextRow(binding, (void*)&data);
        gMessageFactory->addUint32(data.mId);                       // Server Id.
        gMessageFactory->addString(data.mName);                     // Server name
        gMessageFactory->addUint32(0xffff8f80);
    }
    gMessageFactory->addUint32(client->getCharsAllowed());

    // Send our message;
    Message* message = gMessageFactory->EndMessage();
    client->SendChannelA(message, 3, false);

    // Now send the current status.
    _sendServerStatus(client);

    // Destroy our database object stuff thigns
    mDatabase->destroyDataBinding(binding);
}

//======================================================================================================================
void LoginManager::_sendCharacterList(LoginClient* client, DatabaseResult* result)
{
    CharacterInfo data;
    memset(&data, 0, sizeof(CharacterInfo));

    // This DataBinding code I'm not sure where to put atm.  I've thought about a base class for any objects that want
    // DataBinding, but I don't want to go overboard on abstraction.  Any suggestions would be appreciated.  :)
    DataBinding* binding = mDatabase->createDataBinding(5);
    binding->addField(DFT_uint64, offsetof(CharacterInfo, mCharacterId), 8);
    binding->addField(DFT_bstring, offsetof(CharacterInfo, mFirstName), 64);
    binding->addField(DFT_bstring, offsetof(CharacterInfo, mLastName), 64);
    binding->addField(DFT_uint32, offsetof(CharacterInfo, mServerId), 4);
    binding->addField(DFT_bstring, offsetof(CharacterInfo, mBaseModel), 64);

    uint32 charCount = static_cast<uint32>(result->getRowCount());

    // Create our server list message
    gMessageFactory->StartMessage();                                 // Opcode group number
    gMessageFactory->addUint32(opEnumerateCharacterId);
    gMessageFactory->addUint32(charCount);                            // Character count
    for (uint32 i = 0; i < charCount; i++)
    {
        result->getNextRow(binding, &data);

        // Append first and last names
        BString fullName, baseModel;

        fullName << data.mFirstName.getAnsi();

        if(data.mLastName.getLength())
        {
            fullName << " ";
        //    fullName << data.mLastName.getAnsi();
        }

        fullName.convert(BSTRType_Unicode16);

        gMessageFactory->addString(fullName);                           // Characters full name

        baseModel = data.mBaseModel;
        gMessageFactory->addUint32(baseModel.getCrc());                 // Base model
        gMessageFactory->addUint64(data.mCharacterId);                  // Character Id
        gMessageFactory->addUint32(data.mServerId);                     // Server Id
        gMessageFactory->addUint32(1);                                  // Uknown
    }
    Message* message = gMessageFactory->EndMessage();
    client->SendChannelA(message, 2, false);

    // Destroy our data binding
    mDatabase->destroyDataBinding(binding);
}

//======================================================================================================================
void LoginManager::_processDeleteCharacter(Message* message,LoginClient* client)
{
    /*uint32 galaxyId = */message->getUint32();
    uint64 characterId = message->getUint64();

    client->setState(LCSTATE_DeleteCharacter);
    mDatabase->executeSqlAsync(this,(void*)client,"SELECT %s.sf_CharacterDelete(\'%" PRIu64 "\')",mDatabase->galaxy(), characterId);
    
}

//======================================================================================================================
void LoginManager::_sendDeleteCharacterReply(uint32 result,LoginClient* client)
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opDeleteCharacterReplyMessage);
    gMessageFactory->addUint32(result);
    Message* newMessage = gMessageFactory->EndMessage();

    client->SendChannelA(newMessage, 2, false);

    // Set the account authenticated to 0 (the server will attempt to relogin for any further processing).
    // Set the state to the end state to prevent character deletion infinite loop.
    client->setState(LCSTATE_End);
    mDatabase->executeProcedureAsync(0, 0, "UPDATE %s.account SET account_authenticated = 0 WHERE account_id = %u;",mDatabase->galaxy(), client->getAccountId());
 
}

//======================================================================================================================
void LoginManager::_sendServerStatus(LoginClient* client)
{
    // Create our server list message
    gMessageFactory->StartMessage();										// Opcode group number
    gMessageFactory->addUint32(opLoginClusterStatus);						// Opcode
    gMessageFactory->addUint32(mServerDataList.size());					// Server count

    ServerDataList::iterator iter;
    for (iter = mServerDataList.begin(); iter != mServerDataList.end(); iter++)
    {
        gMessageFactory->addUint32((*iter)->mId);							// Server id
        gMessageFactory->addString((*iter)->mAddress);						// Server address
        gMessageFactory->addUint16((*iter)->mConnectionPort);				// Connection port
        gMessageFactory->addUint16((*iter)->mPingPort);						// Ping port
        gMessageFactory->addUint32((*iter)->mPopulation);					// Population
        gMessageFactory->addUint32(0x00000cb2);								//
        gMessageFactory->addUint32(client->getCharsAllowed());				// Characters Allowed
        gMessageFactory->addUint32(0xffff8f80);
        if((*iter)->mStatus==3 && client->getCsr()>0)						// server status 0=offline, 1=loading, 2=online, 3=locked
        {
            gMessageFactory->addUint32(2);
        } else {
            gMessageFactory->addUint32((*iter)->mStatus);
        }
        gMessageFactory->addUint8(0);
    }


    Message* message = gMessageFactory->EndMessage();

    client->SendChannelA(message, 3, false);
}

//======================================================================================================================
void LoginManager::_updateServerStatus(DatabaseResult* result)
{
    uint32 serverCount = 1;

    ServerData data;
    memset(&data, 0, sizeof(ServerData));

    // This DataBinding code I'm not sure where to put atm.
    DataBinding* binding = mDatabase->createDataBinding(8);
    binding->addField(DFT_uint32, offsetof(ServerData, mId), 4);
    binding->addField(DFT_bstring, offsetof(ServerData, mName), 50);
    binding->addField(DFT_bstring, offsetof(ServerData, mAddress), 100);
    binding->addField(DFT_uint16, offsetof(ServerData, mConnectionPort), 2);
    binding->addField(DFT_uint16, offsetof(ServerData, mPingPort), 2);
    binding->addField(DFT_uint32, offsetof(ServerData, mPopulation), 4);
    binding->addField(DFT_uint32, offsetof(ServerData, mStatus), 4);
    binding->addField(DFT_uint32, offsetof(ServerData, mLastUpdate), 4);

    serverCount = static_cast<uint32>(result->getRowCount());

    for (uint32 i = 0; i < serverCount; i++)
    {
        result->getNextRow(binding, &data);

        ServerDataList::iterator iter = mServerDataList.begin();

        while(iter != mServerDataList.end())
        {
            // If it's this server and it has been updated, then update our local copy.
            if ((*iter)->mId == data.mId)
            {
                if ((*iter)->mLastUpdate < data.mLastUpdate)
                {
                    *(*iter) = data;
                    mSendServerList = true;
                }
                break;
            }

            ++iter;
        }

        // If we hit the end of the list, it means we need to add this server
        if (iter == mServerDataList.end())
        {
            ServerData* newData = new ServerData();
            *newData = data;
            mServerDataList.push_back(newData);
        }
    }

    // Destroy our database object stuff thigns
    mDatabase->destroyDataBinding(binding);

    if (mSendServerList)
    {
        LoginClientList::iterator it = mLoginClientList.begin();

        while(it != mLoginClientList.end())
        {
            _sendServerStatus((*it));

            ++it;
        }

        mSendServerList = false;
    }
}

// Launcher Functions
//======================================================================================================================
void LoginManager::_handleLauncherSession(LoginClient* client, Message* message)
{
    // Extract our username, password, and id string
    BString username, password, clientId;
    message->getStringAnsi(username);
    message->getStringAnsi(password);
    message->getStringAnsi(clientId);

    if(strcmp("20090610-18:00",clientId.getAnsi()) != 0)
    {
        LOG(WARNING) << "illegal launcher: " << clientId.getAnsi();
        client->Disconnect(0);
        return;
    }

    client->setUsername(username);
    client->setPassword(password);

    int8 sql[512];

    //call the session_key creation sproc
    sprintf(sql,"SELECT account_id FROM %s.account WHERE account_username='%s' AND account_password = SHA1('%s');",mDatabase->galaxy(), client->getUsername().getAnsi(), client->getPassword().getAnsi());

    //set the state
    client->setState(LCSTATE_RetrieveAccountId);

    //and execute
    mDatabase->executeProcedureAsync(this, client, sql);
    
}

//======================================================================================================================
void LoginManager::_getLauncherSessionKey(LoginClient* client, DatabaseResult* result)
{
    AccountData data;
    DataBinding* binding = mDatabase->createDataBinding(1);
    binding->addField(DFT_int64, offsetof(AccountData, mId), 8);

    if (result->getRowCount())
    {
        //we have the account id
        result->getNextRow(binding, (void*)&data);
        client->setAccountId(data.mId);

        //log it
        DLOG(INFO) << "void LoginManager::_sendLauncherSessionKey Login: AccountId: " << data.mId<< " Name: " << client->getUsername().getAnsi();

        //get the session_key made and returned
        int8 sql[512];
        sprintf(sql,"CALL %s.sp_AccountSessionKeyGenerate(%" PRIu64 ");",mDatabase->galaxy(), data.mId);

        client->setState(LCSTATE_RetrieveSessionKey);
        mDatabase->executeProcedureAsync(this, client, sql);
    
    }
    else
    {
        Message* newMessage;

        BString errType, errMsg;
        errType = "@cpt_login_fail";
        errMsg = "@msg_login_fail";

        DLOG(INFO) << " Login failed for username: "  << client->getUsername().getAnsi() <<", password: ********" << client->getPassword().getAnsi();

        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opErrorMessage);
        gMessageFactory->addString(errType);
        gMessageFactory->addString(errMsg);
        gMessageFactory->addUint8(0);

        newMessage = gMessageFactory->EndMessage();

        client->SendChannelA(newMessage, 3,false);
        client->Disconnect(6);
    }

    // Destroy our database object
    mDatabase->destroyDataBinding(binding);
}

//======================================================================================================================
void LoginManager::_sendLauncherSessionKey(LoginClient* client, DatabaseResult* result)
{
    SessionKeyData data;
    DataBinding* binding = mDatabase->createDataBinding(1);
    binding->addField(DFT_bstring, offsetof(SessionKeyData, mSessionKey),32);

    result->getNextRow(binding, (void*)&data);

    //start the message
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opLauncherSessionCreated);
    gMessageFactory->addUint32(32);
    gMessageFactory->addString(data.mSessionKey);

    //and send it
    Message* message = gMessageFactory->EndMessage();
    client->SendChannelA(message,4,false);

    //disconnect this client now
    client->Disconnect(6);
}


//======================================================================================================================
