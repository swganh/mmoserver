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
#include "MessageLib/MessageLib.h"
#include "MathLib/Quaternion.h"



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
}

//=============================================================================

bool CellObject::removeChild(Object* object)
{
	ObjectList::iterator it = mChildObjects.begin();
	while(it != mChildObjects.end())
	{
		if((*it) == object)
		{
			mChildObjects.erase(it);
			return(true);
		}
		++it;
	}
	return(false);
}

//=============================================================================

bool CellObject::checkForChild(Object* object)
{
	ObjectList::iterator it = mChildObjects.begin();
	while(it != mChildObjects.end())
	{
		if((*it) == object)
		{
			return(true);
		}
		++it;
	}
	return(false);
}

//=============================================================================


void CellObject::addChild(Object* object, PlayerObjectSet* inRangePlayers, PlayerObject* player)
{
	addChild(object);
	PlayerObjectSet::iterator it = inRangePlayers->begin();
	while(it != inRangePlayers->end())
	{
		PlayerObject* targetObject = (*it);
		gMessageLib->sendCreateObject(object,targetObject);
		targetObject->addKnownObjectSafe(object);
		object->addKnownObjectSafe(targetObject);
		++it;
	}
	if(player)
	{
		gMessageLib->sendCreateObject(object,player);
		player->addKnownObjectSafe(object);
		object->addKnownObjectSafe(player);
	}
	return;
}