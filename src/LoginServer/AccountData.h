/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_DATABASEMANAGER_ACCOUNTDATA_H
#define ANH_DATABASEMANAGER_ACCOUNTDATA_H

#include <cstdint>
#include "Utils/typedefs.h"
#include "Utils/bstring.h"

// These are for DB queries.
//======================================================================================================================
struct AccountData
{
    uint64_t            mId;
    int8                  mUsername[32];
    int8                  mPassword[32];
    uint32                mAccountId;
    int8                  mBanned;
    int8                  mActive;
    uint32                mCharsAllowed;
    int8					mCsr;
};


//======================================================================================================================
class ServerData
{
public:
    uint32                mId;
    BString                mName;
    BString                mAddress;
    uint16                mConnectionPort;
    uint16                mPingPort;
    uint32                mPopulation;
    uint32                mStatus;
    uint32                mLastUpdate;
};

//======================================================================================================================
// Session_Key is used by the launcher
//======================================================================================================================
class SessionKeyData
{
public:
    BString             mSessionKey;
};

#endif // ANH_DATABASEMANAGER_ACCOUNTDATA_H



