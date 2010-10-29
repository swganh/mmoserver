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
    AttrType_CounterUsesRemaining		=	23,
    AttrType_Charges					=	185,
    AttrType_DataVolume					=	244,
    AttrType_factory_count				=	400
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


