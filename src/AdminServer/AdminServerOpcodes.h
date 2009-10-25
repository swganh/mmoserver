/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ADMINSERVER_ADMINOPCODES_H
#define ANH_ADMINSERVER_ADMINOPCODES_H


enum admin_opcodes
{
	opClientCreateCharacter				=	0xB97F3074,
	opClientRandomNameRequest			=	0xD6D1B6D1,
	opClientRandomNameResponse			=	0xE85FB868,
	opClientCreateCharacterSuccess		=	0x1DB575CC,
	opClientCreateCharacterFailed		=	0xdf333c6e,
	opLagRequest						=	0x31805EE0,

	opGetMapLocationsMessage			=	0x1a7ab839,
	opGetMapLocationsResponseMessage	=	0x9f80464c,

	opHeartBeat							=	0xa16cf9af
};

#endif

