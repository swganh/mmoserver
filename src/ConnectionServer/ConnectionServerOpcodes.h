/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006, 2007 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CONNECTIONSERVER_CONNECTIONOPCODES_H
#define ANH_CONNECTIONSERVER_CONNECTIONOPCODES_H


enum connection_opcodes
{
	opClientIdMsg				=	0xd5899226,
	opClientPermissionsMessage	=	0xE00730E5,
	opClusterClientDisconnect	=	0x44e7e4fa,

	opClientCreateCharacter				=	0xB97F3074,
	opClientRandomNameRequest			=	0xD6D1B6D1,
	opClientRandomNameResponse			=	0xE85FB868,
	opClientCreateCharacterSuccess		=	0x1DB575CC,
	opClientCreateCharacterFailed		=	0xdf333c6e,
	opLagRequest						=	0x31805EE0,

	opGetMapLocationsMessage			=	0x1a7ab839,
	opGetMapLocationsResponseMessage	=	0x9f80464c,

	opHeartBeat					=	0xa16cf9af,
};

#endif

