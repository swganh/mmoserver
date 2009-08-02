/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_NPC_ENUMS_H
#define ANH_ZONESERVER_NPC_ENUMS_H


//=============================================================================

enum NPCFamily
{
	NpcFamily_Trainer	= 1,
	NpcFamily_Filler	= 2,
	NpcFamily_QuestGiver = 3,
	NpcFamily_AttackableObject = 7,		// Like Debris and stuff we can fight. They have a single health bar. H(am)
	NpcFamily_AttackableCreatures = 8,	// Like npc's and creatures in the wild we can fight. Full HAM.
	NpcFamily_NaturalLairs = 9,			// 
	NpcFamily_MissionLairs = 10,		// 
	NpcFamily_InvisibleLairs = 11		// 
};

//=============================================================================

#endif

