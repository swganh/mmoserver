/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

