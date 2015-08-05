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

#ifndef ANH_ZONESERVER_CAMPREGION_H
#define ANH_ZONESERVER_CAMPREGION_H

#include "RegionObject.h"
#include "WorldManager.h"
#include "MathLib/Rectangle.h"
#include "Utils/typedefs.h"


//=============================================================================


class PlayerObject;


//=============================================================================


class CampRegion : public RegionObject
{
public:

    CampRegion();
    virtual ~CampRegion();


    virtual void	update();
    virtual void	onObjectEnter(Object* object);
    virtual void	onObjectLeave(Object* object);

    void	setOwner(uint64 owner){mOwnerId = owner;}
	uint64	getOwner(){return mOwnerId;}

	void	setAbandoned(bool mmh){mAbandoned = mmh;}
	uint64	getAbandoned(){return mAbandoned;}
				
	void	setMaxXp(uint32 max){mXpMax = max;}
	uint32	getMaxXp(){return mXpMax;}

	void	setCamp(uint64 id){mCampId = id;}
	uint64	getCamp(){return mCampId;}

	uint64	getUpTime(){return((gWorldManager->GetCurrentGlobalTick() - mSetUpTime)/1000);}
				
	uint32	getVisitors(){return(links.size());}
	uint32	getCurrentVisitors(){return(mVisitingPlayers.size());}

	void	setCampOwnerName(std::string name){mOwnerName = name;}
	std::string	getCampOwnerName(){return mOwnerName;}

	void	setHealingModifier(float mod){mHealingModifier = mod;}
	float	getHealingModifier(){return mHealingModifier;}

	void	despawnCamp();
	void	applyHAMHealing(Object* object);
	void	applyWoundHealing(Object* object);
	void	applyXp();

	protected:

		uint64				mCampId;
		uint64				mOwnerId;
		bool				mAbandoned;
		uint64				mSetUpTime;
		//uint64				mLeftTime;
		uint64				mExpiresTime;
		uint32				mXpMax;
		uint32				mXp;
		std::string			mOwnerName;
		float				mHealingModifier;

		uint32				mHealingDone;

		bool				mDestroyed;

		struct				campLink;
		std::list<campLink*>	links;
};
#endif
