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

#include "City.h"
#include "PlayerObject.h"
#include "QTRegion.h"
#include "QuadTree.h"
#include "WorldManager.h"
#include "ZoneTree.h"

//=============================================================================

City::City() : RegionObject(),
mSI(gWorldManager->getSI()),
mQTRegion(NULL)
{
	mActive		= false;
	mRegionType = Region_City;
}

//=============================================================================

City::~City()
{
}

//=============================================================================

void City::update()
{
	if(!mSubZoneId)
	{
		mQTRegion	= mSI->getQTRegion(mPosition.x,mPosition.z);
		mSubZoneId	= (uint32)mQTRegion->getId();
		mQueryRect	= Anh_Math::Rectangle(mPosition.x - mWidth,mPosition.z - mHeight,mWidth * 2,mHeight * 2);
	}

	Object*		object;
	ObjectSet	objList;
	
	mSI->getObjectsInRange(this,&objList,ObjType_Player,mWidth);

	if(mQTRegion)
	{
		mQTRegion->mTree->getObjectsInRange(this,&objList,ObjType_Player,&mQueryRect);
	}

	ObjectSet::iterator objIt = objList.begin();

	while(objIt != objList.end())
	{
		object = (*objIt);

		if(!(checkKnownObjects(object)))
		{
			onObjectEnter(object);
		}

		++objIt;
	}

	ObjectSet oldKnownObjects = mKnownObjects;
	ObjectSet::iterator objSetIt = oldKnownObjects.begin();

	while(objSetIt != oldKnownObjects.end())
	{
		object = (*objSetIt);

		if(objList.find(object) == objList.end())
		{
			onObjectLeave(object);
		}

		++objSetIt;
	}
}

//=============================================================================

void City::onObjectEnter(Object* object)
{
	PlayerObject* player = (PlayerObject*)object;
	//player->setCityRegionId(this->getId());

	addKnownObjectSafe(object);

	gLogger->logMsgF("%s entered %s (%u players in city)",MSG_HIGH,player->getFirstName().getAnsi(),mCityName.getAnsi(),mKnownPlayers.size());
}

//=============================================================================

void City::onObjectLeave(Object* object)
{

	PlayerObject* player = (PlayerObject*)object;

	//if(player->getCityRegionId() == this->getId())
		//player->setCityRegionId(0);

	removeKnownObject(object);

	gLogger->logMsgF("%s left %s (%u players in city)",MSG_HIGH,player->getFirstName().getAnsi(),mCityName.getAnsi(),mKnownPlayers.size());
}

//=============================================================================

