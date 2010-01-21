/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_BANK_H
#define ANH_ZONESERVER_TANGIBLE_BANK_H

#include "TangibleObject.h"

class PlayerObject;

//=============================================================================

class Bank : public TangibleObject
{
	friend class ObjectFactory;
	friend class PlayerObjectFactory;

	public:

		Bank();
		~Bank();

		void		setParent(PlayerObject* player){ mParent = player; }
		ObjectList*	getObjects(){ return &mObjects; }

		int32		getCredits(){ return mCredits; }
		void		setCredits(int32 credits){ mCredits = credits; }
		bool		updateCredits(int32 amount);

		int8		getPlanet(){ return mPlanet; }
		void		setPlanet(int8 planet){ mPlanet = planet; }
	
	private:

		PlayerObject*	mParent;
		ObjectList		mObjects;
		int32			mCredits;
		int8			mPlanet;
};

//=============================================================================

#endif

