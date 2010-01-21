/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "RegionObject.h"

RegionObject::RegionObject() : Object()
{
	mType = ObjType_Region;
	mActive = false;
}

RegionObject::~RegionObject()
{
}
