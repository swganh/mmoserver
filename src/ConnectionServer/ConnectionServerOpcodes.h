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

	opHeartBeat					=	0xa16cf9af,
};

#endif

