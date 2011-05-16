/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_SKILL_H
#define ANH_ZONESERVER_SKILL_H

#include "Utils/bstring.h"
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
    BString					mName;
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



