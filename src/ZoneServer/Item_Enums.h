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

#ifndef ANH_ZONESERVER_ITEM_ENUMS_H
#define ANH_ZONESERVER_ITEM_ENUMS_H


//=============================================================================

enum ItemFamily
{
	ItemFamily_SurveyTools				= 1,
	ItemFamily_TravelTickets			= 2,
	ItemFamily_CraftingTools			= 3,
	ItemFamily_CraftingStations			= 4,
	ItemFamily_Furniture				= 5,
	ItemFamily_Foods					= 6,
	ItemFamily_Wearable					= 7,
	ItemFamily_ManufacturingSchematic	= 8,
	ItemFamily_Generic					= 9,
	ItemFamily_Weapon					= 10,
	ItemFamily_Instrument				= 11,
	ItemFamily_BugJar					= 12,
	ItemFamily_FireWork					= 19,
	ItemFamily_Deed						= 15,
	ItemFamily_Dice						= 16,
	ItemFamily_Medicine					= 22,
	ItemFamily_Mission					= 23,
	ItemFamily_Powerup					= 24,
	ItemFamily_Scout					= 25,
	ItemFamily_Hopper					= 29,
	ItemFamily_FactoryCrate				= 30
};

//=============================================================================

enum ItemType
{
	//====================================================
	// cafting tools / stations
	ItemType_ClothingTool				= 11,
	ItemType_GenericTool				= 12,
	ItemType_WeaponTool					= 13,
	ItemType_FoodTool					= 14,
	ItemType_StructureTool				= 15,
	ItemType_JediTool					= 16,
	ItemType_SpaceTool					= 17,

	ItemType_StructureStationPublic		= 18,
	ItemType_ClothingStation			= 19,
	ItemType_FoodStationPublic			= 20,
	ItemType_WeaponStationPublic		= 22,
	ItemType_WeaponStation				= 23,
	ItemType_StructureStation			= 24,
	ItemType_ClothingStationPublic		= 29,
	ItemType_FoodStation				= 30,
	ItemType_SpaceStationPublic			= 31,
	ItemType_SpaceStation				= 32,

	ItemType_ManSchematic				= 1312,

	//====================================================
	//Foraged Foods
	ItemType_Jar_Foraged_Fungus			= 38,
	ItemType_Ko_Do_Fruit				= 51,
	ItemType_EsostEw_Zann				= 63,
	ItemType_Maroj_Melon				= 66,
	ItemType_Foraged_Berries			= 74,
	ItemType_Sijjo_Sewi					= 82,
	ItemType_Sosi_Hodor					= 97,
	ItemType_Foraged_Bugs				= 110,
	ItemType_Schule_Nef					= 117,
	ItemType_Wild_Snaff					= 129,
	ItemType_Flurr_Cle_Onion			= 161,
	ItemType_Alever_Twethpek			= 170,
	ItemType_Jar_Foraged_Grubs			= 186,
	ItemType_Insect_Bait				= 2784,
	ItemType_Worm_Bait					= 2785,
	ItemType_Grub_Bait					= 2786,
	ItemType_Chum_Bait					= 2787,

	//====================================================
	// Music Instruments
	ItemType_Slitherhorn				= 1313,
	ItemType_Kloo_Horn					= 1314,
	ItemType_fizzz						= 1315,
	ItemType_bandfill					= 1316,
	ItemType_flute_droopy				= 1317,//chidinkalu horn
	ItemType_fanfar						= 1318,
	ItemType_omni_box					= 1319,
	ItemType_Nalargon					= 1320,
	ItemType_nalargon_max_reebo			= 1321,
	ItemType_traz						= 1322,
	ItemType_mandoviol					= 1323,
	ItemType_organ						= 1324,
	ItemType_drums						= 1325,
	ItemType_dual_wave_synthesizer		= 1326,


	//====================================================
	// Factories
	ItemType_factory_clothing			= 1590,
	ItemType_factory_food				= 1591,
	ItemType_factory_item				= 1592,
	ItemType_factory_structure			= 1593,

	//====================================================
	// Harvesters
	ItemType_generator_fusion_personal	= 1594,
	ItemType_generator_solar_personal	= 1595,
	ItemType_generator_wind_personal	= 1596,

	ItemType_harvester_flora_personal	= 1601,
	ItemType_harvester_flora_heavy		= 1602,
	ItemType_harvester_flora_medium		= 1603,

	ItemType_harvester_gas_personal		= 1604,
	ItemType_harvester_gas_heavy		= 1605,
	ItemType_harvester_gas_medium		= 1606,

	ItemType_harvester_liquid_personal	= 1607,
	ItemType_harvester_liquid_heavy		= 1608,
	ItemType_harvester_liquid_medium	= 1609,

	ItemType_harvester_moisture_personal	= 1610,
	ItemType_harvester_moisture_heavy		= 1611,
	ItemType_harvester_moisture_medium		= 1612,

	ItemType_harvester_ore_personal			= 1613,
	ItemType_harvester_ore_heavy			= 1614,
	ItemType_harvester_ore_medium			= 1615,

	ItemType_deed_bank_corellian			= 1560,
	ItemType_deed_bank_naboo				= 1561,
	ItemType_deed_merchant_tent				= 1728,

	ItemType_deed_naboo_small_house			= 1616,
	
	ItemType_deed_naboo_large_house			= 1729,
	ItemType_deed_naboo_medium_house		= 1730,
	ItemType_deed_naboo_small_house_2		= 1731,
	

