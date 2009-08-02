/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ADMINSERVER_CHARACTERADMINHANDLER_H
#define ANH_ADMINSERVER_CHARACTERADMINHANDLER_H

#include "Common/MessageDispatchCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
//#include "Utils/typedefs.h"


//======================================================================================================================
class Message;
class Database;
class DatabaseResult;
class MessageDispatch;
class DispatchClient;

enum QueryType
{
	CAQuery_CreateCharacter	= 1,
	CAQuery_RequestName		= 2,
};

class CAAsyncContainer
{
	public:

		CAAsyncContainer(QueryType queryType,DispatchClient* client){ mQueryType = queryType; mClient = client; }

		QueryType		mQueryType;
		DispatchClient*	mClient;
		string			mObjBaseType;
};

//======================================================================================================================

class CharacterCreateInfo
{
public:
  uint32        mAccountId;
  uint64        mCharacterId;
  string        mFirstName;
  string        mLastName;
  string        mBaseModel;
  string        mProfession;
  string        mStartCity;
  float         mScale; 
  string        mBiography;
  uint16        mAppearance[0x255];
  uint16        mHairCustomization[0x71];
  string        mHairModel;
  string        mHairColor;
};



//======================================================================================================================
class CharacterAdminHandler : public MessageDispatchCallback, public DatabaseCallback
{
	public:

		CharacterAdminHandler(void);
		~CharacterAdminHandler(void);

		void			Startup(Database* database, MessageDispatch* dispatch);
		void			Shutdown(void);
		void			Process(void);

		// Inherited from MessageDispatchCallback
		virtual void	handleDispatchMessage(uint32 opcode, Message* message, DispatchClient* client);
		virtual void	handleDatabaseJobComplete(void* ref,DatabaseResult* result);

	private:

		void				_processRandomNameRequest(Message* message, DispatchClient* client);
		void                _processCreateCharacter(Message* message, DispatchClient* client);
		void                _parseAppearanceData(Message* message, CharacterCreateInfo* info);

		void				_parseHairData(Message* message, CharacterCreateInfo* info);

		void				_sendCreateCharacterSuccess(uint64 characterId,DispatchClient* client);
		void				_sendCreateCharacterFailed(uint32 errorCode,DispatchClient* client);

		Database*           mDatabase;
		MessageDispatch*    mMessageDispatch;
};


#endif //MMOSERVER_ADMINSERVER_CHARACTERADMINHANDLER_H


