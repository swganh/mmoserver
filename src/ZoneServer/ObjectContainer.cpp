/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ObjectContainer.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"
#include "WorldManager.h"

//=============================================================================

ObjectContainer::ObjectContainer() 
{
	mCapacity = 0;	
}

//=============================================================================


ObjectContainer::ObjectContainer(uint64 id,uint64 parentId,string model,ObjectType type) 
				:Object(id,parentId,model,ObjType_Tangible)
{
	mCapacity = 0;	

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
		mData.push_back(Data->getId()); 
		//PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
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
	ObjectIDList::iterator it = mData.begin();

	while(it != mData.end())
	{
		if((*it) == id) 
			return(gWorldManager->getObjectById((*it))); 

		++it;
	}
	gLogger->logMsgF("ObjectContainer::getDataById Data %I64u not found",MSG_HIGH, id);
	return NULL;
}

//=============================================================================

bool ObjectContainer::removeData(Object* data)
{
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == data->getId())
		{
			it = mData.erase(it);
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
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == id)
		{
			it = mData.erase(it);
			return true;
		}
		++it;
	}
	gLogger->logMsgF("ObjectContainer::removeDataById  %I64u not found",MSG_HIGH, id);
	return false;
}

//=============================================================================

ObjectIDList::iterator ObjectContainer::removeData(ObjectIDList::iterator it)
{
	it = mData.erase(it);
return it;
}
