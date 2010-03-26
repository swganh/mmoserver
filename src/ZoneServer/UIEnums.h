/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/



#ifndef ANH_ZONESEVER_UIENUMS_H
#define ANH_ZONESEVER_UIENUMS_H


enum UI_Window_Name
{
	LB_teach_Select_Skill			= 1,
	MB_teach_Ask_Learn_Skill		= 2
};

//================================================================================

enum ui_element_types
{
	SUI_Element_Window		= 1,
	SUI_Element_Button		= 2,
	SUI_Element_DropdownBox	= 3,
	SUI_Element_InputField	= 4
};

//================================================================================

enum ui_window_types
{
	SUI_Window_MessageBox										= 1,
	SUI_Window_ListBox											= 2,
	SUI_Window_InputBox											= 3,
	SUI_Window_Teach_SelectSkill_ListBox						= 4,
	SUI_Window_Teach_OfferSkill_MsgBox							= 5,
	SUI_Window_SelectDance_Listbox								= 6,
	SUI_Window_SelectMusic_Listbox								= 7,
	SUI_Window_SelectGroupLootMode_Listbox						= 8,
	SUI_Window_SelectGroupLootMaster_Listbox					= 9,
	SUI_Window_CharacterBuilderMainMenu_ListBox					= 10,
	SUI_Window_CharacterBuilderCreditsMenu_ListBox				= 11,
	SUI_Window_CharacterBuilderCreditsMenuInventory_InputBox	= 12,
	SUI_Window_CharacterBuilderCreditsMenuBank_InputBox			= 13,
	SUI_Window_CharacterBuilderAttributesMenu_ListBox			= 14,
	SUI_Window_CharacterBuilderXpMenu_ListBox					= 15,
	SUI_Window_CharacterBuilderItemsMenu_ListBox				= 16,
	SUI_Window_SelectOutcast_Listbox							= 17,
	SUI_Window_TicketSelect_ListBox								= 18,
	SUI_Window_SmplRadioactive_MsgBox							= 19,
	SUI_Window_SmplGamble_ListBox								= 20,
	SUI_Window_SmplWaypNode_ListBox								= 21,
	SUI_Window_CharacterBuilderProfessionMastery_ListBox		= 22,
	SUI_Window_CharacterBuilderItemIdInputBox 					= 23,
	//SUI_Window_CharacterBuilderResourcesCatMenu_ListBox		= 24,

	SUI_Window_CharacterBuilderResourcesTypesMenu_ListBox		= 25,
	SUI_Window_CharacterBuilderResourcesCRCMenu_ListBox			= 26,
	SUI_Window_CloneSelect_ListBox								= 29,
	SUI_Window_ResourcesParent_ListBox							= 30,
	SUI_Window_Insurance_Newbie_MessageBox						= 31,
	SUI_Window_Insurance_ListBox								= 32,
	SUI_Window_InsureAll_Newbie_MessageBox						= 33,
	SUI_Window_InsuranceAll_MessageBox							= 34,
	SUI_Window_FireworkShow_Add									= 35,
	SUI_Window_FireworkShow_Reorder								= 36,
	SUI_Window_FireworkShow_Remove								= 37,
	SUI_Window_FireworkShow_Modify								= 38,

	SUI_Window_Structure_Delete									= 39,
	SUI_Window_Structure_Delete_Confirm							= 40,
	SUI_Window_Structure_Admin_List								= 41,
	SUI_Window_Structure_Rename									= 42,
	SUI_Window_Transfer_Box										= 43,
	SUI_Window_Pay_Maintenance									= 44,
	SUI_Window_Deposit_Power									= 45,
	SUI_Window_Structure_Status									= 46,
	SUI_Window_Factory_Schematics								= 47,

	//New CharBuilderTerminal Windows
	SUI_Window_CharacterBuilder_ListBox_ExperienceMenu=48,
	SUI_Window_CharacterBuilder_ListBox_CreditMenu=49,
	SUI_Window_CharacterBuilder_ListBox_BuffMenu=50,
	SUI_Window_CharacterBuilder_ListBox_ItemMenu=51,
	SUI_Window_CharacterBuilder_ListBox_ResourceMenu=52,

