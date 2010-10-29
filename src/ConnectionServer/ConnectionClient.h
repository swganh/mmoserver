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

#ifndef ANH_CONNECTIONSERVER_CONNECTIONCLIENT_H
#define ANH_CONNECTIONSERVER_CONNECTIONCLIENT_H

#include "NetworkManager/NetworkClient.h"


//======================================================================================================================

enum ConnectionClientState
{
    CCSTATE_QueryAuth,
    CCSTATE_QueryChars,
    CCSTATE_AllowedChars,
    CCState_End
};

//======================================================================================================================

class ConnectionClient : public NetworkClient
{
public:

    ConnectionClient(void) : mAccountId(0), mServerId(0) {};
    ~ConnectionClient(void) {};

    ConnectionClientState         getState(void)                            {
        return mState;
    }
    uint32                        getAccountId(void)                        {
        return mAccountId;
    }
    uint32                        getServerId(void)                         {
        return mServerId;
    }
    uint32						  getCharsAllowed(void)						{
        return mCharsAllowed;
    }
    uint32						  getCurrentChars(void)						{
        return mCurrentChars;
    }

    void                          setState(ConnectionClientState state)     {
        mState = state;
    }
    void                          setAccountId(uint32 id)                   {
        mAccountId = id;
    }
    void                          setServerId(uint32 id)                    {
        mServerId = id;
    }
    void                          setCharsAllowed(uint32 chars)				{
        mCharsAllowed = chars;
    }
    void                          setCurrentChars(uint32 currentChars)      {
        mCurrentChars = currentChars;
    }

private:

    ConnectionClientState	mState;
    uint32					mCharsAllowed;
    uint32					mCurrentChars;
    uint32                  mAccountId;
    uint32                  mServerId;
};

//======================================================================================================================

#endif //MMOSERVER_CONNECTIONSERVER_CONNECTIONCLIENT_H

