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

#ifndef ANH_ZONESERVER_REGION_OBJECT_H
#define ANH_ZONESERVER_REGION_OBJECT_H

#include "Object.h"

//=============================================================================

enum RegionType
{
    Region_City				= 1,
    Region_Spawn			= 2,
    Region_Badge			= 3,
    Region_Mission			= 4,
    Region_Zone				= 5,
    Region_Camp				= 6,
    Region_MissionSpawn		= 7
};

//=============================================================================

class RegionObject : public Object
{
public:
	RegionObject();
	virtual ~RegionObject();

	std::string			getRegionName(){ return mRegionName; }
	void				setRegionName(const std::string name){ mRegionName = name; }

	std::string			getNameFile(){ return mNameFile; }
	void				setNameFile(const std::string namefile){ mNameFile = namefile; }

	RegionType			getRegionType(){ return mRegionType; }
	void				setType(RegionType rType){ mRegionType = rType; }

	float				getWidth(){ return mWidth; }
	void				setWidth(float width){ mWidth = width; }

	float				getHeight(){ return mHeight; }
	void				setHeight(float height){ mHeight = height; }

	bool				getActive(){ return mActive; }
	void				setActive(bool a){ mActive = a; }

	virtual void		update(){}
	virtual void		onObjectEnter(Object* object){}
	virtual void		onObjectLeave(Object* object){}

	bool				addVisitor(Object* visitor);
	void				removeVisitor(Object* visitor);
		
	bool				checkVisitor(Object* object);

    std::shared_ptr<RegionObject> getSharedFromThis();
protected:

	ObjectIDSet			mVisitingPlayers;
	RegionType			mRegionType;
	float				mWidth;
	float				mHeight;
	std::string			mRegionName;
	std::string			mNameFile;
	bool				mActive;
};


#endif

