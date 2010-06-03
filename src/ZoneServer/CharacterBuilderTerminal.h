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

#ifndef ANH_ZONESERVER_TANGIBLE_CHARACTERBUILDERTERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_CHARACTERBUILDERTERMINAL_H

#include "Terminal.h"
#include <vector>

class PlayerObject;

typedef std::vector<std::pair<string,uint32> >	SortedList;
typedef std::vector<std::pair<uint32,uint32> >	ItemEntryList;
typedef	std::vector<uint32>						ResCatLookupList;

//=============================================================================

class CharacterBuilderTerminal : public Terminal
{
	friend class TerminalFactory;

public:

	CharacterBuilderTerminal();
	~CharacterBuilderTerminal();

	virtual void		prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
	void		handleObjectMenuSelect(uint8 messageType,Object* srcObject);
	void		handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);

private:

	void GiveItem(PlayerObject* player, uint32 id);
	void InitMenus();
	void InitExperience();
	void InitCredits();
	void InitBuffs();
	void InitItems();
	void InitStructures();
	void InitFurniture();
	void InitVehicles();
	void InitInstruments();
	void InitTools();
	void InitWeapons();
	void InitArmor();
	void InitProfessions();
	void InitWounds();

	void SendXPMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void SendResourcesMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleResourcesCRC(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleResourceTypes(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleResourcesTypes(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleWoundMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window);

	void _handleMainMenu(PlayerObject* player, uint32 action, int32 element, string inputStr, UIWindow* window);
	void _handleExperienceMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleCreditMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleBuffMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleItemMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleResourceMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleStructureMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleFurnitureMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleVehicleMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleInstrumentMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleToolMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleWeaponMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleArmorMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleFactoryMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleHarvesterMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleCampMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleHouseMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleCivicMenu(PlayerObject* player, uint32 action, int32 element, string inputStr, UIWindow* window);
	void _handleGuildMenu(PlayerObject* player, uint32 action, int32 element, string inputStr, UIWindow* window);
	void _handleCityMenu(PlayerObject* player, uint32 action, int32 element, string inputStr, UIWindow* window);
	void _handleRugMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handlePlantMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleElegantMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleModernMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handlePlainMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleCheapMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleSurveyToolMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleCraftingToolMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleMeleeMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleRangedMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleBoneArmorMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleCompositeArmorMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleUbeseArmorMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleFloraMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleGasMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleChemicalMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleWaterMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleMineralMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleGenericMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleTatooineMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleOneHandSwordMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleTwoHandSwordMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleAxeMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleBatonMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handlePolearmMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleKnifeMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleCarbineMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleThrownMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleHeavyMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handlePistolMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleRifleMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleCSRItemSelect(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleMainCsrMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window);
	void _handleProfessionMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window);

	int32				mInputBoxAmount;
	BStringVector		mMainMenu;
	BStringVector		mMainCsrMenu;
	//BStringVector		mCreditsMenu;
	//BStringVector		mItemsMenu;
	//ItemEntryList		mItemsTypes;
	//BStringVector		mAttributesMenu;

	//1st level menus
	BStringVector			mExperienceMenu;
	BStringVector			mProfessionMenu;
	BStringVector			mCreditMenu;
	BStringVector			mBuffMenu;
	BStringVector			mItemMenu;
	BStringVector			mResourceMenu;
	BStringVector			mWoundMenu;

	//2nd level menus
	BStringVector			mStructureMenu;
	BStringVector			mFurnitureMenu;
	BStringVector			mVehicleMenu;
	BStringVector			mInstrumentMenu;
	BStringVector			mToolMenu;
	BStringVector			mWeaponMenu;
	BStringVector			mArmorMenu;

	//3rd level menus
	//Structure
	BStringVector			mFactoryMenu;
	BStringVector			mHarvesterMenu;
	BStringVector			mCampMenu;
	BStringVector			mHouseMenu;
	BStringVector			mCivicMenu;
	//Furniture
	BStringVector			mRugMenu;
	BStringVector			mPlantMenu;
	BStringVector			mElegantMenu;
	BStringVector			mModernMenu;
	BStringVector			mPlainMenu;
	BStringVector			mCheapMenu;
	//Tools
	BStringVector			mSurveyToolMenu;
	BStringVector			mCraftingToolMenu;
	//Weapons
	BStringVector			mMeleeMenu;
	BStringVector			mRangedMenu;
	//Armor
	BStringVector			mBoneArmorMenu;
	BStringVector			mCompositeArmorMenu;
	BStringVector			mUbeseArmorMenu;

	//4th level menus
	//Harvesters
	BStringVector			mFloraMenu;
	BStringVector			mGasMenu;
	BStringVector			mChemicalMenu;
	BStringVector			mWaterMenu;
	BStringVector			mMineralMenu;
	//Houses
	BStringVector			mGenericMenu;
	BStringVector			mTatooineMenu;
	//Civic Structures
	BStringVector			mGuildHallMenu;
	BStringVector			mCityHallMenu;
	//Melee Weapon
	BStringVector			mOneHandSwordMenu;
	BStringVector			mTwoHandSwordMenu;
	BStringVector			mAxeMenu;
	BStringVector			mBatonMenu;
	BStringVector			mPolearmMenu;
	BStringVector			mKnifeMenu;
	BStringVector			mUnarmedMenu;
	//Ranged Weapon
	BStringVector			mCarbineMenu;
	BStringVector			mThrownMenu;
	BStringVector			mHeavyMenu;
	BStringVector			mPistolMenu;
	BStringVector			mRifleMenu;


	SortedList*			mSortedList;
};

