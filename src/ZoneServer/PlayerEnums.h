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

#ifndef		ANH_ZONESERVER_PLAYERENUMS_H
#define 	ANH_ZONESERVER_PLAYERENUMS_H


//=============================================================================

enum EMLocationType
{
	EMLocation_NULL				=	0,
	EMLocation_PlayerStructure	=	1,
	EMLocation_Camp				=	2,
	EMLocation_Cantina			=	3


};

enum EMBuff
{
	EMBuff_Primary		=	0,//primary Mind
	EMBuff_Secondary	=	1//secondary mindstats
};

enum PlayerConnState
{
	PlayerConnState_Connected	= 0,
	PlayerConnState_LinkDead	= 1,
	PlayerConnState_Zoning		= 2,
	PlayerConnState_Destroying	= 3
};

enum IDSession
{
	IDSessionNONE	= 0,
	IDSessionID		= 1,
	IDSessionPREY	= 2
};
//=============================================================================
enum PlayerOffSet
{
	INVENTORY_OFFSET = 1,
	MISSION_OFFSET   = 2,
	DATAPAD_OFFSET   = 3,
	BANK_OFFSET      = 4,
	WEAPON_OFFSET    = 5,
	HAIR_OFFSET      = 8
};

//=============================================================================

enum PlayerFlag
{
	PlayerFlag_Lfg			=	0x00000001,
	PlayerFlag_NoobHelper	=	0x00000002,
	PlayerFlag_RolePlayer	=	0x00000004,
	PlayerFlag_Afk			=	0x00000080,
	PlayerFlag_LinkDead		=	0x00000100,
	PlayerFlag_FactionRank	=	0x00000200,
};

enum PlayerCustomFlag
{
	PlayerCustomFlag_BurstRun				=	0x00000001,
	PlayerCustomFlag_BurstRunCD				=	0x00000002,
	PlayerCustomFlag_LogOut					=	0x00000004,
	PlayerCustomFlag_StructurePlacement		=	0x00000008,//custom flag
	PlayerCustomFlag_InjuryTreatment		=	0x00000016, //custom flag
	PlayerCustomFlag_WoundTreatment			=	0x00000032, //custom flag
	PlayerCustomFlag_QuickHealInjuryTreatment=  0x00000064,
};

//=============================================================================

enum TimerType
{
	Timer_CancelTradeInvitation = 0
};

//=============================================================================

class TimerAsyncContainer
{
	public:

		TimerAsyncContainer(TimerType tT,void* player){ mtT = tT; mPlayer = player; }
		~TimerAsyncContainer(){}

		TimerType			mtT;

		void*				mPlayer;			//playerobject we have work in
		void*				mToBeRemoved;		//player to be removed

};

struct DenyServiceStruct2
{
	string			Name;
};

struct BuffStruct
{
	EMBuff	buffType;
	uint64	customer;
	uint64	buffStart;
	float	buffValuePercent;
	uint32	buffLengthSeconds;
	bool	passive;
};
//=============================================================================

#endif




