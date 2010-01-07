/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_OBJECT_ENUMS_H
#define ANH_ZONESERVER_OBJECT_ENUMS_H

//=============================================================================

enum AttributeType
{
	AttrType_None						=	0,
	AttrType_Volume						=	1,
	AttrType_serial_number				=	16,
	AttrType_crafter					=	17,
	AttrType_CraftToolStatus			=	18,
	AttrType_CraftToolTime				=	22,
	AttrType_CounterUsesRemaining		=	23
};

enum ObjectType
{
	ObjType_None				=	0,
	ObjType_Creature			=	1,
	ObjType_Player				=	2,
	ObjType_NPC					=	4,
	ObjType_Cell				=	8,
	ObjType_Tangible			=	16,
	ObjType_Building			=	32,
	ObjType_Lair				=	64,
	ObjType_Mission				=	128,
	ObjType_DraftSchematic		=	256,
	ObjType_Waypoint			=	512,
	ObjType_Region				=	1024,
	ObjType_NonPersistant		=	2048,
	ObjType_Structure			=	4096,
	
	ObjType_Intangible			=	16384

	//might it make sense to put them together to object type structure ?
};

//=============================================================================

enum ObjectLoadState
{
	LoadState_Loading				= 1,
	LoadState_Loaded				= 2,
	LoadState_Attributes			= 3,
	LoadState_Tangible_Data			= 4,
	LoadState_Creature_Skills		= 5,
	LoadState_Creature_XP			= 6,
	LoadState_Creature_Badges		= 7,
	LoadState_Creature_Factions		= 8,
	LoadState_ContainerContent		= 9
};

//=============================================================================

enum EquipSlot
{
	EquipSlot_Head					= 1,
	EquipSlot_Neck					= 2,
	EquipSlot_Chest1				= 4,
	EquipSlot_Chest2				= 8,
	EquipSlot_LeftBicep				= 16,
	EquipSlot_LeftBracer			= 32,
	EquipSlot_RightBicep			= 64,
	EquipSlot_RightBracer			= 128,
	EquipSlot_Gloves				= 256,
	EquipSlot_Belt					= 512,
	EquipSlot_Legs1					= 1024,
	EquipSlot_Legs2					= 2048,
	EquipSlot_Shoes					= 4096,
	EquipSlot_Finger				= 8192,
	EquipSlot_Back					= 16384,
	EquipSlot_Weapon				= 32768,
	EquipSlot_MissionBag			= 65536,
	EquipSlot_Hair					= 131072,
	EquipSlot_Inventory				= 262144,
	EquipSlot_Datapad				= 524288,
	EquipSlot_Bank					= 1048576,
	EquipSlot_Wrists				= 2097152
};

//=============================================================================

#endif