//======================================================================================================================
//
//thats were the raw frog data goes
//


class ItemFrogItemClass
{
public:

	ItemFrogItemClass () {};      
	~ItemFrogItemClass (){}; 

	//void AddCommoditie(TypeListItem theCommoditie){Commoditie = theCommoditie;}
	string	GetName() {return(name);}
	void	SetName(string n) {name = n;}
	
	uint32	GetType(){return type;}
	void	SetType(uint32 n) {type = n;}

	uint32	GetFamily(){return family;}
	void	SetFamily(uint32 n) {family = n;}

	uint8	GetActive(){return active;}
	void	SetActive(uint8 n) {active = n;}

	uint64			id;
	uint32			type;
	uint32			family;
	string			name;
	uint8			active;
	string			f_name;

};

typedef std::vector<ItemFrogItemClass*> ItemFrogItemList;

class ItemFrogTypeClass
{
public:

	ItemFrogTypeClass () {};      
	~ItemFrogTypeClass (); 

	ItemFrogItemClass*			mItemFrogItemHandler;
	ItemFrogItemList			mItemFrogItemList;

	//void InsertItem(TypeListItem Commoditie);
	ItemFrogItemClass*	getItem(uint32 nr){return mItemFrogItemList[nr];}
	void				InsertItem(ItemFrogItemClass* item){mItemFrogItemList.push_back(item);}
	
	ItemFrogItemClass*	getType(uint32 type);

	uint32			family;
	string			f_name;

};


typedef std::vector<ItemFrogTypeClass*>			ItemFrogTypeList;

class ItemFrogClass
{
	public:
		ItemFrogTypeClass*				ItemFrogTypeHandler;
		ItemFrogTypeList				mItemFrogTypeList;

		ItemFrogClass(){}
		~ItemFrogClass();

		ItemFrogTypeClass*		GetFamily(uint32 nr){return mItemFrogTypeList[nr];}
		void					AddFamily(ItemFrogTypeClass* item){mItemFrogTypeList.push_back(item) ;}//adds an item to the list
		
		ItemFrogTypeClass*		LookUpFamily(uint32 family);
		ItemFrogItemClass*		LookUpType(uint32 type);
		 //TypeListItem	LookUpCommoditie(uint32 crc);
		 //uint32			getCategory(uint32 crc);
		 
		 //uint32			GetCount(){return(CommoditiesTypeCount);}
		 //uint32			CommoditiesTypeCount;
		
};

//=============================================================================

#endif

