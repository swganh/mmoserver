/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006, 2007 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_LOGINSERVER_LOGINOPCODES_H
#define ANH_LOGINSERVER_LOGINOPCODES_H


enum login_opcodes
{
	opLoginClientToken					=	0xAAB296C6,
	opLoginEnumCluster					=	0xC11C63B9,
	opLoginClusterStatus				=	0x3436AEB6,
	opEnumerateCharacterId				=	0x65EA4574,
	opLoginClientId						=	0x41131f96,
	opDeleteCharacterMessage			=	0xe87ad031,
	opDeleteCharacterReplyMessage		=	0x8268989b,

	opHeartBeat							=	0xa16cf9af,
};

#endif

