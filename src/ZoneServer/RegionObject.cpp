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

#include "RegionObject.h"

RegionObject::RegionObject() : Object()
{
    mType = ObjType_Region;
    mActive = false;
}

RegionObject::~RegionObject()
{
}
bool RegionObject::addVisitor(Object* visitor)
{
	if(checkVisitor(visitor))
	{
		return false;
	}

	mVisitingPlayers.insert(visitor->getId());
	return true;
}

//=============================================================================
// returns true when item *is* found

bool RegionObject::checkVisitor(Object* object)
{
	
	ObjectIDSet::const_iterator it = mVisitingPlayers.find(object->getId());

	if(it != mVisitingPlayers.end())
	{
		return(true);
	}
	
	return(false);
}

void RegionObject::removeVisitor(Object* object)
{
	
	ObjectIDSet::iterator it = mVisitingPlayers.find(object->getId());

	if(it != mVisitingPlayers.end())
	{
	
		mVisitingPlayers.erase(it);

	}
	
}
std::shared_ptr<RegionObject> RegionObject::getSharedFromThis() {
    return std::static_pointer_cast<RegionObject>(Object::shared_from_this());
}