	//2nd level menus
	SUI_Window_CharacterBuilder_ListBox_StructureMenu=53,
	SUI_Window_CharacterBuilder_ListBox_FurnitureMenu=54,
	SUI_Window_CharacterBuilder_ListBox_VehicleMenu=55,
	SUI_Window_CharacterBuilder_ListBox_InstrumentMenu=56,
	SUI_Window_CharacterBuilder_ListBox_ToolMenu=57,
	SUI_Window_CharacterBuilder_ListBox_WeaponMenu=58,
	SUI_Window_CharacterBuilder_ListBox_ArmorMenu=59,

	//3rd level menus
	//Structure
	SUI_Window_CharacterBuilder_ListBox_FactoryMenu=60,
	SUI_Window_CharacterBuilder_ListBox_HarvesterMenu=61,
	SUI_Window_CharacterBuilder_ListBox_CampMenu=62,
	SUI_Window_CharacterBuilder_ListBox_HouseMenu=63,
	//Furniture
	SUI_Window_CharacterBuilder_ListBox_RugMenu=64,
	SUI_Window_CharacterBuilder_ListBox_PlantMenu=65,
	SUI_Window_CharacterBuilder_ListBox_ElegantMenu=66,
	SUI_Window_CharacterBuilder_ListBox_ModernMenu=67,
	SUI_Window_CharacterBuilder_ListBox_PlainMenu=68,
	SUI_Window_CharacterBuilder_ListBox_CheapMenu=69,
	//Tools
	SUI_Window_CharacterBuilder_ListBox_SurveyToolMenu=70,
	SUI_Window_CharacterBuilder_ListBox_CraftingToolMenu=71,
	//Weapons
	SUI_Window_CharacterBuilder_ListBox_MeleeMenu=72,
	SUI_Window_CharacterBuilder_ListBox_RangedMenu=73,
	//Armor
	SUI_Window_CharacterBuilder_ListBox_BoneArmorMenu=74,
	SUI_Window_CharacterBuilder_ListBox_CompositeArmorMenu=75,
	SUI_Window_CharacterBuilder_ListBox_UbeseArmorMenu=76,

	//4th level menus
	//Harvesters
	SUI_Window_CharacterBuilder_ListBox_FloraMenu=77,
	SUI_Window_CharacterBuilder_ListBox_GasMenu=78,
	SUI_Window_CharacterBuilder_ListBox_ChemicalMenu=79,
	SUI_Window_CharacterBuilder_ListBox_WaterMenu=80,
	SUI_Window_CharacterBuilder_ListBox_MineralMenu=81,
	//Houses
	SUI_Window_CharacterBuilder_ListBox_GenericMenu=82,
	SUI_Window_CharacterBuilder_ListBox_TatooineMenu=83,
	//Melee Weapon
	SUI_Window_CharacterBuilder_ListBox_OneHandSwordMenu=84,
	SUI_Window_CharacterBuilder_ListBox_TwoHandSwordMenu=85,
	SUI_Window_CharacterBuilder_ListBox_AxeMenu=86,
	SUI_Window_CharacterBuilder_ListBox_BatonMenu=87,
	SUI_Window_CharacterBuilder_ListBox_PolearmMenu=88,
	SUI_Window_CharacterBuilder_ListBox_KnifeMenu=89,
	//Ranged Weapon
	SUI_Window_CharacterBuilder_ListBox_CarbineMenu=90,
	SUI_Window_CharacterBuilder_ListBox_ThrownMenu=91,
	SUI_Window_CharacterBuilder_ListBox_HeavyMenu=92,
	SUI_Window_CharacterBuilder_ListBox_PistolMenu=93,
	SUI_Window_CharacterBuilder_ListBox_RifleMenu=94

};

//================================================================================

enum ui_mb_types
{
	SUI_MB_OK				= 1,
	SUI_MB_OKCANCEL			= 2,
	SUI_MB_YESNO			= 3
	
};

//================================================================================

enum ui_lb_types
{
	SUI_LB_OK							= 1,
	SUI_LB_OKCANCEL						= 2,
	SUI_LB_CANCELREFRESH				= 3,
	SUI_LB_CANCEL_SCHEMATIC_REMOVEUSE	= 4,
	SUI_LB_CANCEL_SCHEMATIC_USE			= 5,
	SUI_LB_YESNO						= 6
};

//================================================================================

enum ui_ib_types
{
	SUI_IB_NODROPDOWN_OK			= 1,
	SUI_IB_DROPDOWN_OK				= 2,
	SUI_IB_NODROPDOWN_OKCANCEL		= 3,
	SUI_IB_DROPDOWN_OKCANCEL		= 4
};

//================================================================================


#endif

