
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "ObjectFactory.h"
#include "HouseObject.h"
#include "PlayerObject.h"
#include "Inventory.h"
#include "CellObject.h"

#include "ResourceContainer.h"
#include "StructureManager.h"
#include "UIManager.h"
#include "MathLib/Quaternion.h"
#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include <cassert>

//=============================================================================

HouseObject::HouseObject() : BuildingObject()
{
	mType = ObjType_Building;
	
	setWidth(64);
	setHeight(64);
	
}

//=============================================================================

HouseObject::~HouseObject()
{

}



void HouseObject::checkCellPermission(PlayerObject* player)
{
	

	if(this->getPublic())
	{
		//structure is public - are we banned ?
		StructureAsyncCommand command;
		command.Command = Structure_Command_CellEnterDenial;
		command.PlayerId = player->getId();
		command.StructureId = this->getId();
		
		gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"BAN",command);

	}
	else
	{
		//structure is private - do we have access ?
		StructureAsyncCommand command;
		command.Command = Structure_Command_CellEnter;
		command.PlayerId = player->getId();
		command.StructureId = this->getId();
		
		gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ENTRY",command);

	}	
}


//========================================================================0
//
//

void HouseObject::handleObjectReady(Object* object,DispatchClient* client, uint64 hopper)
{
	Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(hopper));
	if(!item)
	{
		gLogger->logMsgF("FactoryObject::handleObjectReady::could not find Hopper",MSG_HIGH);
		assert(false && "HouseObject::handleObjectReady could not find hopper");
	}
}



//=============================================================================
//handles the radial selection

void HouseObject::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject);
	if(!player)
	{	
		gLogger->logMsgF("FactoryObject::handleObjectMenuSelect::could not find player",MSG_HIGH);
		return;
	}
	
}

//=============================================================================
// ´not needed - this is handled over the structures terminal
void HouseObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	
}



void HouseObject::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;

//	switch(asynContainer->mQueryType)
//	{

		
//		default:break;

//	}

	SAFE_DELETE(asynContainer);
}


bool HouseObject::hasAdmin(uint64 id)
{
	ObjectIDList		adminList =	getHousingList();
	
	ObjectIDList::iterator it =	 adminList.begin();

	while (it != adminList.end())
	{
		if( id == (*it))
			return true;

		it++;
	}
	return false;
}

void HouseObject::prepareDestruction()
{
	//iterate through all the cells - do they need to be deleted ?
	//place players inside a cell in the world
	CellObjectList*				cellList	= getCellList();
	CellObjectList::iterator	cellIt		= cellList->begin();

	while(cellIt != cellList->end())
	{
		CellObject* cell = (*cellIt);
					
		//remove items in the building from the world 
		//place players and their pets in the maincell
		cell->prepareDestruction();

		++cellIt;
	}

}



