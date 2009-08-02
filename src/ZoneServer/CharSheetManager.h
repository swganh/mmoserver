/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CHARSHEETMANAGER_H
#define ANH_ZONESERVER_CHARSHEETMANAGER_H

#define gCharSheetManager	CharSheetManager::getSingletonPtr()

#include "Utils/typedefs.h"
#include <map>
#include "Badge.h"
#include "Common/MessageDispatchCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include <boost/pool/pool.hpp>


//=========================================================================================#

class Database;
class DatabaseResult;
class Message;
class MessageDispatch;
class DispatchClient;
class CSAsyncContainer;

//=========================================================================================

typedef std::vector<Badge*>			BadgeList;
typedef void						(CharSheetManager::*CSFunction)(Message*,DispatchClient*);
typedef std::map<uint32,CSFunction>	CSCommandMap;

//=========================================================================================

enum CharSheetQuery
{
	CharSheetQuery_Factions			=	1,
	CharSheetQuery_BadgeCategories	=	2,
	CharSheetQuery_Badges			=	3,
};

//=========================================================================================

class CSAsyncContainer
{
	public:

		CSAsyncContainer(CharSheetQuery query){ mQuery = query; }
		~CSAsyncContainer(){}

		CharSheetQuery	mQuery;
};

//=========================================================================================

class CharSheetManager : public DatabaseCallback, public MessageDispatchCallback
{
	public:

		static CharSheetManager*	Init(Database* database,MessageDispatch* dispatch);
		static CharSheetManager*	getSingletonPtr() { return mSingleton; }

		~CharSheetManager();

		virtual void			handleDatabaseJobComplete(void* ref, DatabaseResult* result);
		virtual void			handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client);

		string					getFactionById(uint32 id){ return mvFactions[id - 1]; }

		string					getBadgeCategory(uint8 id){ return mvBadgeCategories[id - 1]; }
		Badge*					getBadgeById(uint32 id){ return mvBadges[id]; }

	private:

		CharSheetManager(Database* database,MessageDispatch* dispatch);

		void					_processFactionRequest(Message* message,DispatchClient* client);
		void					_processPlayerMoneyRequest(Message* message,DispatchClient* client);
		void					_processStomachRequest(Message* message,DispatchClient* client);
		void					_processGuildRequest(Message* message,DispatchClient* client);

		void					_loadCommandMap();
		void					_registerCallbacks();
		void					_unregisterCallbacks();

		static bool					mInsFlag;
		static CharSheetManager*	mSingleton;
		CSCommandMap				mCommandMap;
		Database*					mDatabase;
		MessageDispatch*			mMessageDispatch;

		BStringVector				mvFactions;
		BStringVector				mvBadgeCategories;
		BadgeList					mvBadges;

		boost::pool<boost::default_user_allocator_malloc_free>		mDBAsyncPool;
};

#endif

