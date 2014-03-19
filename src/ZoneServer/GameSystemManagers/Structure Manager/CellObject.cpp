/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/Objects/Tangible Object/TangibleObject.h"
#include "ZoneServer\Objects\permissions\container_permissions_interface.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/Objects/PlayerStructureTerminal.h"
#include "MessageLib/MessageLib.h"




//=============================================================================

CellObject::CellObject() : StaticObject()
{
    mType = ObjType_Cell;
	mTotalLoad = 0;
    //mModel = "object/cell/shared_cell.iff";
	SetTemplate("object/cell/shared_cell.iff");
	
}

//=============================================================================

CellObject::CellObject(uint64 id,uint64 parentId,std::string model) : StaticObject(id,parentId,model,ObjType_Cell)
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

void CellObject::prepareDestruction() {
    
	//iterate through contained Objects
	//everything still inside gets removed from memory
	this->ViewObjects(this, 0,false, [&] (Object* object)	{
		if(!object)	{
			LOG(error) << "CellObject::prepareDestruction() cell : " << this->getId();
			LOG(error) << "object not found";
			return;
		}
	
        //we should have gotten rid of them by now!
        if(object->getType() == ObjType_Player) {
            LOG(error) << "CellObject::prepareDestruction() cell : " << this->getId() << "theres still a player here :(";
			//assert(false && "Player objects should have already been removed by this point");
            return;
        }

        if(object->getType() == ObjType_Creature) {
            CreatureObject* pet = static_cast<CreatureObject*>(object);

            //put the creature into the world
            pet->setParentIdIncDB(0);
            pet->updatePosition(0,pet->getWorldPosition());

            return;
        }
		//ok everything left inside will be destroyed
		//gWorldManager->destroyObject(object); will remove it out of the si, too (and thus out of the cell)
		
        gWorldManager->destroyObject(object);
    });
}
//=============================================================================

void CellObject::RemoveCreature(Object* requester, Object* oldObject)
{
    //// CHECK PERMISSIONS //// --- no failure possible
	if (oldObject->getObjectType() != SWG_PLAYER  && oldObject->getObjectType() != SWG_CREATURE)	{
		LOG(error) << "CellObject::RemovePlayer " << oldObject->getId() << " is no player or creature";
		return;
	}
	//just remove without checking for updates
	//the spatial index manager keeps track of that 
	boost::upgrade_lock<boost::shared_mutex> lock(global_container_lock_);

    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);

        ////Remove Object from Datastructure
        for(auto& slot = slot_descriptor_.begin();slot != slot_descriptor_.end(); slot++)
        {
            slot->second->remove_object(oldObject);
        }
        oldObject->SetContainer(nullptr);

		//if(hasObject(data->getId()))
		//{
		//	assert(false);
		//	return false;
		//}

		//mData.push_back(data->getId());

    }
	return;
}
//=============================================================================

bool CellObject::AddCreature(Object* newObject)
{
	uint32 arrangement = GetAppropriateArrangementId(newObject);
    if(AddCreature(newObject, arrangement))	{
		newObject->SetArrangementId(arrangement);
		return true;
	}
	return false;
}

bool CellObject::AddCreature(Object* creature, int32_t arrangement_id)
{
	if (creature->getObjectType() != SWG_PLAYER  && creature->getObjectType() != SWG_CREATURE)	{
		LOG(error) << "CellObject::AddPlayer " << creature->getId() << " is no player or creature";
		return false;
	}

    //// CHECK PERMISSIONS ////
	if(!container_permissions_->canInsert(this, creature, creature))	{
		LOG (info) << "CellObject::AddPlayercouldnt add Creature : " << creature->getId() << " to " << getId();
		return false;
	}

    boost::upgrade_lock<boost::shared_mutex> lock(global_container_lock_);

    //Add Object To Datastructure
    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
        arrangement_id = __InternalInsert(creature, creature->mPosition, arrangement_id);
    }

    
	return true;
}

//=============================================================================

