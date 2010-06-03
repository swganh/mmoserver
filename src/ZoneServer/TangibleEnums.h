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

#ifndef ANH_ZONESERVER_TANGIBLE_ENUMS_H
#define ANH_ZONESERVER_TANGIBLE_ENUMS_H

//=============================================================================
//
// Tangible groups
//
enum TangibleGroup
{
	TanGroup_None					= 0,
	TanGroup_PlayerInternal			= 1,
	TanGroup_Inventory				= 2,
	TanGroup_Hair					= 3,
	TanGroup_Terminal				= 4,
	TanGroup_Container				= 5,
	TanGroup_TicketCollector		= 6,
	TanGroup_Item					= 7,
	TanGroup_ResourceContainer		= 8,
	TanGroup_Datapad				= 9,
	TanGroup_Structure				= 10,
	TanGroup_Static					= 11,
	TanGroup_ManufacturingSchematic	= 12,
	TanGroup_Hopper					= 13
};


//=============================================================================

// these reflect type ids from the db type tables
enum TangibleType
{
	TanType_None			= 0,
//=============================================================================
//
// Group: Hair
//
	TanType_Hair			= 1,

//=============================================================================
//
// Group: PlayerInternal
//
	TanType_Bank			= 1,
	TanType_MissionBag		= 3,

//=============================================================================
//
// Group: Datapad
//
	TanType_CharacterDatapad	= 1,
	TanType_DroidDatapad		= 2,

//=============================================================================
//
// Group: Inventory
//
	TanType_CharInventory		= 1,
	TanType_CreatureInventory	= 2,

//=============================================================================
//
// Group: Terminal
//
	TanType_MissionTerminal				= 1,
	TanType_BazaarTerminal				= 2,
	TanType_BankTerminal				= 3,
	TanType_RebelMissionTerminal		= 4,
	TanType_HQTerminal					= 5,
	TanType_BountyMissionTerminal		= 6,
	TanType_SpaceTerminal				= 7,
	TanType_ImperialMissionTerminal		= 8,
	TanType_NewsNetTerminal				= 9,
	TanType_ScoutMissionTerminal		= 10,
	TanType_EntertainerMissionTerminal	= 11,
	TanType_ArtisanMissionTerminal		= 12,
	TanType_BestineQuest1Terminal		= 13,
	TanType_BestineQuest2Terminal		= 14,
	TanType_ElevatorTerminal			= 15,
	TanType_TravelTerminal				= 16,
	TanType_InsuranceTerminal			= 17,
	TanType_CloningTerminal				= 18,
	TanType_BallotBoxTerminal			= 19,
	TanType_BountyDroidTerminal			= 20,
	TanType_GuildTerminal				= 21,
	TanType_MissionNewbieTerminal		= 22,
	TanType_MissionStatueTerminal		= 23,
	TanType_NewbieClothingTerminal		= 24,
	TanType_NewbieFoodTerminal			= 25,
	TanType_NewbieInstrumentTerminal	= 26,
	TanType_NewbieMedicineTerminal		= 27,
	TanType_NewbieToolTerminal			= 28,
	TanType_PlayerStructureTerminal		= 29,
	TanType_HQRebelTerminal				= 30,
	TanType_HQImperialTerminal			= 31,
	TanType_PMRegisterTerminal			= 32,
	TanType_SKillTerminal				= 33,
	TanType_CityTerminal				= 34,
	TanType_PlayerStructureNoSnapTerm	= 35,
	TanType_CityVoteTerminal			= 36,
	TanType_PlayerStructureNoSnapMini	= 37,
	TanType_NymCaveTerminal				= 38,
	TanType_CommandConsoleTerminal		= 39,
	TanType_GeoBunkerTerminal			= 40,
	TanType_BestineQuests3				= 41,
	TanType_ElevatorUpTerminal			= 42,
	TanType_ElevatorDownTerminal		= 43,
	TanType_HQTurrentControlTermainl	= 44,
	TanType_ImageDesignTerminal			= 45,
	TanType_WaterPressureTerminal		= 46,
	TanType_Light_Enc_VotingTerminal	= 47,
	TanType_Dark_Enc_ChallengeTerminal	= 48,
	TanType_Dark_Enc_VotingTerminal		= 49,
	TanType_CharacterBuilderTerminal	= 50,
	TanType_ShipInteriorSecurity1		= 51,
	TanType_POBShipTerminal				= 52,
	TanType_Light_Enc_ChallengeTerminal = 53,
	TanType_CampTerminal				= 54,

//=============================================================================
//
// Group: TicketCollector
//

	TanType_TicketCollector	= 1

//=============================================================================
//
// Group: Container
//

};

//=============================================================================

#endif

