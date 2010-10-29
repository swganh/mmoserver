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
// from sounds table
enum WMSound
{

    WMSound_use_food				= 934,
    WMSound_pl_Hum_F_eat			= 1591,
    WMSound_pl_Hum_M_eat			= 3823,
    WMSound_pl_Reptile_F_eat		= 1156,
    WMSound_pl_Reptile_M_eat		= 3284,
    WMSound_pl_Wke_F_eat			= 2367,
    WMSound_pl_Wke_M_eat			= 90,

    WMSound_Eat_Human_Female		= 1803,
    WMSound_Eat_Human_Male			= 1822,
    WMSound_Eat_Wookiee_Female		= 2075,
    WMSound_Eat_Wookiee_Male		= 1477,
    WMSound_Eat_Reptile_Female		= 181,
    WMSound_Eat_Reptile_Male		= 3226,

    WMSound_Drink_Human_Female		= 1146,
    WMSound_Drink_Human_Male		= 2286,
    WMSound_Drink_Reptile_Female	= 2458,
    WMSound_Drink_Reptile_Male		= 1921,
    WMSound_Drink_Wookiee_Female	= 3264,
    WMSound_Drink_Wookiee_Male		= 124,

    WMSound_Mission_Accepted		= 3887

};


#endif

