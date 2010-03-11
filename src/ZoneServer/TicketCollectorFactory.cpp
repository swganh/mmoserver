/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "TicketCollectorFactory.h"
#include "ObjectFactoryCallback.h"
#include "TicketCollector.h"
#include "LogManager/LogManager.h"
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

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void TicketCollectorFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,TCFQuery_MainData,client),"SELECT * FROM ticket_collectors WHERE id = %"PRIu64"",id);
}

//=============================================================================

TicketCollector* TicketCollectorFactory::_createTicketCollector(DatabaseResult* result)
{
	TicketCollector*	ticketCollector = new TicketCollector();

	uint64 count = result->getRowCount();

	result->GetNextRow(mTicketCollectorBinding,(void*)ticketCollector);

	ticketCollector->mTypeOptions = 0x108;
	ticketCollector->setLoadState(LoadState_Loaded);

	return ticketCollector;
}

//=============================================================================

void TicketCollectorFactory::_setupDatabindings()
{
	mTicketCollectorBinding = mDatabase->CreateDataBinding(13);
	mTicketCollectorBinding->addField(DFT_uint64,offsetof(TicketCollector,mId),8,0);
	mTicketCollectorBinding->addField(DFT_uint64,offsetof(TicketCollector,mParentId),8,1);
	mTicketCollectorBinding->addField(DFT_bstring,offsetof(TicketCollector,mModel),256,2);
	mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mDirection.mX),4,3);
	mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mDirection.mY),4,4);
	mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mDirection.mZ),4,5);
	mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mDirection.mW),4,6);
	mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mPosition.mX),4,7);
	mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mPosition.mY),4,8);
	mTicketCollectorBinding->addField(DFT_float,offsetof(TicketCollector,mPosition.mZ),4,9);
	mTicketCollectorBinding->addField(DFT_bstring,offsetof(TicketCollector,mName),64,11);
	mTicketCollectorBinding->addField(DFT_bstring,offsetof(TicketCollector,mNameFile),64,12);
	mTicketCollectorBinding->addField(DFT_bstring,offsetof(TicketCollector,mPortDescriptor),64,13);
}

//=============================================================================

void TicketCollectorFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mTicketCollectorBinding);
}

//=============================================================================

