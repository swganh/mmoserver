/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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

		string				getRegionName(){ return mRegionName; }
		void				setRegionName(const string name){ mRegionName = name; }

		string				getNameFile(){ return mNameFile; }
		void				setNameFile(const string namefile){ mNameFile = namefile; }

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

	protected:

		RegionType			mRegionType;
		float				mWidth;
		float				mHeight;
		string				mRegionName;
		string				mNameFile;
		bool				mActive;
};


#endif

