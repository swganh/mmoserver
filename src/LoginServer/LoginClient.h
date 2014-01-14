/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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
  LCSTATE_End
};


//======================================================================================================================
class LoginClient : public NetworkClient
{
public:
                              LoginClient(void) : mState(LCSTATE_ClientCreated) {};
  virtual                     ~LoginClient(void) {};

  LoginClientState            getState(void)                            { return mState; };
  uint32                      getAccountId(void)                        { return mAccountId; };
  string&                     getUsername(void)                         { return mUsername; };
  string&                     getPassword(void)                         { return mPassword; };
  uint32                      getCharsAllowed()                         { return mCharsAllowed; }
  int8						  getCsr()									{ return mCsr; }

  void                        setState(LoginClientState state)          { mState = state; };
  void                        setAccountId(uint32 id)                   { mAccountId = id; };
  void                        setUsername(string username)              { mUsername = username; };
  void                        setPassword(string password)              { mPassword = password; };
  void                        setCharsAllowed(uint32 count)             { mCharsAllowed = count; }
  void                        setCsr(int8 csr)							{ mCsr = csr; };
private:
  LoginClientState            mState;
  uint32                      mAccountId;
  string                      mUsername;
  string                      mPassword;
  uint32                      mCharsAllowed;
  int8						  mCsr;
};



#endif // ANH_LOGINSERVER_LOGINCLIENT_H



