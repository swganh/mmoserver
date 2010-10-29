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

#ifndef ANH_LOGINSERVER_LOGINCLIENT_H
#define ANH_LOGINSERVER_LOGINCLIENT_H

#include "NetworkManager/NetworkClient.h"


//======================================================================================================================
enum LoginClientState
{
    LCSTATE_ClientCreated,
    LCSTATE_ServerHelloSent,
    LCSTATE_QueryAuth,
    LCSTATE_QueryServerList,
    LCSTATE_QueryCharacterList,
    LCSTATE_DeleteCharacter,
    LCSTATE_UpdateServerStatus,
    LCSTATE_RetrieveAccountId,
    LCSTATE_RetrieveSessionKey,
    LCSTATE_End
};


//======================================================================================================================
class LoginClient : public NetworkClient
{
public:
    LoginClient(void) : mState(LCSTATE_ClientCreated) {};
    virtual                     ~LoginClient(void) {};

    LoginClientState            getState(void)                            {
        return mState;
    };
    uint32                      getAccountId(void)                        {
        return mAccountId;
    };
    BString&                     getUsername(void)                         {
        return mUsername;
    };
    BString&                     getPassword(void)                         {
        return mPassword;
    };
    uint32                      getCharsAllowed()                         {
        return mCharsAllowed;
    }
    int8						  getCsr()									{
        return mCsr;
    }

    void                        setState(LoginClientState state)          {
        mState = state;
    };
    void                        setAccountId(uint32 id)                   {
        mAccountId = id;
    };
    void                        setUsername(BString username)              {
        mUsername = username;
    };
    void                        setPassword(BString password)              {
        mPassword = password;
    };
    void                        setCharsAllowed(uint32 count)             {
        mCharsAllowed = count;
    }
    void                        setCsr(int8 csr)							{
        mCsr = csr;
    };
private:
    LoginClientState            mState;
    uint32                      mAccountId;
    BString                      mUsername;
    BString                      mPassword;
    uint32                      mCharsAllowed;
    int8						  mCsr;
};



#endif // ANH_LOGINSERVER_LOGINCLIENT_H



