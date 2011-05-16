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

