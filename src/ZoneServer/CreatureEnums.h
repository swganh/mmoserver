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
#include <cstdint>

#ifndef ANH_ZONESERVER_CREATURE_ENUMS_H
#define ANH_ZONESERVER_CREATURE_ENUMS_H


//=============================================================================
//
// these reflect tables
//

enum PerformingState
{
    PlayerPerformance_None		= 0,
    PlayerPerformance_Dance		= 1,
    PlayerPerformance_Music		= 2
};

enum PerformingPause
{
    Pause_None					= 0,
    Pause_Start					= 1,
    Pause_Paused				= 2
};

//=============================================================================

enum CreatureGroup
{
    CreoGroup_Creature							= 0,
    CreoGroup_PersistentNpc						= 1,
    CreoGroup_Player							= 2,
    CreoGroup_Shuttle							= 3,
    CreoGroup_AttackableObject					= 4,	// Better naming?
    CreoGroup_Vehicle							= 5
};

//=============================================================================
enum CreatureState :uint64_t
{
    CreatureState_ClearState					=	0x0000000000000000,
    CreatureState_Cover							=	0x0000000000000001,
    CreatureState_Combat						=	0x0000000000000002,
    CreatureState_Peace							=	0x0000000000000004,
    CreatureState_Aiming						=	0x0000000000000008,
    CreatureState_Alert							=	0x0000000000000010,
    CreatureState_Berserk						=	0x0000000000000020,
    CreatureState_FeignDeath					=	0x0000000000000040,
    CreatureState_CombatAttitudeEvasive			=	0x0000000000000080,
    CreatureState_CombatAttitudeNormal			=	0x0000000000000100,
    CreatureState_CombatAttitudeAggressive		=	0x0000000000000200,
    CreatureState_Tumbling						=	0x0000000000000400,
    CreatureState_Rallied						=	0x0000000000000800,
    CreatureState_Stunned						=	0x0000000000001000,
    CreatureState_Blinded						=	0x0000000000002000,
    CreatureState_Dizzy							=	0x0000000000004000,
    CreatureState_Intimidated					=	0x0000000000008000,
    CreatureState_Immobilized					=	0x0000000000010000,
    CreatureState_Frozen						=	0x0000000000020000,
    CreatureState_Swimming						=	0x0000000000040000,
    CreatureState_SittingOnChair				=	0x0000000000080000,
    CreatureState_Crafting						=	0x0000000000100000,
    CreatureState_GlowingJedi					=	0x0000000000200000,
    CreatureState_MaskScent						=	0x0000000000400000,
    CreatureState_Poisoned						=	0x0000000000800000,
    CreatureState_Bleeding						=	0x0000000001000000,
    CreatureState_Diseased						=	0x0000000002000000,
    CreatureState_OnFire						=	0x0000000004000000,
    CreatureState_RidingMount					=	0x0000000008000000,
    CreatureState_MountedCreature				=	0x0000000010000000,
    CreatureState_PilotingShip					=	0x0000000020000000,
    CreatureState_ShipOperations				=	0x0000000040000000,
    CreatureState_ShipGunner					=	0x0000000080000000,
    CreatureState_ShipInterior					=	0x0000000100000000,
    CreatureState_PilotingPobShip				=	0x0000000200000000
};

//=============================================================================
enum CreatureLocomotion :uint64_t
{
    CreatureLocomotion_Standing							=	0x0000000000000001,
    CreatureLocomotion_Sneaking							=	0x0000000000000002,
    CreatureLocomotion_Walking							=	0x0000000000000004,
    CreatureLocomotion_Running							=	0x0000000000000008,
    CreatureLocomotion_Kneeling							=	0x0000000000000010,
    CreatureLocomotion_CrouchSneaking					=	0x0000000000000020,
    CreatureLocomotion_CrouchWalking					=	0x0000000000000040,
    CreatureLocomotion_Prone							=	0x0000000000000080,
    CreatureLocomotion_Crawling							=	0x0000000000000100,
    CreatureLocomotion_ClimbingStationary				=	0x0000000000000200,
    CreatureLocomotion_Climbing							=	0x0000000000000400,
    CreatureLocomotion_Hovering							=	0x0000000000000800,
    CreatureLocomotion_Flying							=	0x0000000000001000,
    CreatureLocomotion_LyingDown						=	0x0000000000002000,
    CreatureLocomotion_Sitting							=	0x0000000000004000,
    CreatureLocomotion_SkillAnimating					=	0x0000000000008000,
    CreatureLocomotion_DrivingVehicle					=	0x0000000000010000,
    CreatureLocomotion_RidingCreature					=	0x0000000000020000,
    CreatureLocomotion_KnockedDown						=	0x0000000000040000,
    CreatureLocomotion_Incapacitated					=	0x0000000000080000,
    CreatureLocomotion_Dead								=	0x0000000000100000,
    CreatureLocomotion_Blocking							=	0x0000000000200000
};
//=============================================================================
enum CreaturePosture
{
    CreaturePosture_Upright				=	0,
    CreaturePosture_Crouched			=	1,
    CreaturePosture_Prone				=	2,
    CreaturePosture_Sneaking			=	3,
    CreaturePosture_Blocking			=	4,
    CreaturePosture_Climbing			=	5,
    CreaturePosture_Flying				=	6,
    CreaturePosture_LyingDown			=	7,
    CreaturePosture_Sitting				=	8,
    CreaturePosture_SkillAnimating		=	9,
    CreaturePosture_DrivingVehicle		=	10,
    CreaturePosture_RidingCreature		=	11,
    CreaturePosture_KnockedDown			=	12,
    CreaturePosture_Incapacitated		=	13,
    CreaturePosture_Dead				=	14
};

