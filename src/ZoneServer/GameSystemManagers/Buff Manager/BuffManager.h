/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#ifndef ANH_BUFFMANAGER_H
#define ANH_BUFFMANAGER_H

#define	 gBuffManager	BuffManager::getSingletonPtr()

//======================================================================================================================
#include "Zoneserver/GameSystemManagers/Buff Manager/Buff.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/EventHandler.h"
#include "Utils/typedefs.h"

//======================================================================================================================
namespace swganh	{
namespace database	{
class Database;
class DatabaseCallback;
class DatabaseResult;
}}

class CreatureObject;
class PlayerObject;
class QueryContainerBase;
class WMAsyncContainer;
struct buffAsyncContainer;

enum BMQuery
{
    BMQuery_Null			= 0,
    BMQuery_Buffs			= 1,
    BMQuery_Buff_Attributes	= 2,
    BMQuery_Delete			= 3,
    BMQuery_Save_Async		= 4
};
//======================================================================================================================
//======================================================================================================================
class BuffManager : public swganh::database::DatabaseCallback, public Anh_Utils::EventHandler
{
    friend class CreatureObject;
public:

    ~BuffManager(void);
    BuffManager(swganh::database::Database* database);

    static		BuffManager*	Init(swganh::database::Database* database);
    static		BuffManager*	getSingletonPtr() {
        return mSingleton;
    }

    void		handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result);
    void		SaveBuffs(PlayerObject* playerObject, uint64 currenttime);
    bool		SaveBuffsAsync(WMAsyncContainer* asyncContainer,swganh::database::DatabaseCallback* callback, PlayerObject* playerObject, uint64 currenttime);
    void		LoadBuffs(PlayerObject* playerObject, uint64 currenttime);
    void		LoadBuffAttributes(buffAsyncContainer* envelope);
    void		InitBuffs(PlayerObject* Player);

private:

    void		LoadBuffsFromResult(buffAsyncContainer* asyncContainer, swganh::database::DatabaseResult* result);
    void		LoadBuffAttributesFromResult(buffAsyncContainer* asyncContainer, swganh::database::DatabaseResult* result);


    bool		AddBuffToDB(WMAsyncContainer* asyncContainer,swganh::database::DatabaseCallback* callback, Buff* buff, uint64 currenttime);
    void		AddBuffToDB(Buff* buff, uint64 currenttime);


    static		BuffManager*	mSingleton;
    swganh::database::Database*	mDatabase;
    static bool mInsFlag;
};

#endif //ANH_BUFFMANAGER_H
