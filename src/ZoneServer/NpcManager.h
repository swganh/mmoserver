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

#ifndef ANH_ZONESERVER_NPC_MANAGER_H
#define ANH_ZONESERVER_NPC_MANAGER_H


#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/typedefs.h"
#include "ObjectFactoryCallback.h"

//=============================================================================

class AttackableCreature;
class CreatureObject;
class Database;
class NPCObject;
class SpawnData;
class Weapon;

//======================================================================================================================
//
// Container for asyncronous database queries
//
enum NpcQuery
{
    NpcQuery_Lairs = 0
};

class NpcAsyncContainer
{
public:
    NpcAsyncContainer(NpcQuery query) {
        mQuery = query;
    }
    NpcQuery mQuery;
};


//=============================================================================


class NpcManager  : public ObjectFactoryCallback, public DatabaseCallback
{
public:

    static NpcManager* Instance(void);

    static inline void deleteManager(void)
    {
        if (mInstance)
        {
            delete mInstance;
            mInstance = 0;
        }
    }

    // Inherited  interface that we have to provide.
    virtual void	handleObjectReady(Object* object);
    virtual void    handleDatabaseJobComplete(void* ref, DatabaseResult* result);

    // void	addCreature(uint64 creatureId, const SpawnData *spawn);
    void	handleExpiredCreature(uint64 creatureId);
    // void	removeNpc(uint64 npcId);
    bool	handleAttack(CreatureObject *attacker, uint64 targetId);

    uint64	handleNpc(NPCObject* npc, uint64 timeOverdue);

    void	loadLairs(void);


protected:
    NpcManager(Database* database);
    ~NpcManager();

private:
    // This constructor prevents the default constructor to be used, since it is private.
    NpcManager();

    // Simulated Combat Manager
    uint8	_hitCheck(CreatureObject* attacker,CreatureObject* defender,Weapon* weapon);
    bool	_verifyCombatState(CreatureObject* attacker, uint64 defenderId);

    uint8	_executeAttack(CreatureObject* attacker,CreatureObject* defender,Weapon* weapon);
    void	setTargetDirection(AttackableCreature* npc);
    // void			_setupDatabindings();
    // void			_destroyDatabindings();

    static NpcManager* mInstance;
    Database* mDatabase;
    // DataBinding*	mItemIdentifierBinding;
    // DataBinding*	mItemBinding;
};

//=============================================================================



#endif

