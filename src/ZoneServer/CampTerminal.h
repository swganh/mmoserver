/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_CAMPTERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_CAMPTERMINAL_H

#include "Terminal.h"

//=============================================================================

class CampTerminal : public Terminal
{
	friend class TerminalFactory;

	public:

		CampTerminal();
		~CampTerminal();

		virtual void			prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void			handleObjectMenuSelect(uint8 messageType,Object* srcObject);

		void					setOwner(uint64 owner){mOwnerId = owner;}
		uint64					getOwner(){return mOwnerId;}

		void					setCamp(uint64 camp){mCampId = camp;}
		uint64					getCamp(){return mCampId;}

		void					setCampRegion(uint64 region){mCampRegionId = region;}
		uint64					getCampRegion(){return mCampRegionId;}

	private:
		uint64					mCampId;	//id of the tent - has the object list
		uint64					mOwnerId;
		BStringVector			mAttributesMenu;
		uint64					mCampRegionId;

		
};

//=============================================================================

#endif

