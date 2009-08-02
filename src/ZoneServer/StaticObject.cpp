/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "StaticObject.h"

//=============================================================================

StaticObject::StaticObject() : Object()
{
}

//=============================================================================

StaticObject::StaticObject(uint64 id,uint64 parentId,string model,ObjectType type)
			 :Object(id,parentId,model,type)
{
}

//=============================================================================

StaticObject::~StaticObject()
{
}

//=============================================================================