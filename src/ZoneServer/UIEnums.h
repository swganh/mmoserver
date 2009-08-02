/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/



#ifndef ANH_ZONESEVER_UIENUMS_H
#define ANH_ZONESEVER_UIENUMS_H


enum UI_Window_Name
{
	LB_teach_Select_Skill			= 1,
	MB_teach_Ask_Learn_Skill		= 2,
};

//================================================================================

enum ui_element_types
{
	SUI_Element_Window		= 1,
	SUI_Element_Button		= 2,
	SUI_Element_DropdownBox	= 3,
	SUI_Element_InputField	= 4,
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

	
};

//================================================================================

enum ui_mb_types
{
	SUI_MB_OK			= 1,
	SUI_MB_OKCANCEL		= 2,
	SUI_MB_YESNO		= 3,
};

//================================================================================

enum ui_lb_types
{
	SUI_LB_OK			= 1,
	SUI_LB_OKCANCEL		= 2,
};

//================================================================================

enum ui_ib_types
{
	SUI_IB_NODROPDOWN_OK			= 1,
	SUI_IB_DROPDOWN_OK				= 2,
	SUI_IB_NODROPDOWN_OKCANCEL		= 3,
	SUI_IB_DROPDOWN_OKCANCEL		= 4,
};

//================================================================================


#endif

