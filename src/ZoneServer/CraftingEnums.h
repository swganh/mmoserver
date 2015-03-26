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

#ifndef ANH_ZONESERVER_CRAFTINGENUMS_H
#define ANH_ZONESERVER_CRAFTINGENUMS_H


//=============================================================================

enum CraftError
{
    CraftError_None										= 0,
    CraftError_Internal_No_Owner						= 1,
    CraftError_Not_In_Assembly_Stage					= 2,
    CraftError_Not_In_Customization_Stage				= 3,
    CraftError_Internal_No_Draft_Schematic				= 4,
    CraftError_Internal_No_Tool							= 5,
    CraftError_Internal_No_Manufacture_Schematic		= 6,
    CraftError_Invalid_Slot								= 7,
    CraftError_Invalid_Slot_Option						= 8,
    CraftError_Internal_Invalid_Ingredient_Size			= 9,
    CraftError_Slot_Already_Full						= 10,
    CraftError_Internal_Invalid_Ingredient				= 11,
    CraftError_Ingredient_Not_In_Inventory				= 12,
    CraftError_Internal_Cant_Remove_Resource_From_Crate	= 13,
    CraftError_Bad_Resource_Chosen						= 14,
    CraftError_Component_Too_Damaged					= 15,
    CraftError_Internal_Cant_Transfer_Component			= 16,
    CraftError_Bad_Component_Chosen						= 17,
    CraftError_Internal_Cant_Find_Inventory				= 18,
    CraftError_Internal_Bad_Target_Container			= 19,
    CraftError_Slot_Already_Empty						= 20,
    CraftError_Internal_Cant_Create_Resource_Container	= 21,
    CraftError_Trying_Assemble_With_Empty_Slot			= 22,
    CraftError_Trying_Assemble_With_Partial_Full_Slot	= 23,
    CraftError_Internal_No_Test_Prototype_Found			= 24,
    CraftError_Name_Reserved							= 25,
    CraftError_Schematic_Cant_Be_Used					= 26,
    CraftError_Failed_Transfer_Resources_To_Station		= 27
};

//=============================================================================

enum CraftExperimentResult
{
    CraftExp_Amazing_Success	= 0,
    CraftExp_Great_Success		= 1,
    CraftExp_Good_Success		= 2,
    CraftExp_Moderate_Success	= 3,
    CraftExp_Success			= 4,
    CraftExp_Failure			= 5,
    CraftExp_Moderate_Failure	= 6,
    CraftExp_Big_Failure		= 7,
    CraftExp_Critical_Failure	= 8
};

//=============================================================================

enum CraftAssemblyResult
{
    CraftAss_Amazing_Success	= 0,
    CraftAss_Great_Success		= 1,
    CraftAss_Good_Success		= 2,
    CraftAss_Moderate_Success	= 3,
    CraftAss_Success			= 4,
    CraftAss_Failure			= 5,
    CraftAss_Moderate_Failure	= 6,
    CraftAss_Big_Failure		= 7,
    CraftAss_Critical_Failure	= 8
};

//=============================================================================

enum CraftCreateResult
{
    CraftCreate_Failure	= 0,
    CraftCreate_Success	= 1,
    CraftCreate_2		= 2
};

//=============================================================================

enum CraftSessionQuery
{
    CraftSessionQuery_Prototype				= 1,
    CraftSessionQuery_SkillmodExp			= 2,
    CraftSessionQuery_SkillmodAss			= 3,
    CraftSessionQuery_CustomizationData		= 4
};

//=============================================================================

#endif


