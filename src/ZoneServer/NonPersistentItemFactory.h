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

#ifndef ANH_ZONESERVER_NON_PERSISTENT_ITEM_FACTORY_H
#define ANH_ZONESERVER_NON_PERSISTENT_ITEM_FACTORY_H

#include "FactoryBase.h"

#define	 gNonPersistentItemFactory	NonPersistentItemFactory::Instance()

//=============================================================================


//=============================================================================

class NonPersistentItemFactory : public FactoryBase
{
public:
    static NonPersistentItemFactory* Instance(void);
    /*
    static inline NonPersistentItemFactory*	Instance(void)
    {
    	if (!mSingleton)
    	{
    		mSingleton = new NonPersistentItemFactory(WorldManager::getSingletonPtr()->getDatabase());
    	}
    	return mSingleton;
    }
    */

    static inline void deleteFactory(void)
    {
        if (mSingleton)
        {
            delete mSingleton;
            mSingleton = 0;
        }
    }

    void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
    void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id, uint64 newId);

protected:
    NonPersistentItemFactory(Database* database);
    ~NonPersistentItemFactory();

private:
    // This constructor prevents the default constructor to be used, since it is private.
    NonPersistentItemFactory();

    void			_setupDatabindings();
    void			_destroyDatabindings();

    Item* _createItem(DatabaseResult* result, uint64 newId);

    static NonPersistentItemFactory*	mSingleton;
    DataBinding*	mItemIdentifierBinding;
    DataBinding*	mItemBinding;
};

//=============================================================================



#endif

