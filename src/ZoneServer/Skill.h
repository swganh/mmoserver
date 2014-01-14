/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SKILL_H
#define ANH_ZONESERVER_SKILL_H

#include "Utils/typedefs.h"
#include <vector>


//======================================================================

typedef std::pair<uint32,int32>					SkillMod;
typedef std::vector<uint32>						SpeciesList;
typedef std::vector<uint32>						SkillCommandList;
typedef std::vector<SkillMod>					SkillModsList;
typedef std::vector<uint32>						SchematicGroupsList;
typedef std::vector<uint32>						SkillsPreclusionsList;
typedef std::vector<uint32>						SkillsRequiredList;
typedef std::vector<uint32>						SkillXpTypesList;

//======================================================================

class Skill
{
	friend class SkillManager;

	public:

		Skill();
		~Skill();

		uint32					mId;
		string					mName;
		uint8					mGodOnly;
		uint8					mIsSearchable;
		uint8					mIsTitle;
		uint8					mIsProfession;
		int32					mMoneyRequired;
		uint8					mSkillPointsRequired;
		uint8					mJediStateRequired;
		uint8					mXpType;
		int32					mXpCost;
		int32					mXpCap;
		int32					mBadgeId;
		SpeciesList				mSpeciesRequired;
		SkillCommandList		mCommands;
		SkillModsList			mSkillMods;
		SchematicGroupsList		mSchematics;
		SkillsPreclusionsList	mSkillPrecusions;
		SkillsRequiredList		mSkillsRequired;
		SkillXpTypesList		mSkillXpTypesList;
};

//======================================================================

#endif



