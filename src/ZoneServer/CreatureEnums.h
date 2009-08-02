/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

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
	PlayerPerformance_Music		= 2,
};

enum PerformingPause
{
	Pause_None					= 0,
	Pause_Start					= 1,
	Pause_Paused				= 2,
};

//=============================================================================

enum CreatureGroup
{
	CreoGroup_Creature		= 0,
	CreoGroup_PersistentNpc	= 1,
	CreoGroup_Player		= 2,
	CreoGroup_Shuttle		= 3,
	CreoGroup_AttackableObject	= 4,	// Better naming?
	CreoGroup_Vehicle		= 5,
};

//=============================================================================

enum CreatureState
{
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
	/*CreatureState_ShipInterior					=	0x0000000100000000,
	CreatureState_PilotingPobShip				=	0x0000000200000000,*/
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
	CreaturePosture_Dead				=	14,
};

//=============================================================================

enum CreatureLocomotion
{

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
	CreaturePvPStatus_Duel			=	0x00000040,
};

//=============================================================================

enum CreatureEquipSlot
{
	CreatureEquipSlot_Head					= 0x00000001,
	CreatureEquipSlot_Neck					= 0x00000002,
	CreatureEquipSlot_Chest1				= 0x00000004,
	CreatureEquipSlot_Chest2				= 0x00000008,
	CreatureEquipSlot_LeftBicep				= 0x00000010,
	CreatureEquipSlot_LeftBracer			= 0x00000020,
	CreatureEquipSlot_RightBicep			= 0x00000040,
	CreatureEquipSlot_RightBracer			= 0x00000080,
	CreatureEquipSlot_Gloves				= 0x00000100,
	CreatureEquipSlot_Belt					= 0x00000200,
	CreatureEquipSlot_Legs1					= 0x00000400,
	CreatureEquipSlot_Legs2					= 0x00000800,
	CreatureEquipSlot_Shoes					= 0x00001000,
	CreatureEquipSlot_LeftFinger			= 0x00002000,
	CreatureEquipSlot_Back					= 0x00004000,
	CreatureEquipSlot_Weapon				= 0x00008000,
	CreatureEquipSlot_MissionBag			= 0x00010000,
	CreatureEquipSlot_Hair					= 0x00020000,
	CreatureEquipSlot_Inventory				= 0x00040000,
	CreatureEquipSlot_Datapad				= 0x00080000,
	CreatureEquipSlot_Bank					= 0x00100000,
	CreatureEquipSlot_Wrists				= 0x00200000,
	CreatureEquipSlot_RightFinger			= 0x00400000,
	CreatureEquipSlot_Instrument			= 0x00800000,
	CreatureEquipSlot_Googles				= 0x01000000,
	CreatureEquipSlot_Mouth					= 0x02000000,
	CreatureEquipSlot_RightWrist			= 0x04000000,
	CreatureEquipSlot_LeftWrist				= 0x08000000,
	CreatureEquipSlot_Rider					= 0x10000000,
};

#define CREATURE_SLOT_COUNT	29
#define CREATURE_MAX_SLOT	1 << CREATURE_SLOT_COUNT

//=============================================================================

#endif



