/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

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
	ItemFamily_FireWork					= 19,
	ItemFamily_Deed						= 15,
	ItemFamily_Medicine					= 22,
	ItemFamily_Mission					= 23,
	ItemFamily_Powerup					= 24,
	ItemFamily_Scout					= 25
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

	ItemType_harvester_ore_personal		= 1613,
	ItemType_harvester_ore_heavy		= 1614,
	ItemType_harvester_ore_medium		= 1615,


	//====================================================
	// Guild Halls
	ItemType_deed_guildhall_corellian	= 1597,
	ItemType_deed_guildhall_generic		= 1598,
	ItemType_deed_guildhall_naboo		= 1599,
	ItemType_deed_guildhall_tatooine	= 1600,

	//====================================================
	// bikes
	ItemType_Deed_X34					= 1736,
	ItemType_Deed_Speederbike			= 1737,
	ItemType_Deed_Swoop					= 1738,


	ItemType_Stimpack_A					= 1768,
	ItemType_Stimpack_B					= 1769,
	ItemType_Stimpack_C					= 1770,
	ItemType_Stimpack_D					= 1771,
	ItemType_Stimpack_E					= 1772,
	ItemType_Pet_Stimpack_A				= 1939,
	ItemType_Pet_Stimpack_B				= 1940,
	ItemType_Pet_Stimpack_C				= 1941,
	ItemType_Pet_Stimpack_D				= 1942,
	ItemType_Camp_basic					= 1970,
	ItemType_Camp_elite					= 1971,
	ItemType_Camp_improved				= 1972,
	ItemType_Camp_luxury				= 1973,
	ItemType_Camp_multi					= 1974,
	ItemType_Camp_quality				= 1975,
	ItemType_Firework_Type_1			= 1756,
	ItemType_Firework_Type_2			= 1761,
	ItemType_Firework_Type_3			= 1760,
	ItemType_Firework_Type_4			= 1755,
	ItemType_Firework_Type_5			= 1754,
	ItemType_Firework_Type_10			= 1757,
	ItemType_Firework_Type_11			= 1758,
	ItemType_Firework_Type_18			= 1759,
	ItemType_Firework_Show				= 1762


	//Owen - changing the ids of the fireworks to be in order of the firework numbers would look cleaner, here and other places
};

//=============================================================================

#endif

