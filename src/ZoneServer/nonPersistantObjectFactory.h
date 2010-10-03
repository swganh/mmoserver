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

#ifndef ANH_ZONESERVER_NON_PERSISTANT_H
#define ANH_ZONESERVER_NON_PERSISTANT_H

#include "FactoryBase.h"

#include <boost/pool/pool.hpp>
#include <glm/glm.hpp>

#define 	gNonPersistantObjectFactory	NonPersistantObjectFactory::getSingletonPtr()

class CampTerminal;
class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class PlayerStructure;
class PlayerObject;
class TangibleObject;

struct StructureDeedLink;
struct StructureItemTemplate;

//=============================================================================


class NonPersistantObjectFactory : public FactoryBase
{
public:

    static NonPersistantObjectFactory*	getSingletonPtr() {
        return mSingleton;
    }
    static NonPersistantObjectFactory*	Instance();

    static inline void deleteFactory(void)
    {
        if (mSingleton)
        {
            delete mSingleton;
            mSingleton = 0;
        }
    }

    virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    virtual void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
    void					createTangible(ObjectFactoryCallback* ofCallback, uint32 familyId, uint32 typeId, uint64 parentId, const glm::vec3& position, BString customName, DispatchClient* client);

    PlayerStructure*		requestBuildingFenceObject(float x, float y, float z, PlayerObject* player);
    PlayerStructure*		requestBuildingSignObject(float x, float y, float z, PlayerObject* player, BString name, BString namefile, BString custom);

    //spawns temporary objects for camps
    TangibleObject*			spawnTangible(StructureItemTemplate* placableTemplate, uint64 parentId, const glm::vec3& position, const BString& customName, PlayerObject* player);
    CampTerminal*			spawnTerminal(StructureItemTemplate* placableTemplate, uint64 parentId, const glm::vec3& position, const BString& customName, PlayerObject* player, StructureDeedLink*	deedData);

    void					_createItem(DatabaseResult* result,Item* item);

    TangibleObject*			cloneTangible(TangibleObject* theTemplate);

protected:
    NonPersistantObjectFactory(Database* database);
    ~NonPersistantObjectFactory();

private:

    // This constructor prevents the default constructor to be used, since it is private.
    NonPersistantObjectFactory();

    static NonPersistantObjectFactory*	mSingleton;

    void					_setupDatabindings();
    void					_destroyDatabindings();

    Database*				mDatabase;
    DataBinding*			mItemBinding;
    DataBinding*			mItemIdentifierBinding;
    uint64					mId;
};

//=============================================================================


#endif
