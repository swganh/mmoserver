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
    opTutorialServerStatusRequest			= 0x5E48A399,
    opTutorialServerStatusReply				= 0x989EDF5A,
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
    opDeleteCharacterReplyMessage			= 0x8268989b,
    opLauncherSessionOpen                   = 0x486f6f6e,
    opLauncherSessionCreated                = 0x2e4e6574
};


#endif // ANH_COMMON_MESSAGEOPCODES_H