//=============================================================================

enum CreaturePvPStatus
{
    CreaturePvPStatus_None			=	0x00000000,
    CreaturePvPStatus_Attackable	=	0x00000001,
    CreaturePvPStatus_Aggressive	=	0x00000002,
    CreaturePvPStatus_Overt			=	0x00000004,
    CreaturePvPStatus_Tef			=	0x00000008,
    CreaturePvPStatus_Player		=	0x00000010,
    CreaturePvPStatus_Enemy			=	0x00000020,
    CreaturePvPStatus_Duel			=	0x00000040
};

//=============================================================================

enum CreatureEquipSlot :
unsigned long long
{
    CreatureEquipSlot_Back					= 0x000000001,
    CreatureEquipSlot_Bandolier				= 0x000000002,
    CreatureEquipSlot_Bank					= 0x000000004,
    CreatureEquipSlot_Bicep_Left			= 0x000000008,
    CreatureEquipSlot_Bicep_Right			= 0x000000010,
    CreatureEquipSlot_Bracer_Lower_Left		= 0x000000020,
    CreatureEquipSlot_Bracer_Lower_Right	= 0x000000040,
    CreatureEquipSlot_Bracer_Upper_Left		= 0x000000080,
    CreatureEquipSlot_Bracer_Upper_Right	= 0x000000100,
    CreatureEquipSlot_Chest1				= 0x000000200,
    CreatureEquipSlot_Chest2				= 0x000000400,
    CreatureEquipSlot_Chest3_Left			= 0x000000800,
    CreatureEquipSlot_Chest3_Right			= 0x000001000,
    CreatureEquipSlot_Cloak					= 0x000002000,
    CreatureEquipSlot_Datapad				= 0x000004000,
    CreatureEquipSlot_Earring_Left			= 0x000008000,
    CreatureEquipSlot_Earring_Right			= 0x000010000,
    CreatureEquipSlot_Eyes					= 0x000020000,
    CreatureEquipSlot_Gloves				= 0x000040000,
    CreatureEquipSlot_Hair					= 0x000080000,
    CreatureEquipSlot_Hat					= 0x000100000,
    CreatureEquipSlot_Hold_Left				= 0x000200000,
    CreatureEquipSlot_Hold_Right			= 0x000400000,
    CreatureEquipSlot_Hold_Both				= 0x000600000,
    CreatureEquipSlot_Inventory				= 0x000800000,
    CreatureEquipSlot_Mission				= 0x001000000,
    CreatureEquipSlot_Mouth					= 0x002000000,	
    CreatureEquipSlot_Neck					= 0x004000000,
    CreatureEquipSlot_Pants1				= 0x008000000,
    CreatureEquipSlot_Pants2				= 0x010000000,
    CreatureEquipSlot_Finger_Left			= 0x020000000,
    CreatureEquipSlot_Finger_Right			= 0x040000000,
    CreatureEquipSlot_Shoes					= 0x080000000,
    CreatureEquipSlot_Utility_Belt			= 0x100000000,
    CreatureEquipSlot_Wrist_Left			= 0x200000000,
    CreatureEquipSlot_Wrist_Right			= 0x400000000
};

#define CREATURE_SLOT_COUNT	35ULL
#define CREATURE_MAX_SLOT	1ULL << CREATURE_SLOT_COUNT

//=============================================================================

#endif



