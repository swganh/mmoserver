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
			glm::vec3 playerWorldPosition = player->getWorldPosition();
			playerWorldPosition.x += 2;
			playerWorldPosition.z += 2;
			player->updatePosition(0,playerWorldPosition);
			player->setParentIdIncDB(0);
			//already removed out of the cell
			objIt = cellObjects->begin();
		}
		else
		if(CreatureObject* pet = dynamic_cast<CreatureObject*>(object))
		{
			pet->setParentIdIncDB(0);
			pet->updatePosition(0,pet->getWorldPosition());
			//already removed out of the cell
			objIt = cellObjects->begin();
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

