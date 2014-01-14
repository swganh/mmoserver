/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "CellObject.h"
#include "PlayerObject.h"
#include "TangibleObject.h"
#include "PlayerStructureTerminal.h"
#include "MessageLib/MessageLib.h"




//=============================================================================

CellObject::CellObject() : StaticObject()
{
	mType = ObjType_Cell;
	mModel = "object/cell/shared_cell.iff";
}

//=============================================================================

CellObject::CellObject(uint64 id,uint64 parentId,string model) : StaticObject(id,parentId,model,ObjType_Cell)
{
}

//=============================================================================

CellObject::~CellObject()
{
	//see objectcontainer for details
}

//=============================================================================
// the building gets destroyed - thus we nead to clear up our content in world and in db
//   
// separate this from the destructor as we do not want players in building on server shutdown 
// to be placed in the gameworld

void CellObject::prepareDestruction()
{
	 //iterate through contained Objects
	ObjectIDList* cellObjects		= this->getObjects();
	ObjectIDList::iterator objIt	= cellObjects->begin();

	while(objIt != cellObjects->end())
	{
		Object* object = gWorldManager->getObjectById((*objIt));

		if(PlayerObject* player = dynamic_cast<PlayerObject*>(object))
		{
			//place the player in the world
			player->setParentId(0,0xffffffff,player->getKnownPlayers(),true);
			objIt = cellObjects->erase(objIt);
		}
		else
		if(CreatureObject* pet = dynamic_cast<CreatureObject*>(object))
		{
			pet->setParentId(0,0xffffffff,pet->getKnownPlayers(),true);
			objIt = cellObjects->erase(objIt);
		}
		else
		{
			gWorldManager->destroyObjectForKnownPlayers(object);
			
			//Carefull! destroyObject removes the object from the cell!!!
			//the iterator is invalid afterwards!!!
			gWorldManager->destroyObject(object);
			objIt = cellObjects->begin();
		}
		//careful with iterating here!!!
	}
}
//=============================================================================

//=============================================================================



//=============================================================================

