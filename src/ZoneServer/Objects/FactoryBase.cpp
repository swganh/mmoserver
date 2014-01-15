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
#include "ZoneServer/Objects/FactoryBase.h"

#include <cppconn/resultset.h>

#include "Utils/utils.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "ZoneServer/Objects/Object.h"
#include "ZoneServer/WorldManager.h"

//=============================================================================

FactoryBase::FactoryBase(swganh::database::Database* database)
    : mILCPool(sizeof(InLoadingContainer))
    , mQueryContainerPool(sizeof(QueryContainerBase))
    , mDatabase(database)
{
    mAttributeBinding = mDatabase->createDataBinding(3);
    mAttributeBinding->addField(swganh::database::DFT_stdstring,offsetof(Attribute_QueryContainer,mKey),64,0);
    mAttributeBinding->addField(swganh::database::DFT_stdstring,offsetof(Attribute_QueryContainer,mValue),128,1);
    mAttributeBinding->addField(swganh::database::DFT_uint8,offsetof(Attribute_QueryContainer,mInternal),1,2);
}

//=============================================================================

FactoryBase::~FactoryBase()
{
    mDatabase->destroyDataBinding(mAttributeBinding);
}



//=============================================================================

InLoadingContainer* FactoryBase::_getObject(uint64 id)
{
    ObjectLoadMap::iterator it = mObjectLoadMap.find(id);

    if(it != mObjectLoadMap.end())
        return((*it).second);

    return(NULL);
}

//=============================================================================

bool	FactoryBase::PlayerLoading(uint64 id)
{
	ObjectLoadMap::iterator it = mObjectLoadMap.find(id);

    if(it != mObjectLoadMap.end())    {
        return(true);
    }
    return(false);
}

bool FactoryBase::_removeFromObjectLoadMap(uint64 id)
{
    ObjectLoadMap::iterator it = mObjectLoadMap.find(id);

    if(it != mObjectLoadMap.end())
    {
        mObjectLoadMap.erase(it);
        return(true);
    }
    return(false);
}

//=============================================================================
// this should always be last in load order
void FactoryBase::_buildAttributeMap(Object* object,swganh::database::DatabaseResult* result)
{
    std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

    Attribute_QueryContainer	attribute;
    uint64						count = result->getRowCount();
    int8						str[256];
    BStringVector				dataElements;

    for(uint64 i = 0; i < count; i++)
    {
        if (result_set->next())
        {
            attribute.mKey		= result_set->getString(1);
            attribute.mValue	= result_set->getString(2);
            attribute.mInternal = result_set->getUInt(3);

            //result->getNextRow(mAttributeBinding,(void*)&attribute);
			if(common::memcrc(attribute.mKey) == common::memcrc("cat_manf_schem_ing_resource"))
            {
				BString v(attribute.mValue.c_str());
                v.split(dataElements,' ');
                sprintf(str,"cat_manf_schem_ing_resource.\"%s",dataElements[0].getAnsi());

                attribute.mKey		= str;
                attribute.mValue	= dataElements[1].getAnsi();

                //add key to the worldmanager
				if(gWorldManager->getAttributeKey(common::memcrc(attribute.mKey)) == "")
                {
                    gWorldManager->mObjectAttributeKeyMap.insert(std::make_pair(common::memcrc(attribute.mKey),attribute.mKey));
                }

            }

            if(attribute.mInternal)
				object->addInternalAttribute(BString(attribute.mKey.c_str()),std::string(attribute.mValue));
            else
                object->addAttribute(attribute.mKey,std::string(attribute.mValue));
        }
    }

    object->setLoadState(LoadState_Loaded);
}

//=============================================================================
