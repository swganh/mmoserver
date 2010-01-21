/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CAMPREGION_H
#define ANH_ZONESERVER_CAMPREGION_H

#include "RegionObject.h"
#include "WorldManager.h"
#include "MathLib/Rectangle.h"
#include "Utils/typedefs.h"


//=============================================================================

class ZoneTree;
class PlayerObject;
class QTRegion;

//=============================================================================

typedef std::set<uint64>						VisitorSet;
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
				
				uint32	getVisitors(){return(mVisitorSet.size());}
				uint32	getCurrentVisitors(){return(mKnownPlayers.size());}
				

				void	setCampOwnerName(string name){mOwnerName = name;}
				string	getCampOwnerName(){return mOwnerName;}

				void	setHealingModifier(float mod){mHealingModifier = mod;}
				float	getHealingModifier(){return mHealingModifier;}

				void	despawnCamp();
				void	applyXp();

	protected:

		ZoneTree*			mSI;
		QTRegion*			mQTRegion;
		Anh_Math::Rectangle mQueryRect;
		uint64				mCampId;
		uint64				mOwnerId;
		bool				mAbandoned;
		uint64				mSetUpTime;
		uint64				mLeftTime;
		uint32				mXpMax;
		uint32				mXp;
		string				mOwnerName;
		float				mHealingModifier;

		bool				mDestroyed;

		VisitorSet			mVisitorSet;
};


#endif
