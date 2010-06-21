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

CellObject::CellObject(uint64 id,uint64 parentId,BString model) : StaticObject(id,parentId,model,ObjType_Cell)
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

