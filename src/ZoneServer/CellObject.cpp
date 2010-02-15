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

//=============================================================================



//=============================================================================

