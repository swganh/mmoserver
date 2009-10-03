/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ObjectContainer.h"
#include "MessageLib/MessageLib.h"
#include "PlayerObject.h"

//=============================================================================

ObjectContainer::ObjectContainer() 
{
	mCapacity = 65535;	
}

//=============================================================================


ObjectContainer::ObjectContainer(uint64 id,uint64 parentId,string model,ObjectType type) 
				:Object(id,parentId,model,ObjType_Tangible)
{
	mCapacity = 65535;	

}

//=============================================================================

ObjectContainer::~ObjectContainer()
{
	
}

//=============================================================================
																			   
bool ObjectContainer::addData(Object* Data) 
{ 
	if(mCapacity)
	{
		mData.push_back(Data); 
		//PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
		mCapacity--;
		return true;
	}
	else
	{
		//PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
		//gMessageLib->sendSystemMessage(player,L"","base_player","too_many_waypoints");
		return false;

	}
}

//=============================================================================

Object* ObjectContainer::getDataById(uint64 id)
{
	ObjectList::iterator it = mData.begin();

	while(it != mData.end())
	{
		if((*it)->getId() == id) return(*it); ++it;
	}
	gLogger->logMsgF("ObjectContainer::getDataById Data %I64u not found",MSG_HIGH, id);
	return NULL;
}

//=============================================================================

bool ObjectContainer::removeData(Object* data)
{
	ObjectList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == data)
		{
			removeData(it);
			return true;
		}
		++it;
	}
	gLogger->logMsgF("ObjectContainer::removeDataByPointer Data %I64u not found",MSG_HIGH, data->getId());
	return false;
}

//=============================================================================

bool ObjectContainer::removeData(uint64 id)
{
	ObjectList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it)->getId() == id)
		{
			removeData(it);
			return true;
		}
		++it;
	}
	gLogger->logMsgF("ObjectContainer::removeDataById  %I64u not found",MSG_HIGH, id);
	return false;
}

//=============================================================================

ObjectList::iterator ObjectContainer::removeData(ObjectList::iterator it)
{
	it = mData.erase(it);
	mCapacity++;
return it;
}
