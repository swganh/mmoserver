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

#ifndef ANH_ZONESERVER_ITEM_FACTORY_H
#define ANH_ZONESERVER_ITEM_FACTORY_H

#include "FactoryBase.h"
#include "ObjectFactoryCallback.h"

#define		gItemFactory	ItemFactory::getSingletonPtr()

//=============================================================================

class Item;
class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;

//=============================================================================

enum ItemFactoryQuery
{
    ItemFactoryQuery_MainData				= 1,
    ItemFactoryQuery_Attributes				= 2,
    NonPersistantItemFactoryQuery_MainData	= 3,
    ItemFactoryQuery_Items					= 4
};

//=============================================================================

class ItemFactory : public FactoryBase, public ObjectFactoryCallback
{
public:

    static	ItemFactory*	getSingletonPtr() {
        return mSingleton;
    }
    static	ItemFactory*	Init(Database* database);

    virtual ~ItemFactory();

    virtual void			handleObjectReady(Object* object,DispatchClient* client);

    void					handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    void					requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
    void					requestContainerContent(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client, uint32 depth = 0);

private:

    ItemFactory(Database* database);

    void					_postProcessAttributes(Object* object);

    void					_setupDatabindings();
    void					_destroyDatabindings();

    Item*					_createItem(DatabaseResult* result);

    static ItemFactory*		mSingleton;
    static bool				mInsFlag;

    DataBinding*			mItemIdentifierBinding;
    DataBinding*			mItemBinding;
};

//=============================================================================

class ItemIdentifier
{
public:

    ItemIdentifier() {}
    ~ItemIdentifier() {}

    uint32	mFamilyId;
    uint32	mTypeId;
};

//=============================================================================

#endif

