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

#ifndef ANH_ZONESERVER_BUILDING_ENUMS_H
#define ANH_ZONESERVER_BUILDING_ENUMS_H


//=============================================================================

enum BuildingFamily
{
    BuildingFamily_None					= 0,
    BuildingFamily_Parking_Garage		= 1,
    BuildingFamily_Cantina				= 2,
    BuildingFamily_Guildhall			= 3,
    BuildingFamily_Hotel				= 4,
    BuildingFamily_Medical_Center		= 5,
    BuildingFamily_ShuttlePort			= 6,
    BuildingFamily_Starport				= 7,
    BuildingFamily_Theme_Park			= 8,
    BuildingFamily_Theater				= 9,
    BuildingFamily_Salon				= 10,
    BuildingFamily_Cloning_Facility		= 11,
    BuildingFamily_Player_House			= 12
};

//=============================================================================

enum PlayerStructureFamily
{
    PlayerStructure_None					= 0,
    PlayerStructure_Camp					= 1,
    PlayerStructure_Harvester				= 2,
    PlayerStructure_House					= 3,
    PlayerStructure_City_Municipal			= 4,
    PlayerStructure_City_Deco				= 5,
    PlayerStructure_Factory					= 6,
    PlayerStructure_Temporary				= 7,
    PlayerStructure_Fence					= 8,
    PlayerStructure_Sign					= 9,
    PlayerStructure_TreBuilding				= 10

};

enum CampFamily
{
    Camp_None								= 0,
    Camp_Basic								= 1,
    Camp_Multi_Person						= 2,
    Camp_Improved							= 3,
    Camp_Field_Base							= 4,
    Camp_High_Tech_Field_Base				= 5
};

enum HarvesterFamily
{
    HarvesterFamily_None					= 0,

    HarvesterFamily_Fusion					= 41,
    HarvesterFamily_Solar					= 42,
    HarvesterFamily_Wind					= 43,

    HarvesterFamily_small_Flora				= 1,
    HarvesterFamily_medium_Flora			= 2,
    HarvesterFamily_large_Flora				= 3,

    HarvesterFamily_small_Mineral			= 4,
    HarvesterFamily_medium_Mineral			= 5,
    HarvesterFamily_large_Mineral			= 6,

    HarvesterFamily_small_Gas				= 7,
    HarvesterFamily_medium_Gas				= 8,
    HarvesterFamily_large_Gas				= 9,

    HarvesterFamily_small_Chemical			= 10,
    HarvesterFamily_medium_Chemical			= 11,
    HarvesterFamily_large_Chemical			= 12

};

enum FactoryFamily
{
    FactoryFamily_None						= 0

};

enum HouseFamily
{
    HouseFamily_None						= 0

};


#endif



