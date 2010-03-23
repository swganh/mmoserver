/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_WORLDMANAGER_ENUMS_H
#define ANH_ZONESERVER_WORLDMANAGER_ENUMS_H


//======================================================================================================================
//
// Database Queries
//

enum WMLogOut
{
	WMLogOut_No_LogOut			= 0,
	WMLogOut_LogOut				= 1,
	WMLogOut_Zone_Transfer		= 2,
	WMLogOut_Char_Load			= 3 //save the present char and load a new one

};

enum WMQuery
{
	WMQuery_ObjectCount				= 0,
	WMQuery_All_Buildings			= 1,
	WMQuery_Cells					= 2,
	WMQuery_Terminals				= 3,
	WMQuery_Furniture				= 4,
	WMQuery_Containers				= 5,
	WMQuery_Wearables				= 6,
	WMQuery_Foods					= 7,
	WMQuery_SavePlayer_Position		= 8,
	WMQuery_SavePlayer_Attributes	= 9,
	WMQuery_Inventory				= 10,
	WMQuery_CraftingStations		= 11,
	WMQuery_TicketCollectors		= 12,
	WMQuery_Trainers				= 13,
	WMQuery_FillerNPCs				= 14,
	WMQuery_Shuttles				= 15,
	WMQuery_TravelTickets			= 16,
	WMQuery_CraftingTools			= 17,
	WMQuery_SurveyTools				= 18,
	WMQuery_ClientEffects			= 19,
	WMQuery_PlanetNamesAndFiles		= 20,
	WMQuery_ResourceContainers		= 21,
	WMQuery_AllObjectsChildObjects	= 22,
	WMQuery_Moods					= 23,
	WMQuery_Cities					= 24,
	WMQuery_Sounds					= 25,
	WMQuery_BadgeRegions			= 26,
	WMQuery_AttributeKeys			= 27,
	WMQuery_NpcConverseAnimations	= 28,
	WMQuery_NpcChatter				= 29,
	WMQuery_WorldScripts			= 30,
	WMQuery_ZoneRegions				= 31,
	WMQuery_SpawnRegions			= 32,
	WMQuery_SavePlayer_Buff			= 33,
	WMQuery_CreatureSpawnRegions	= 34,
	WMQuery_Harvesters				= 35,
	WMQuery_Factories				= 36,
	WMQuery_Houses					= 37
};

//======================================================================================================================
//
// States
//
enum WMState
{
	WMState_StartUp				= 0,
	WMState_LoadReady			= 1,
	WMState_Running				= 2,
	WMState_ShutDown			= 3
};

//======================================================================================================================
//
// Timers
//
enum WMTimer
{

};

//======================================================================================================================

//======================================================================================================================
//
// Sound Ids
//
enum WMSound
{
	WMSound_use_food				= 934,
	WMSound_pl_Hum_F_eat			= 1591,
	WMSound_pl_Hum_M_eat			= 3823,
	WMSound_Eat_Human_Female		= 1803,
	WMSound_Eat_Human_Male			= 1822,
	WMSound_Drink_Human_Female		= 1146,
	WMSound_Drink_Human_Male		= 2286,
	WMSound_Mission_Accepted		= 3887

};


#endif

