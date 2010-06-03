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

enum ui_window_types : unsigned int
{
	SUI_Window_MessageBox										= 1,
	SUI_Window_ListBox,
	SUI_Window_InputBox,
	SUI_Window_Teach_SelectSkill_ListBox,
	SUI_Window_Teach_OfferSkill_MsgBox,
	SUI_Window_SelectDance_Listbox,
	SUI_Window_SelectMusic_Listbox,
	SUI_Window_SelectGroupLootMode_Listbox,
	SUI_Window_SelectGroupLootMaster_Listbox,
	SUI_Window_CharacterBuilderMainMenu_ListBox,
	SUI_Window_CharacterBuilderCreditsMenu_ListBox,
	SUI_Window_CharacterBuilderCreditsMenuInventory_InputBox,
	SUI_Window_CharacterBuilderCreditsMenuBank_InputBox,
	SUI_Window_CharacterBuilderAttributesMenu_ListBox,
	SUI_Window_CharacterBuilderXpMenu_ListBox,
	SUI_Window_CharacterBuilderItemsMenu_ListBox,
	SUI_Window_SelectOutcast_Listbox,
	SUI_Window_TicketSelect_ListBox,
	SUI_Window_SmplRadioactive_MsgBox,
	SUI_Window_SmplGamble_ListBox,
	SUI_Window_SmplWaypNode_ListBox,
	SUI_Window_CharacterBuilderProfessionMastery_ListBox,
	SUI_Window_CharacterBuilderItemIdInputBox,
	//SUI_Window_CharacterBuilderResourcesCatMenu_ListBox,

	SUI_Window_CharacterBuilderResourcesTypesMenu_ListBox,
	SUI_Window_CharacterBuilderResourcesCRCMenu_ListBox,
	SUI_Window_CloneSelect_ListBox,
	SUI_Window_ResourcesParent_ListBox,
	SUI_Window_Insurance_Newbie_MessageBox,
	SUI_Window_Insurance_ListBox,
	SUI_Window_InsureAll_Newbie_MessageBox,
	SUI_Window_InsuranceAll_MessageBox,
	SUI_Window_FireworkShow_Add,
	SUI_Window_FireworkShow_Reorder,
	SUI_Window_FireworkShow_Remove,
	SUI_Window_FireworkShow_Modify,

	SUI_Window_Structure_Delete,
	SUI_Window_Structure_Delete_Confirm,
	SUI_Window_Structure_Admin_List,
	SUI_Window_Structure_Rename,
	SUI_Window_Transfer_Box,
	SUI_Window_Pay_Maintenance,
	SUI_Window_Deposit_Power,
	SUI_Window_Structure_Status,
	SUI_Window_Factory_Schematics,

	//New CharBuilderTerminal Windows
	SUI_Window_CharacterBuilder_ListBox_ExperienceMenu,
	SUI_Window_CharacterBuilder_ListBox_CreditMenu,
	SUI_Window_CharacterBuilder_ListBox_BuffMenu,
	SUI_Window_CharacterBuilder_ListBox_ItemMenu,
	SUI_Window_CharacterBuilder_ListBox_ResourceMenu,

	//2nd level menus
	SUI_Window_CharacterBuilder_ListBox_StructureMenu,
	SUI_Window_CharacterBuilder_ListBox_FurnitureMenu,
	SUI_Window_CharacterBuilder_ListBox_VehicleMenu,
	SUI_Window_CharacterBuilder_ListBox_InstrumentMenu,
	SUI_Window_CharacterBuilder_ListBox_ToolMenu,
	SUI_Window_CharacterBuilder_ListBox_WeaponMenu,
	SUI_Window_CharacterBuilder_ListBox_ArmorMenu,

	//3rd level menus
	//Structure
	SUI_Window_CharacterBuilder_ListBox_FactoryMenu,
	SUI_Window_CharacterBuilder_ListBox_HarvesterMenu,
	SUI_Window_CharacterBuilder_ListBox_CampMenu,
	SUI_Window_CharacterBuilder_ListBox_HouseMenu,
	SUI_Window_CharacterBuilder_ListBox_CivicMenu,
	//Furniture
	SUI_Window_CharacterBuilder_ListBox_RugMenu,
	SUI_Window_CharacterBuilder_ListBox_PlantMenu,
	SUI_Window_CharacterBuilder_ListBox_ElegantMenu,
	SUI_Window_CharacterBuilder_ListBox_ModernMenu,
	SUI_Window_CharacterBuilder_ListBox_PlainMenu,
	SUI_Window_CharacterBuilder_ListBox_CheapMenu,
	//Tools
	SUI_Window_CharacterBuilder_ListBox_SurveyToolMenu,
	SUI_Window_CharacterBuilder_ListBox_CraftingToolMenu,
	//Weapons
	SUI_Window_CharacterBuilder_ListBox_MeleeMenu,
	SUI_Window_CharacterBuilder_ListBox_RangedMenu,
	//Armor
	SUI_Window_CharacterBuilder_ListBox_BoneArmorMenu,
	SUI_Window_CharacterBuilder_ListBox_CompositeArmorMenu,
	SUI_Window_CharacterBuilder_ListBox_UbeseArmorMenu,

	//4th level menus
	//Harvesters
	SUI_Window_CharacterBuilder_ListBox_FloraMenu,
	SUI_Window_CharacterBuilder_ListBox_GasMenu,
	SUI_Window_CharacterBuilder_ListBox_ChemicalMenu,
	SUI_Window_CharacterBuilder_ListBox_WaterMenu,
	SUI_Window_CharacterBuilder_ListBox_MineralMenu,
	//Houses
	SUI_Window_CharacterBuilder_ListBox_GenericMenu,
	SUI_Window_CharacterBuilder_ListBox_TatooineMenu,

	//CivicStructures
	SUI_Window_CharacterBuilder_ListBox_GuildHallMenu,
	SUI_Window_CharacterBuilder_ListBox_CityHallMenu,

	//Melee Weapon
	SUI_Window_CharacterBuilder_ListBox_OneHandSwordMenu,
	SUI_Window_CharacterBuilder_ListBox_TwoHandSwordMenu,
	SUI_Window_CharacterBuilder_ListBox_AxeMenu,
	SUI_Window_CharacterBuilder_ListBox_BatonMenu,
	SUI_Window_CharacterBuilder_ListBox_PolearmMenu,
	SUI_Window_CharacterBuilder_ListBox_KnifeMenu,
	SUI_Window_CharacterBuilder_ListBox_UnarmedMenu,
	//Ranged Weapon
	SUI_Window_CharacterBuilder_ListBox_CarbineMenu,
	SUI_Window_CharacterBuilder_ListBox_ThrownMenu,
	SUI_Window_CharacterBuilder_ListBox_HeavyMenu,
	SUI_Window_CharacterBuilder_ListBox_PistolMenu,
	SUI_Window_CharacterBuilder_ListBox_RifleMenu,
	SUI_Window_CharacterBuilder_ListBox_WoundMenu,

	//trade
	SUI_Window_Trade_BankTip_ConfirmSurcharge

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

