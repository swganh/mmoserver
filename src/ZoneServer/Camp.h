/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CAMP_H
#define ANH_ZONESERVER_CAMP_H

//#include "tangibleobject.h"
#include "PlayerStructure.h"
#include "CellObject.h"


typedef std::vector<CellObject*>	CellObjectList;
//=============================================================================
class PlayerStructure;


//=============================================================================

class Camp :	public PlayerStructure
{

	public:

		Camp();
		~Camp();

		CampFamily		getCampFamily(){ return mCampFamily; }
		void			setCampFamily(CampFamily bf){ mCampFamily = bf; }

		float			getHealingModifier(){return(mHealingModifier);}
		void			setHealingModifier(float modifier){mHealingModifier = modifier;}


	private:

		float			mWidth;
		float			mHeight;
		uint32			mTotalLoadCount;
		float			mHealingModifier;

		CampFamily		mCampFamily;
};

//=============================================================================

#endif