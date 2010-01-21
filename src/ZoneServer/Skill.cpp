/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Skill.h"


//======================================================================

Skill::Skill()
{
	mCommands.reserve(50);
	mSkillMods.reserve(25);
	mSchematics.reserve(5);
	mSkillXpTypesList.reserve(5);
	mSkillsRequired.reserve(4);
}

//======================================================================

Skill::~Skill()
{
}

//======================================================================