	ItemType_deed_corellia_large_house		= 1712,
	ItemType_deed_corellia_large_house_2	= 1713,
	ItemType_deed_corellia_medium_house		= 1714,
	ItemType_deed_corellia_medium_house_2	= 1715,
	
	ItemType_deed_corellia_small_house_1	= 1716,
	ItemType_deed_corellia_small_house_2	= 1717,
	ItemType_deed_corellia_small_house_3	= 1718,
	ItemType_deed_corellia_small_house_4	= 1719,	
	
	ItemType_deed_generic_large_house_1		= 1720,
	ItemType_deed_generic_large_house_2		= 1721,
	ItemType_deed_generic_medium_house_1	= 1722,
	ItemType_deed_generic_medium_house_2	= 1723,
	ItemType_deed_generic_small_house_1		= 1724,
	ItemType_deed_generic_small_house_2		= 1725,
	ItemType_deed_generic_small_house_3		= 1726,
	ItemType_deed_generic_small_house_4		= 1727,

	ItemType_deed_tatooine_large_house		= 1732,
	ItemType_deed_tatooine_medium_house		= 1733,
	ItemType_deed_tatooine_small_house		= 1734,
	ItemType_deed_tatooine_small_house_2	= 1735,

	//====================================================
	// Guild Halls
	ItemType_deed_guildhall_corellian	= 1597,
	ItemType_deed_guildhall_generic		= 1598,
	ItemType_deed_guildhall_naboo		= 1599,
	ItemType_deed_guildhall_tatooine	= 1600,

	ItemType_deed_cityhall_corellia		= 1566,
	ItemType_deed_cityhall_naboo		= 1567,
	ItemType_deed_cityhall_tatooine		= 1568,

	//====================================================
	// bikes
	ItemType_Deed_X34					= 1736,
	ItemType_Deed_Speederbike			= 1737,
	ItemType_Deed_Swoop					= 1738,

	// meds
	ItemType_Stimpack_A					= 1768,
	ItemType_Stimpack_B					= 1769,
	ItemType_Stimpack_C					= 1770,
	ItemType_Stimpack_D					= 1771,
	ItemType_Stimpack_E					= 1772,
	ItemType_Ranged_Stimpack_A			= 1875,
	ItemType_Ranged_Stimpack_B			= 1876,
	ItemType_Ranged_Stimpack_C			= 1877,
	ItemType_Ranged_Stimpack_D			= 1878,
	ItemType_Ranged_Stimpack_E			= 1879,
	// woundpacks
	ItemType_Wound_Action_A				= 1901,
	ItemType_Wound_Action_B				= 1902,
	ItemType_Wound_Action_C				= 1903,
	ItemType_Wound_Action_D				= 1904,
	ItemType_Wound_Action_E				= 1905,
	ItemType_Wound_Constitution_A		= 1906,
	ItemType_Wound_Constitution_B		= 1907,
	ItemType_Wound_Constitution_C		= 1908,
	ItemType_Wound_Constitution_D		= 1909,
	ItemType_Wound_Constitution_E		= 1910,
	ItemType_Wound_Health_A				= 1911,
	ItemType_Wound_Health_B				= 1912,
	ItemType_Wound_Health_C				= 1913,
	ItemType_Wound_Health_D				= 1914,
	ItemType_Wound_Health_E				= 1915,
	ItemType_Wound_Quickness_A			= 1916,
	ItemType_Wound_Quickness_B			= 1917,
	ItemType_Wound_Quickness_C			= 1918,
	ItemType_Wound_Quickness_D			= 1919,
	ItemType_Wound_Quickness_E			= 1920,
	ItemType_Wound_Stamina_A			= 1921,
	ItemType_Wound_Stamina_B			= 1922,
	ItemType_Wound_Stamina_C			= 1923,
	ItemType_Wound_Stamina_D			= 1924,
	ItemType_Wound_Stamina_E			= 1925,
	ItemType_Wound_Strength_A			= 1926,
	ItemType_Wound_Strength_B			= 1927,
	ItemType_Wound_Strength_C			= 1928,
	ItemType_Wound_Strength_D			= 1929,
	ItemType_Wound_Strength_E			= 1930,
	// pet packs
	ItemType_Pet_Stimpack_A				= 1939,
	ItemType_Pet_Stimpack_B				= 1940,
	ItemType_Pet_Stimpack_C				= 1941,
	ItemType_Pet_Stimpack_D				= 1942,
	

	// camps
	ItemType_Camp_basic					= 1970, //Basic Camp Kit
	ItemType_Camp_elite					= 1971, //Field Base Kit
	ItemType_Camp_improved				= 1972, //Improved Camp Kit
	ItemType_Camp_luxury				= 1973, //High Tech Field Base Kit
	ItemType_Camp_multi					= 1974, //Multiperson Camp Kit
	ItemType_Camp_quality				= 1975, //High Quality Camp Kit
	
	// fireworks
	ItemType_Firework_Type_1			= 1756,
	ItemType_Firework_Type_2			= 1761,
	ItemType_Firework_Type_3			= 1760,
	ItemType_Firework_Type_4			= 1755,
	ItemType_Firework_Type_5			= 1754,
	ItemType_Firework_Type_10			= 1757,
	ItemType_Firework_Type_11			= 1758,
	ItemType_Firework_Type_18			= 1759,
	ItemType_Firework_Show				= 1762,

	// cubes
	ItemType_Generic_ChanceCube			= 1739
};

//=============================================================================

#endif

