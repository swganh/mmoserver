/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "CellObject.h"


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

