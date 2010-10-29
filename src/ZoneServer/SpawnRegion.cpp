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

#include "SpawnRegion.h"
#include "PlayerObject.h"
#include "QTRegion.h"
#include "QuadTree.h"
#include "WorldManager.h"
#include "ZoneTree.h"

//=============================================================================

SpawnRegion::SpawnRegion()
    : RegionObject()
    , mQTRegion(NULL)
    , mSI(gWorldManager->getSI())
    , mMission(0)
{
    mActive		= true;
    mRegionType = Region_Spawn;
}

//=============================================================================

SpawnRegion::~SpawnRegion()
{
}

//=============================================================================

void SpawnRegion::update()
{
    //this is very problematic - currently we assume an object entered us when it is in the same qtregion we are in

    //run about every 4.5 seconds
    if(!mSubZoneId)
    {
        mQTRegion	= mSI->getQTRegion(mPosition.x,mPosition.z);
        mSubZoneId	= (uint32)mQTRegion->getId();
        mQueryRect	= Anh_Math::Rectangle(mPosition.x - mWidth,mPosition.z - mHeight,mWidth * 2,mHeight * 2);
    }

    Object*		object;
    ObjectSet	objList;

    if(mParentId)
    {
        mSI->getObjectsInRange(this,&objList,ObjType_Player,mWidth);
    }

    if(mQTRegion)
    {
        mQTRegion->mTree->getObjectsInRange(this,&objList,ObjType_Player,&mQueryRect);
    }

    ObjectSet::iterator objIt = objList.begin();

    while(objIt != objList.end())
    {
        object = dynamic_cast<Object*>(*objIt);

        if(!(checkKnownObjects(object)))
        {
            onObjectEnter(object);
        }

        ++objIt;
    }

    PlayerObjectSet oldKnownObjects = mKnownPlayers;
    PlayerObjectSet::iterator objSetIt = oldKnownObjects.begin();

    while(objSetIt != oldKnownObjects.end())
    {
        object = dynamic_cast<Object*>(*objSetIt);

        if(objList.find(object) == objList.end())
        {
            onObjectLeave(object);
        }

        ++objSetIt;
    }
}

//=============================================================================

void SpawnRegion::onObjectEnter(Object* object)
{
    if(object->getParentId() == mParentId)
    {
        //PlayerObject* player = (PlayerObject*)object;
        addKnownObjectSafe(object);
    }
}

//=============================================================================

void SpawnRegion::onObjectLeave(Object* object)
{
    //PlayerObject* player = (PlayerObject*)object;
    removeKnownObject(object);
}

//=============================================================================

