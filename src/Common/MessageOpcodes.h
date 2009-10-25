/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_COMMON_MESSAGEOPCODES_H
#define ANH_COMMON_MESSAGEOPCODES_H



//======================================================================================================================

enum MessageOpcodes
{
	opClusterRegisterServer					= 0xb88f5166,
	opClusterClientConnect					= 0x6B9E5323,
	opClusterClientDisconnect				= 0x44e7e4fa,
	opClusterZoneTransferRequestByTicket	= 0x12BEB7E8,
	opClusterZoneTransferRequestByPosition	= 0xE259B191,
	opClusterZoneTransferApprovedByPosition	= 0xF5E12559,
	opClusterZoneTransferApprovedByTicket	= 0xA608F0B2,
	opClusterZoneTransferDenied				= 0x7B4AF214,
	opClusterZoneTransferCharacter			= 0x74C4FC34,

	opSelectCharacter						= 0xb5098d76,
	opConGenericMessage						= 0x08C5FC76,
	opErrorMessage							= 0xb5abf91a,
	opClientIdMsg							= 0xd5899226,
	opClientPermissionsMessage				= 0xE00730E5,
	opHeartBeat								= 0xa16cf9af,
	opLoginClientToken						= 0xAAB296C6,
	opLoginEnumCluster						= 0xC11C63B9,
	opLoginClusterStatus					= 0x3436AEB6,
	opEnumerateCharacterId					= 0x65EA4574,
	opLoginClientId							= 0x41131f96,
	opDeleteCharacterMessage				= 0xe87ad031,
	opDeleteCharacterReplyMessage			= 0x8268989b
};


#endif // ANH_COMMON_MESSAGEOPCODES_H




