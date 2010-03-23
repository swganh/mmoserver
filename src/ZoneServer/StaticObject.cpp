/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "StaticObject.h"
#include "MathLib/Quaternion.h"


//=============================================================================

//Cells are going to contain quite some stuff in our houses
StaticObject::StaticObject() : ObjectContainer()
{
}

//=============================================================================

StaticObject::StaticObject(uint64 id,uint64 parentId,string model,ObjectType type)
			 :ObjectContainer(id,parentId,model,type)
{
}

//=============================================================================

StaticObject::~StaticObject()
{
}

//=============================================================================