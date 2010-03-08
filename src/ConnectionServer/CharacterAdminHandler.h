/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ADMINSERVER_CHARACTERADMINHANDLER_H
#define ANH_ADMINSERVER_CHARACTERADMINHANDLER_H

#include "ConnectionDispatchCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
//#include "Utils/typedefs.h"


//======================================================================================================================
class Message;
class Database;
class DatabaseResult;
class ConnectionDispatch;
class ConnectionClient;

enum QueryType
{
	CAQuery_CreateCharacter	= 1,
	CAQuery_RequestName		= 2
};

class CAAsyncContainer
{
	public:

		CAAsyncContainer(QueryType queryType,ConnectionClient* client){ mQueryType = queryType; mClient = client; }

		QueryType		mQueryType;
		ConnectionClient*	mClient;
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
class CharacterAdminHandler : public ConnectionDispatchCallback, public DatabaseCallback
{
	public:

		explicit CharacterAdminHandler(uint8 galaxyId);
		~CharacterAdminHandler(void);

		void			Startup(Database* database, ConnectionDispatch* dispatch);
		void			Shutdown(void);
		void			Process(void);

		// Inherited from ConnectionDispatchCallback
		virtual void	handleDispatchMessage(uint32 opcode, Message* message, ConnectionClient* client);
		virtual void	handleDatabaseJobComplete(void* ref,DatabaseResult* result);

	private:

		void				_processRandomNameRequest(Message* message, ConnectionClient* client);
		void                _processCreateCharacter(Message* message, ConnectionClient* client);
		void                _parseAppearanceData(Message* message, CharacterCreateInfo* info);

		void				_parseHairData(Message* message, CharacterCreateInfo* info);

		void				_sendCreateCharacterSuccess(uint64 characterId,ConnectionClient* client);
		void				_sendCreateCharacterFailed(uint32 errorCode,ConnectionClient* client);

		Database*           mDatabase;
		ConnectionDispatch*    mConnectionDispatch;
		uint8					mGalaxyId;
};


#endif //MMOSERVER_ADMINSERVER_CHARACTERADMINHANDLER_H


