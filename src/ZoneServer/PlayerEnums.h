/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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




