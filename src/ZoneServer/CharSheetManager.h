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

#ifndef ANH_ZONESERVER_CHARSHEETMANAGER_H
#define ANH_ZONESERVER_CHARSHEETMANAGER_H

#define gCharSheetManager	CharSheetManager::getSingletonPtr()

#include <map>
#include <vector>
#include <boost/pool/pool.hpp>

#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/bstring.h"
#include "Utils/typedefs.h"

//=========================================================================================#

class Badge;
class CharSheetManager;
class CSAsyncContainer;
class Database;
class DatabaseResult;
class DispatchClient;
class Message;
class MessageDispatch;

//=========================================================================================

typedef std::vector<Badge*>			BadgeList;

//=========================================================================================

enum CharSheetQuery
{
    CharSheetQuery_Factions			=	1,
    CharSheetQuery_BadgeCategories	=	2,
    CharSheetQuery_Badges			=	3
};

//=========================================================================================

class CSAsyncContainer
{
public:

    CSAsyncContainer(CharSheetQuery query) {
        mQuery = query;
    }
    ~CSAsyncContainer() {}

    CharSheetQuery	mQuery;
};

//=========================================================================================

class CharSheetManager : public DatabaseCallback
{
public:

    static CharSheetManager*	Init(Database* database,MessageDispatch* dispatch);
    static CharSheetManager*	getSingletonPtr() {
        return mSingleton;
    }

    ~CharSheetManager();

    virtual void			handleDatabaseJobComplete(void* ref, DatabaseResult* result);

    BString					getFactionById(uint32 id) {
        return mvFactions[id - 1];
    }

    BString					getBadgeCategory(uint8 id) {
        return mvBadgeCategories[id - 1];
    }
    Badge*					getBadgeById(uint32 id) {
        return mvBadges[id];
    }

private:

    CharSheetManager(Database* database,MessageDispatch* dispatch);

    void					_processFactionRequest(Message* message,DispatchClient* client);
    void					_processPlayerMoneyRequest(Message* message,DispatchClient* client);
    void					_processStomachRequest(Message* message,DispatchClient* client);
    void					_processGuildRequest(Message* message,DispatchClient* client);

    void					_registerCallbacks();
    void					_unregisterCallbacks();

    static bool					mInsFlag;
    static CharSheetManager*	mSingleton;
    Database*					mDatabase;
    MessageDispatch*			mMessageDispatch;

    BStringVector				mvFactions;
    BStringVector				mvBadgeCategories;
    BadgeList					mvBadges;

    boost::pool<boost::default_user_allocator_malloc_free>		mDBAsyncPool;
};

#endif

