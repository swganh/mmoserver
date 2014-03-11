/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#ifndef ANH_ZONESERVER_STRUCTUREMANAGER_TYPES1_H
#define ANH_ZONESERVER_STRUCTUREMANAGER_TYPES1_H

#include "ZoneServer\GameSystemManagers\UI Manager\UIEnums.h"

enum WindowQueryType
{
    Window_Query_NULL						=	0,
    Window_Query_Radioactive_Sample			=	1,
    Window_Query_Add_Schematic				=	2

};

enum Structure_QueryType
{
    Structure_Query_NULL						=	0,
    Structure_Query_LoadDeedData				=	1,
    Structure_Query_LoadstructureItem			=	2,

    
    Structure_Query_Add_Permission				=	5,
    Structure_Query_Remove_Permission			=	6,
    Structure_Query_Check_Permission			=	7,
    Structure_StructureTransfer_Lots_Recipient	=	8,
    Structure_StructureTransfer_Lots_Donor		=	9,
    Structure_HopperUpdate						=	10,
    Structure_HopperDiscard						=	11,
    Structure_GetResourceData					=	12,
    Structure_ResourceDiscardUpdateHopper		=	13,
    Structure_ResourceDiscard					=	14,
    Structure_ResourceRetrieve					=	15,

    Structure_GetInactiveHarvesters	 			=	16,
    Structure_GetDestructionStructures			=	17,
    Structure_UpdateStructureDeed				=	18,
    Structure_UpdateCharacterLots				=	19,
    Structure_UpdateAttributes					=	20,
    
    Structure_Query_UpdateAdminPermission		=	23,

    Structure_Query_NoBuildRegionData			=	24

};

enum Structure_Async_CommandEnum
{
    Structure_Command_NULL				=	0,
    Structure_Command_AddPermission		=	1,
    Structure_Command_RemovePermission	=	2,
    Structure_Command_Destroy			=	3,
    
    Structure_Command_TransferStructure	=	6,
    Structure_Command_RenameStructure	=	7,
    Structure_Command_DiscardHopper		=	8,
    Structure_Command_GetResourceData	=	9,
    Structure_Command_DiscardResource	=	10,
    Structure_Command_RetrieveResource	=	11,
    Structure_Command_PayMaintenance	=	12,
    Structure_Command_DepositPower		=	13,
    Structure_Command_ViewStatus		=	14,
    Structure_Command_ViewStatus_Att2	=	15,
    Structure_Command_OperateHarvester	=	16,
    Structure_Command_AccessSchem		=	17,
    Structure_Command_AddSchem			=	18,
    Structure_Command_RemoveSchem		=	19,
    Structure_Command_AccessInHopper	=	20,
    Structure_Command_AccessOutHopper	=	21,
    Structure_Command_StartFactory		=	22,
    Structure_Command_StopFactory		=	23,    
    Structure_Command_CellEnter			=	27,
    Structure_Command_CellEnterDenial	=	28


};

struct StructureAsyncCommand
{
    uint64						HopperId;
    uint64						StructureId;
    uint64						PlayerId;
    uint64						RecipientId;
    uint64						ResourceId;
    uint64						SchematicId;
    uint32						Amount;
    uint8						b1;
    uint8						b2;
    BString						CommandString;
    std::string						PlayerStr;
    BString						List;
    BString						Name;
    Structure_Async_CommandEnum	Command;
};

struct attributeDetail
{
    BString	value;
    uint32	attributeId;
};



//links deeds to structuredata
//TODO still needs to be updated to support several structure types for some placeables
//depending on customization

struct StructureDeedLink
{
    uint32	structure_type;
    uint64	placementMask;
    uint32	skill_Requirement;
    uint32	repair_cost;
    uint32	item_type;
    BString	structureObjectString;
    uint8	requiredLots;
    BString	stf_file;
    BString	stf_name;
    float	healing_modifier;

    uint32 length;
    uint32 width;
};

//templated items that need to be at certain spots on/in the structure
//like signs / campfires/elevator buttons and stuff
struct StructureItemTemplate
{
    uint32				CellNr;
    uint32				structure_id;
    uint32				item_type;
    BString				structureObjectString;

    TangibleGroup		tanType;

    BString				name;
    BString				file;

    glm::vec3	mPosition;
    glm::quat	mDirection;
};
// no build regions
struct NoBuildRegionTemplate
{
    uint32				region_id;
    BString				region_name;
    glm::vec3			mPosition;
    float				width;
    float				height;
    uint32				planet_id;
    uint32				build;
    uint32				no_build_type;
    float				mRadius;
    float				mRadiusSq;
    bool				isCircle;
};

class Type_QueryContainer
{
public:

    Type_QueryContainer() {}

    uint64	mId;
    BString	mString;
    BString	mValue;
};

#endif