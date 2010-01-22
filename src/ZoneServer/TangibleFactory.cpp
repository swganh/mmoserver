/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "TangibleFactory.h"
#include "ContainerObjectFactory.h"
#include "ItemFactory.h"
#include "DatapadFactory.h"

#include "ResourceContainerFactory.h"
#include "TangibleEnums.h"
#include "TerminalFactory.h"
#include "TicketCollectorFactory.h"
#include "LogManager/LogManager.h"
#include "Utils/utils.h"

#include <assert.h>

//=============================================================================

bool					TangibleFactory::mInsFlag    = false;
TangibleFactory*		TangibleFactory::mSingleton  = NULL;

//======================================================================================================================

TangibleFactory*	TangibleFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new TangibleFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

TangibleFactory::TangibleFactory(Database* database) : FactoryBase(database)
{
	mContainerFactory			= ContainerObjectFactory::Init(mDatabase);
	mTerminalFactory			= TerminalFactory::Init(mDatabase);
	mTicketCollectorFactory		= TicketCollectorFactory::Init(mDatabase);
	mItemFactory				= ItemFactory::Init(mDatabase);
	mResourceContainerFactory	= ResourceContainerFactory::Init(mDatabase);
}

//=============================================================================

TangibleFactory::~TangibleFactory()
{
	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void TangibleFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	switch(subGroup)
	{
		case TanGroup_Item:						mItemFactory->requestObject(ofCallback,id,subGroup,subType,client);				break;
		case TanGroup_Terminal:					mTerminalFactory->requestObject(ofCallback,id,subGroup,subType,client);			break;
		case TanGroup_Container:				mContainerFactory->requestObject(ofCallback,id,subGroup,subType,client);			break;
		case TanGroup_TicketCollector:			mTicketCollectorFactory->requestObject(ofCallback,id,subGroup,subType,client);		break;
		case TanGroup_ResourceContainer:		mResourceContainerFactory->requestObject(ofCallback,id,subGroup,subType,client);	break;
		case TanGroup_ManufacturingSchematic:	gDatapadFactory->requestManufacturingSchematic(ofCallback,id);
		case TanGroup_Hopper:					mItemFactory->requestObject(ofCallback,id,subGroup,subType,client);				break;

		default:
			gLogger->logMsg("TangibleFactory::requestObject Unknown Group");
		break;
	}
}

//=============================================================================

void TangibleFactory::releaseAllPoolsMemory()
{
	mItemFactory->releaseQueryContainerPoolMemory();
	mTerminalFactory->releaseQueryContainerPoolMemory();
	mContainerFactory->releaseQueryContainerPoolMemory();
	mTicketCollectorFactory->releaseQueryContainerPoolMemory();
	mResourceContainerFactory->releaseQueryContainerPoolMemory();
}

//=============================================================================


