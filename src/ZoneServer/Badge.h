/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_BADGE_H
#define ANH_ZONESERVER_BADGE_H

#include "utils/typedefs.h"


//======================================================================================================================

class Badge
{
	friend class CharSheetManager;

	public:

		Badge(){}
		~Badge(){}

		uint32	getId(){ return mId; }
		void	setId(uint32 id){ mId = id; }

		string	getName(){ return mName; }
		void	setName(const string name){ mName = name; }

		uint32	getSoundId(){ return mSoundId; }
		void	setSoundId(uint32 sound){ mSoundId = sound; }

		uint8	getCategory(){ return mCategory; }
		void	setCategory(uint8 category){ mCategory = category; }

	private:

		uint32	mId;
		string	mName;
		uint32	mSoundId;
		uint8	mCategory;
};

#endif

