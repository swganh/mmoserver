/* This source file is part of swgANH (Star Wars Galaxies - A
   New Hope - Server Emulator)
   
   For more information, see http://www.swganh.org
   
   Copyright (c) 2006 - 2010 The swgANH Team                  */

#ifndef ANH_DATABASEMANAGER_ACCOUNTDATA_H
#define ANH_DATABASEMANAGER_ACCOUNTDATA_H

#include "Utils/typedefs.h"


// These are for DB queries.
//======================================================================================================================
class AccountData
{
public:
  uint32                mId;
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
  string                mName;
  string                mAddress;
  uint16                mConnectionPort;
  uint16                mPingPort;
  uint32                mPopulation;
  uint32                mStatus;
  uint32                mLastUpdate;
};


#endif // ANH_DATABASEMANAGER_ACCOUNTDATA_H



