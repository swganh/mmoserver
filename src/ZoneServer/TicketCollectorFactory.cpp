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

#include "TicketCollectorFactory.h"
#include "ObjectFactoryCallback.h"
#include "TicketCollector.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"

//=============================================================================

bool					TicketCollectorFactory::mInsFlag    = false;
TicketCollectorFactory*	TicketCollectorFactory::mSingleton  = NULL;

//======================================================================================================================

TicketCollectorFactory*	TicketCollectorFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new TicketCollectorFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

TicketCollectorFactory::TicketCollectorFactory(Database* database) : FactoryBase(database)
{
    _setupDatabindings();
}

//=============================================================================

TicketCollectorFactory::~TicketCollectorFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void TicketCollectorFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case TCFQuery_MainData:
    {
        TicketCollector* collector = _createTicketCollector(result);

        if(collector->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
            asyncContainer->mOfCallback->handleObjectReady(collector,asyncContainer->mClient);
        else
        {

        }
    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void TicketCollectorFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,TCFQuery_MainData,client),
                                        "SELECT * FROM %s.ticket_collectors WHERE id = %" PRIu64 "",mDatabase->galaxy(),id);
    
}

//=============================================================================

TicketCollector* TicketCollectorFactory::_createTicketCollector(DatabaseResult* result)
{
	if (!result->getRowCount()) {
		return nullptr;
	}

    TicketCollector*	ticketCollector = new TicketCollector();

    result->getNextRow(mTicketCollectorBinding,(void*)ticketCollector);

    ticketCollector->mTypeOptions = 0x108;
    ticketCollector->setLoadState(LoadState_Loaded);

    return ticketCollector;
}

//=============================================================================

void TicketCollectorFactory::_setupDatabindings()
{
    mTicketCollectorBinding = mDatabase->createDataBinding(13);
    mTicketCollectorBinding->addField(DFT_uint64,offsetof(TicketCollector,mId),8,0);
    mTicketCollectorBinding->addField(DFT_uint64,offsetof(TicketCollector,mParentId),8,1);
    mTicketCollectorBinding->addField(DFT_bstring,offsetof(TicketCollector,mModel),256,2);
    mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mDirection.x),4,3);
    mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mDirection.y),4,4);
    mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mDirection.z),4,5);
    mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mDirection.w),4,6);
    mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mPosition.x),4,7);
    mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mPosition.y),4,8);
    mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mPosition.z),4,9);
    mTicketCollectorBinding->addField(DFT_bstring,offsetof(TicketCollector,mName),64,11);
    mTicketCollectorBinding->addField(DFT_bstring,offsetof(TicketCollector,mNameFile),64,12);
    mTicketCollectorBinding->addField(DFT_bstring,offsetof(TicketCollector,mPortDescriptor),64,13);
}

//=============================================================================

void TicketCollectorFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mTicketCollectorBinding);
}

//=============================================================================

